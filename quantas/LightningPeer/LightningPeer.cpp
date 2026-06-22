/*
Copyright 2024

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version. QUANTAS is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "LightningPeer.hpp"
#include "../Common/OutputWriter.hpp"
#include "../Common/RandomUtil.hpp"
#include "../Common/RoundManager.hpp"

namespace quantas {

namespace {

enum class TorusDirection {
    Invalid,
    Right,
    Up,
    Left,
    Down,
};

struct PathState {
    interfaceId node = NO_PEER_ID;
    int phase = 0;
};

std::pair<int, int> torusCoordinate(interfaceId node,
                                    const std::map<interfaceId, size_t>& peerIndexById,
                                    int width) {
    auto it = peerIndexById.find(node);
    if (it == peerIndexById.end() || width <= 0) {
        return {-1, -1};
    }

    const int index = static_cast<int>(it->second);
    return {index / width, index % width};
}

TorusDirection torusDirectionBetween(interfaceId from,
                                     interfaceId to,
                                     const std::map<interfaceId, size_t>& peerIndexById,
                                     int height,
                                     int width) {
    if (height <= 0 || width <= 0) {
        return TorusDirection::Invalid;
    }

    const auto [fromRow, fromCol] = torusCoordinate(from, peerIndexById, width);
    const auto [toRow, toCol] = torusCoordinate(to, peerIndexById, width);
    if (fromRow < 0 || toRow < 0) {
        return TorusDirection::Invalid;
    }

    if (fromRow == toRow && ((fromCol + 1) % width) == toCol) {
        return TorusDirection::Right;
    }
    if (fromRow == toRow && ((fromCol - 1 + width) % width) == toCol) {
        return TorusDirection::Left;
    }
    if (fromCol == toCol && ((fromRow - 1 + height) % height) == toRow) {
        return TorusDirection::Up;
    }
    if (fromCol == toCol && ((fromRow + 1) % height) == toRow) {
        return TorusDirection::Down;
    }
    return TorusDirection::Invalid;
}

std::vector<interfaceId> randomizedNeighbors(
    const std::map<interfaceId, std::set<interfaceId>>& topologyView,
    interfaceId node) {
    std::vector<interfaceId> ordered;
    auto it = topologyView.find(node);
    if (it == topologyView.end()) {
        return ordered;
    }
    ordered.assign(it->second.begin(), it->second.end());
    std::shuffle(ordered.begin(), ordered.end(), threadLocalEngine());
    return ordered;
}

bool dfsPath(interfaceId current,
             interfaceId target,
             const std::map<interfaceId, std::set<interfaceId>>& topologyView,
             std::set<interfaceId>& visited,
             std::vector<interfaceId>& path,
             const std::pair<interfaceId, interfaceId>* forbiddenEdge = nullptr) {
    visited.insert(current);
    path.push_back(current);

    if (current == target) {
        return true;
    }

    for (interfaceId next : randomizedNeighbors(topologyView, current)) {
        if (visited.count(next) > 0) {
            continue;
        }
        if (forbiddenEdge != nullptr &&
            ((current == forbiddenEdge->first && next == forbiddenEdge->second) ||
             (current == forbiddenEdge->second && next == forbiddenEdge->first))) {
            continue;
        }
        if (dfsPath(next, target, topologyView, visited, path, forbiddenEdge)) {
            return true;
        }
    }

    path.pop_back();
    return false;
}

std::vector<interfaceId> randomPath(interfaceId source,
                                    interfaceId target,
                                    const std::map<interfaceId, std::set<interfaceId>>& topologyView,
                                    const std::pair<interfaceId, interfaceId>* forbiddenEdge = nullptr) {
    std::vector<interfaceId> path;
    std::set<interfaceId> visited;
    if (!dfsPath(source, target, topologyView, visited, path, forbiddenEdge)) {
        path.clear();
    }
    return path;
}

bool torusMoveAllowed(TorusDirection direction, int phase, bool reverse) {
    if (!reverse) {
        if (phase == 0) {
            return direction == TorusDirection::Right || direction == TorusDirection::Up;
        }
        return direction == TorusDirection::Up;
    }

    if (phase == 0) {
        return direction == TorusDirection::Left || direction == TorusDirection::Down;
    }
    return direction == TorusDirection::Down;
}

int nextPhaseForMove(TorusDirection direction, int phase, bool reverse) {
    if (!reverse) {
        if (phase == 0 && direction == TorusDirection::Up) {
            return 1;
        }
        return phase;
    }

    if (phase == 0 && direction == TorusDirection::Down) {
        return 1;
    }
    return phase;
}

std::vector<interfaceId> torusMonotonePath(
    interfaceId source,
    interfaceId target,
    const std::map<interfaceId, std::set<interfaceId>>& topologyView,
    const std::map<interfaceId, size_t>& peerIndexById,
    int height,
    int width,
    bool reverse,
    const std::pair<interfaceId, interfaceId>* forbiddenEdge = nullptr) {
    std::vector<interfaceId> result;
    if (source == NO_PEER_ID || target == NO_PEER_ID) {
        return result;
    }
    if (source == target) {
        result.push_back(source);
        return result;
    }

    std::vector<PathState> frontier;
    std::map<std::pair<interfaceId, int>, std::pair<interfaceId, int>> parent;
    std::set<std::pair<interfaceId, int>> visited;

    frontier.push_back({source, 0});
    visited.insert({source, 0});
    size_t frontIndex = 0;

    std::pair<interfaceId, int> goal = {NO_PEER_ID, -1};
    while (frontIndex < frontier.size()) {
        const PathState state = frontier[frontIndex++];
        auto it = topologyView.find(state.node);
        if (it == topologyView.end()) {
            continue;
        }

        for (interfaceId next : it->second) {
            if (forbiddenEdge != nullptr &&
                ((state.node == forbiddenEdge->first && next == forbiddenEdge->second) ||
                 (state.node == forbiddenEdge->second && next == forbiddenEdge->first))) {
                continue;
            }

            const TorusDirection direction =
                torusDirectionBetween(state.node, next, peerIndexById, height, width);
            if (!torusMoveAllowed(direction, state.phase, reverse)) {
                continue;
            }

            const int nextPhase = nextPhaseForMove(direction, state.phase, reverse);
            const std::pair<interfaceId, int> nextState = {next, nextPhase};
            if (visited.count(nextState) > 0) {
                continue;
            }

            visited.insert(nextState);
            parent[nextState] = {state.node, state.phase};
            if (next == target) {
                goal = nextState;
                frontIndex = frontier.size();
                break;
            }
            frontier.push_back({next, nextPhase});
        }
    }

    if (goal.first == NO_PEER_ID) {
        return result;
    }

    std::vector<interfaceId> reversedPath;
    std::pair<interfaceId, int> cursor = goal;
    reversedPath.push_back(cursor.first);
    while (!(cursor.first == source && cursor.second == 0)) {
        auto parentIt = parent.find(cursor);
        if (parentIt == parent.end()) {
            reversedPath.clear();
            return reversedPath;
        }
        cursor = parentIt->second;
        reversedPath.push_back(cursor.first);
    }

    result.assign(reversedPath.rbegin(), reversedPath.rend());
    return result;
}

} // namespace

static bool registerLightningPeer = []() {
    return PeerRegistry::registerPeerType(
        "LightningPeer",
        [](interfaceId pubId) { return new LightningPeer(new NetworkInterfaceAbstract(pubId)); });
}();

static bool registerLightningPeerConcrete = []() {
    return PeerRegistry::registerPeerType(
        "LightningPeerConcrete",
        [](interfaceId) { return new LightningPeer(new NetworkInterfaceConcrete()); });
}();

LightningPeer::LightningPeer(NetworkInterface* interfacePtr)
    : Peer(interfacePtr) {}

LightningPeer::LightningPeer(const LightningPeer& rhs)
    : Peer(rhs) {}

LightningPeer::~LightningPeer() = default;

PaymentChannel LightningPeer::getChannel(interfaceId otherId) const {
    const PaymentChannel* channel = findChannel(otherId);
    return channel == nullptr ? PaymentChannel() : *channel;
}

const PaymentChannel* LightningPeer::findChannel(interfaceId otherId) const {
    for (const PaymentChannel& channel : channels) {
        if (channel.otherId == otherId) {
            return &channel;
        }
    }
    return nullptr;
}

PaymentChannel* LightningPeer::findMutableChannel(interfaceId otherId) {
    for (PaymentChannel& channel : channels) {
        if (channel.otherId == otherId) {
            return &channel;
        }
    }
    return nullptr;
}

void LightningPeer::addPaymentChannel(PaymentChannel channel) {
    if (PaymentChannel* current = findMutableChannel(channel.otherId)) {
        *current = channel;
        return;
    }
    channels.push_back(std::move(channel));
}

void LightningPeer::updateChannel(interfaceId target,
                                  int myFunds,
                                  int otherFunds,
                                  int myPending,
                                  int otherPending) {
    if (PaymentChannel* channel = findMutableChannel(target)) {
        channel->mySideBalance = myFunds;
        channel->otherSide = otherFunds;
        channel->mySidePending = myPending;
        channel->otherSidePending = otherPending;
        return;
    }

    PaymentChannel channel;
    channel.otherId = target;
    channel.mySideBalance = myFunds;
    channel.otherSide = otherFunds;
    channel.mySidePending = myPending;
    channel.otherSidePending = otherPending;
    channels.push_back(channel);
}

bool LightningPeer::hasCapacity(interfaceId target, int value) const {
    const PaymentChannel* channel = findChannel(target);
    if (channel == nullptr) {
        return false;
    }
    return channel->mySideBalance - channel->mySidePending >= value;
}

bool LightningPeer::reserveOutgoing(interfaceId target, int value) {
    PaymentChannel* channel = findMutableChannel(target);
    if (channel == nullptr || channel->mySideBalance - channel->mySidePending < value) {
        return false;
    }
    channel->mySidePending += value;
    return true;
}

bool LightningPeer::applyCapacityChange(interfaceId target, int value) {
    PaymentChannel* channel = findMutableChannel(target);
    if (channel == nullptr || fundsAvailable < value || value <= 0) {
        return false;
    }

    channel->mySideBalance += value;
    fundsAvailable -= value;
    ++capacityChanges;
    return true;
}

void LightningPeer::releaseOutgoing(interfaceId target, int value) {
    if (PaymentChannel* channel = findMutableChannel(target)) {
        channel->mySidePending = std::max(0, channel->mySidePending - value);
    }
}

void LightningPeer::settleOutgoing(interfaceId target, int value) {
    if (PaymentChannel* channel = findMutableChannel(target)) {
        channel->mySidePending = std::max(0, channel->mySidePending - value);
        channel->mySideBalance -= value;
        channel->otherSide += value;
    }
}

void LightningPeer::settleIncoming(interfaceId source, int value) {
    if (PaymentChannel* channel = findMutableChannel(source)) {
        channel->mySideBalance += value;
        channel->otherSide -= value;
    }
}

interfaceId LightningPeer::randomTarget() const {
    if (knownPeerIds.size() < 2) {
        return publicId();
    }

    interfaceId target = publicId();
    while (target == publicId()) {
        target = knownPeerIds[randMod(static_cast<int>(knownPeerIds.size()))];
    }
    return target;
}

bool LightningPeer::guardSubmit() const {
    return submitRate > 0 && randMod(submitRate) == 0;
}

json LightningPeer::buildPath(const json& tx) const {
    json path = json::array();
    const interfaceId source = tx.value("source", NO_PEER_ID);
    const interfaceId target = tx.value("target", NO_PEER_ID);
    if (source == NO_PEER_ID || target == NO_PEER_ID) {
        return path;
    }
    if (source == target) {
        path.push_back(source);
        return path;
    }

    if (!torus) {
        const auto computedPath = randomPath(source, target, topologyView);
        for (interfaceId hop : computedPath) {
            path.push_back(hop);
        }
        return path;
    }

    const auto computedPath = torusMonotonePath(
        source,
        target,
        topologyView,
        peerIndexById,
        std::max(1, height),
        std::max(1, width),
        false);
    for (interfaceId hop : computedPath) {
        path.push_back(hop);
    }
    return path;
}

json LightningPeer::buildRebalanceTransaction(interfaceId target, int value) {
    json tx;
    std::vector<interfaceId> cyclePath;

    if (torus) {
        cyclePath = torusMonotonePath(
            publicId(),
            target,
            topologyView,
            peerIndexById,
            std::max(1, height),
            std::max(1, width),
            true);
    } else {
        const std::pair<interfaceId, interfaceId> forbiddenEdge = {publicId(), target};
        cyclePath = randomPath(publicId(), target, topologyView, &forbiddenEdge);
    }

    if (cyclePath.size() < 2) {
        return json();
    }

    cyclePath.push_back(publicId());
    tx["id"] = ++localTransactionCounter;
    tx["value"] = value;
    tx["source"] = publicId();
    tx["target"] = publicId();
    tx["roundSubmitted"] = static_cast<int>(RoundManager::currentRound());
    tx["path"] = json::array();
    for (interfaceId hop : cyclePath) {
        tx["path"].push_back(hop);
    }
    tx["hopIndex"] = 0;
    tx["rebalance"] = true;
    return tx;
}

bool LightningPeer::rebalance(interfaceId source, interfaceId target, int value) {
    if (source != publicId()) {
        return false;
    }

    json tx = buildRebalanceTransaction(target, value);
    if (tx.is_null() || !tx.contains("path") || tx["path"].size() < 3) {
        return false;
    }

    const interfaceId firstHop = tx["path"][1].get<interfaceId>();
    if (!reserveOutgoing(firstHop, value)) {
        return false;
    }

    ++rebalancesAttempted;
    tx["hopIndex"] = 1;
    json msg;
    msg["type"] = "txAttempt";
    msg["tx"] = tx;
    unicastTo(msg, firstHop);
    return true;
}

void LightningPeer::queueTransaction(json tx, bool blockedByRebalance) {
    if (!tx.contains("walletReserved")) {
        tx["walletReserved"] = false;
    }
    if (!tx.contains("sourceCapacityChanged")) {
        tx["sourceCapacityChanged"] = false;
    }
    if (!tx.contains("rebalance")) {
        tx["rebalance"] = false;
    }
    if (!tx.contains("hopIndex")) {
        tx["hopIndex"] = 0;
    }
    PendingTransaction pending;
    pending.tx = std::move(tx);
    pending.blockedByRebalance = blockedByRebalance;
    pendingTransactions.push_back(std::move(pending));
}

void LightningPeer::submitTransaction() {
    if (knownPeerIds.size() < 2 || fundsAvailable < minPaymentAmount) {
        return;
    }

    const interfaceId target = randomTarget();
    const int upperBound = std::min(maxPaymentAmount, fundsAvailable);
    if (upperBound < minPaymentAmount) {
        return;
    }

    json tx;
    tx["id"] = ++localTransactionCounter;
    tx["value"] = uniformInt(minPaymentAmount, upperBound);
    tx["source"] = publicId();
    tx["target"] = target;
    tx["roundSubmitted"] = static_cast<int>(RoundManager::currentRound());
    tx["path"] = buildPath(tx);
    tx["hopIndex"] = 0;
    tx["rebalance"] = false;
    tx["walletReserved"] = false;
    tx["sourceCapacityChanged"] = false;

    if (!tx["path"].is_array() || tx["path"].size() < 2) {
        return;
    }

    queueTransaction(tx);
    ++submittedTransactions;
}

void LightningPeer::tryQueuedTransactions() {
    if (pendingTransactions.empty()) {
        return;
    }

    const size_t attemptsThisRound = pendingTransactions.size();
    for (size_t attempt = 0; attempt < attemptsThisRound; ++attempt) {
        PendingTransaction pending = pendingTransactions.front();
        pendingTransactions.pop_front();

        json tx = pending.tx;
        if (!tx.contains("path") || !tx["path"].is_array() || tx["path"].size() < 2) {
            continue;
        }

        const auto& path = tx["path"];
        const int currentHop = tx.value("hopIndex", 0);
        if (currentHop < 0 || static_cast<size_t>(currentHop + 1) >= path.size()) {
            continue;
        }
        if (path[currentHop].get<interfaceId>() != publicId()) {
            continue;
        }

        const int value = tx.value("value", 0);
        const interfaceId nextHop = path[currentHop + 1].get<interfaceId>();
        if (!reserveOutgoing(nextHop, value)) {
            if (!pending.blockedByRebalance) {
                pending.blockedByRebalance = rebalance(publicId(), nextHop, value);
            }
            if (!pending.blockedByRebalance) {
                tx["sourceCapacityChanged"] = applyCapacityChange(nextHop, value);
                pending.tx = tx;
            }
            pendingTransactions.push_back(std::move(pending));
            continue;
        }

        const bool rebalanceTx = tx.value("rebalance", false);
        const bool walletReserved = tx.value("walletReserved", false);
        const bool sourceCapacityChanged = tx.value("sourceCapacityChanged", false);
        if (!rebalanceTx && currentHop == 0 && !walletReserved && !sourceCapacityChanged) {
            fundsAvailable -= value;
            tx["walletReserved"] = true;
        }

        pending.blockedByRebalance = false;
        tx["hopIndex"] = currentHop + 1;
        json msg;
        msg["type"] = "txAttempt";
        msg["tx"] = tx;
        unicastTo(msg, nextHop);
    }
}

void LightningPeer::performComputation() {
    checkInStrm();

    if (guardSubmit()) {
        submitTransaction();
    }

    tryQueuedTransactions();
}

void LightningPeer::initParameters(const std::vector<Peer*>& _peers, json parameters) {
    const std::vector<LightningPeer*>& peers = reinterpret_cast<const std::vector<LightningPeer*>&>(_peers);
    if (!parameters.is_object() || parameters.is_null()) {
        return;
    }

    submitRate = parameters.value("submitRate", submitRate);
    torus = parameters.value("torus", torus);
    height = std::max(1, parameters.value("height", height));
    width = std::max(1, parameters.value("width", width));
    averageDegree = std::max(2, parameters.value("k", averageDegree));
    minInitialChannelCapacity = parameters.value("minInitialChannelCapacity", minInitialChannelCapacity);
    maxInitialChannelCapacity = parameters.value("maxInitialChannelCapacity", maxInitialChannelCapacity);
    minPaymentAmount = parameters.value("minPaymentAmount", minPaymentAmount);
    maxPaymentAmount = parameters.value("maxPaymentAmount", maxPaymentAmount);
    minPeerWalletBalance = parameters.value("minPeerWalletBalance", minPeerWalletBalance);
    maxPeerWalletBalance = parameters.value("maxPeerWalletBalance", maxPeerWalletBalance);
    if (parameters.contains("minInitialLocalBalanceRatio") &&
        parameters["minInitialLocalBalanceRatio"].is_number()) {
        const double ratioInput = parameters["minInitialLocalBalanceRatio"].get<double>();
        if (ratioInput <= 1.0) {
            minInitialLocalBalanceRatioPercent = static_cast<int>(std::round(ratioInput * 100.0));
        } else {
            minInitialLocalBalanceRatioPercent = static_cast<int>(std::round(ratioInput));
        }
    }
    minInitialLocalBalanceRatioPercent = std::clamp(minInitialLocalBalanceRatioPercent, 0, 50);

    if (maxInitialChannelCapacity < minInitialChannelCapacity) {
        std::swap(maxInitialChannelCapacity, minInitialChannelCapacity);
    }
    if (maxPaymentAmount < minPaymentAmount) {
        std::swap(maxPaymentAmount, minPaymentAmount);
    }
    if (maxPeerWalletBalance < minPeerWalletBalance) {
        std::swap(maxPeerWalletBalance, minPeerWalletBalance);
    }

    for (auto* peerPtr : peers) {
        peerPtr->submitRate = submitRate;
        peerPtr->torus = torus;
        peerPtr->height = height;
        peerPtr->width = width;
        peerPtr->averageDegree = averageDegree;
        peerPtr->minInitialChannelCapacity = minInitialChannelCapacity;
        peerPtr->maxInitialChannelCapacity = maxInitialChannelCapacity;
        peerPtr->minInitialLocalBalanceRatioPercent = minInitialLocalBalanceRatioPercent;
        peerPtr->minPaymentAmount = minPaymentAmount;
        peerPtr->maxPaymentAmount = maxPaymentAmount;
        peerPtr->minPeerWalletBalance = minPeerWalletBalance;
        peerPtr->maxPeerWalletBalance = maxPeerWalletBalance;
        peerPtr->channels.clear();
        peerPtr->pendingTransactions.clear();
        peerPtr->throughput = 0;
        peerPtr->txWait = 0;
        peerPtr->rebalancesAttempted = 0;
        peerPtr->successfulRebalances = 0;
        peerPtr->capacityChanges = 0;
        peerPtr->submittedTransactions = 0;
        peerPtr->funds = uniformInt(minPeerWalletBalance, maxPeerWalletBalance);
        peerPtr->fundsAvailable = peerPtr->funds;
        peerPtr->localTransactionCounter = 0;
        peerPtr->knownPeerIds.clear();
        peerPtr->peerIndexById.clear();
        peerPtr->topologyView.clear();
    }

    std::vector<interfaceId> orderedPeerIds;
    orderedPeerIds.reserve(peers.size());
    for (auto* peerPtr : peers) {
        orderedPeerIds.push_back(peerPtr->publicId());
    }

    std::map<interfaceId, std::set<interfaceId>> builtTopology;

    if (torus) {
        const size_t maxCells = static_cast<size_t>(height) * static_cast<size_t>(width);
        const size_t cellCount = std::min(peers.size(), maxCells);
        for (size_t idx = 0; idx < cellCount; ++idx) {
            const int row = static_cast<int>(idx) / width;
            const int col = static_cast<int>(idx) % width;
            const int right = row * width + ((col + 1) % width);
            const int up = ((row - 1 + height) % height) * width + col;
            if (static_cast<size_t>(right) < cellCount) {
                const interfaceId a = orderedPeerIds[idx];
                const interfaceId b = orderedPeerIds[right];
                builtTopology[a].insert(b);
                builtTopology[b].insert(a);
            }
            if (static_cast<size_t>(up) < cellCount) {
                const interfaceId a = orderedPeerIds[idx];
                const interfaceId b = orderedPeerIds[up];
                builtTopology[a].insert(b);
                builtTopology[b].insert(a);
            }
        }
    } else {
        if (peers.size() >= 2) {
            for (size_t i = 1; i < peers.size(); ++i) {
                const size_t parent = static_cast<size_t>(randMod(static_cast<int>(i)));
                const interfaceId a = orderedPeerIds[i];
                const interfaceId b = orderedPeerIds[parent];
                builtTopology[a].insert(b);
                builtTopology[b].insert(a);
            }
        }

        const int maxEdges = static_cast<int>((static_cast<long>(peers.size()) * static_cast<long>(peers.size() - 1)) / 2L);
        const int targetEdges = std::min(
            maxEdges,
            static_cast<int>((static_cast<long>(averageDegree) * static_cast<long>(peers.size())) / 2L));
        int currentEdges = 0;
        for (const auto& [node, neighbors] : builtTopology) {
            (void)node;
            currentEdges += static_cast<int>(neighbors.size());
        }
        currentEdges /= 2;
        while (currentEdges < targetEdges) {
            const interfaceId a = orderedPeerIds[randMod(static_cast<int>(orderedPeerIds.size()))];
            interfaceId b = a;
            while (b == a) {
                b = orderedPeerIds[randMod(static_cast<int>(orderedPeerIds.size()))];
            }
            if (builtTopology[a].count(b) > 0) {
                continue;
            }
            builtTopology[a].insert(b);
            builtTopology[b].insert(a);
            ++currentEdges;
        }
    }

    for (size_t i = 0; i < peers.size(); ++i) {
        LightningPeer* peerPtr = peers[i];
        peerPtr->knownPeerIds = orderedPeerIds;
        for (size_t j = 0; j < orderedPeerIds.size(); ++j) {
            peerPtr->peerIndexById[orderedPeerIds[j]] = j;
        }
        peerPtr->topologyView = builtTopology;
        peerPtr->channels.clear();
    }

    std::map<interfaceId, size_t> orderedIdToPeerIndex;
    for (size_t i = 0; i < orderedPeerIds.size(); ++i) {
        orderedIdToPeerIndex[orderedPeerIds[i]] = i;
    }

    for (const auto& [node, neighbors] : builtTopology) {
        for (interfaceId neighbor : neighbors) {
            if (node >= neighbor) {
                continue;
            }

            const int totalCapacity = uniformInt(minInitialChannelCapacity, maxInitialChannelCapacity);
            const int minLeftBalance =
                (totalCapacity * minInitialLocalBalanceRatioPercent + 99) / 100;
            const int maxLeftBalance = totalCapacity - minLeftBalance;
            const int leftBalance =
                (minLeftBalance > maxLeftBalance)
                    ? totalCapacity / 2
                    : uniformInt(minLeftBalance, maxLeftBalance);
            const int rightBalance = totalCapacity - leftBalance;

            auto leftPeerIt = orderedIdToPeerIndex.find(node);
            auto rightPeerIt = orderedIdToPeerIndex.find(neighbor);
            if (leftPeerIt == orderedIdToPeerIndex.end() ||
                rightPeerIt == orderedIdToPeerIndex.end()) {
                continue;
            }

            LightningPeer* leftPeer = peers[leftPeerIt->second];
            LightningPeer* rightPeer = peers[rightPeerIt->second];
            leftPeer->updateChannel(neighbor, leftBalance, rightBalance, 0, 0);
            rightPeer->updateChannel(node, rightBalance, leftBalance, 0, 0);
        }
    }
}

void LightningPeer::endOfRound(std::vector<Peer*>& _peers) {
    const std::vector<LightningPeer*>& peers = reinterpret_cast<const std::vector<LightningPeer*>&>(_peers);
    if (peers.empty()) {
        return;
    }

    int totalThroughput = 0;
    int totalTxWait = 0;
    int totalRebalancesAttempted = 0;
    int totalSuccessfulRebalances = 0;
    int totalCapacityChanges = 0;
    int totalSubmitted = 0;
    for (auto* peerPtr : peers) {
        totalThroughput += peerPtr->throughput;
        totalTxWait += peerPtr->txWait;
        totalRebalancesAttempted += peerPtr->rebalancesAttempted;
        totalSuccessfulRebalances += peerPtr->successfulRebalances;
        totalCapacityChanges += peerPtr->capacityChanges;
        totalSubmitted += peerPtr->submittedTransactions;
    }

    OutputWriter::pushValue("throughput", totalThroughput);
    OutputWriter::pushValue("latency", totalThroughput > 0 ? totalTxWait / totalThroughput : 0);
    OutputWriter::pushValue("rebalanceAttempts", totalRebalancesAttempted);
    OutputWriter::pushValue("successfulRebalances", totalSuccessfulRebalances);
    OutputWriter::pushValue("capacityChanges", totalCapacityChanges);
    OutputWriter::pushValue("submittedTransactions", totalSubmitted);
}

void LightningPeer::checkInStrm() {
    while (!inStreamEmpty()) {
        Packet packet = popInStream();
        json msg = packet.getMessage();

        const std::string messageType = msg.value("type", std::string());
        if (!msg.contains("tx") || !msg["tx"].is_object()) {
            continue;
        }

        json tx = msg["tx"];
        if (!tx.contains("path") || !tx["path"].is_array()) {
            continue;
        }

        const json& path = tx["path"];
        const int hopIndex = tx.value("hopIndex", 0);
        const int value = tx.value("value", 0);
        const bool rebalanceTx = tx.value("rebalance", false);

        if (messageType == "txAttempt") {
            if (hopIndex < 0 || static_cast<size_t>(hopIndex) >= path.size()) {
                continue;
            }

            if (path[hopIndex].get<interfaceId>() != publicId()) {
                continue;
            }

            if (static_cast<size_t>(hopIndex) == path.size() - 1) {
                if (hopIndex > 0) {
                    const interfaceId previousHop = path[hopIndex - 1].get<interfaceId>();
                    settleIncoming(previousHop, value);
                }
                if (!rebalanceTx) {
                    fundsAvailable += value;
                    ++throughput;
                    txWait += static_cast<int>(RoundManager::currentRound()) - tx.value("roundSubmitted", 0) + 1;
                }

                if (hopIndex > 0) {
                    const interfaceId previousHop = path[hopIndex - 1].get<interfaceId>();
                    tx["hopIndex"] = hopIndex;
                    json response;
                    response["type"] = "txSucceeded";
                    response["tx"] = tx;
                    unicastTo(response, previousHop);
                }
                continue;
            }

            const interfaceId nextHop = path[hopIndex + 1].get<interfaceId>();
            if (!reserveOutgoing(nextHop, value)) {
                if (hopIndex > 0) {
                    bool launchedRebalance = false;
                    if (!rebalanceTx) {
                        launchedRebalance = rebalance(publicId(), nextHop, value);
                    }
                    if (launchedRebalance) {
                        queueTransaction(tx, true);
                        continue;
                    }
                    if (!rebalanceTx && applyCapacityChange(nextHop, value)) {
                        queueTransaction(tx, false);
                        continue;
                    }
                    const interfaceId previousHop = path[hopIndex - 1].get<interfaceId>();
                    tx["hopIndex"] = hopIndex;
                    json response;
                    response["type"] = "txFailed";
                    response["tx"] = tx;
                    unicastTo(response, previousHop);
                } else {
                    bool launchedRebalance = false;
                    if (!rebalanceTx) {
                        launchedRebalance = rebalance(publicId(), nextHop, value);
                    }
                    if (!launchedRebalance) {
                        tx["sourceCapacityChanged"] = applyCapacityChange(nextHop, value);
                    }
                    queueTransaction(tx, launchedRebalance);
                }
                continue;
            }
            tx["hopIndex"] = hopIndex + 1;
            json forward;
            forward["type"] = "txAttempt";
            forward["tx"] = tx;
            unicastTo(forward, nextHop);
        } else if (messageType == "txSucceeded") {
            if (hopIndex <= 0 || static_cast<size_t>(hopIndex) >= path.size()) {
                continue;
            }

            const int currentIndex = hopIndex - 1;
            if (currentIndex < 0 || path[currentIndex].get<interfaceId>() != publicId()) {
                continue;
            }

            const interfaceId downstreamHop = path[hopIndex].get<interfaceId>();
            settleOutgoing(downstreamHop, value);
            if (currentIndex > 0) {
                const interfaceId upstreamHop = path[currentIndex - 1].get<interfaceId>();
                settleIncoming(upstreamHop, value);
                tx["hopIndex"] = currentIndex;
                json response;
                response["type"] = "txSucceeded";
                response["tx"] = tx;
                unicastTo(response, upstreamHop);
            } else if (rebalanceTx) {
                ++successfulRebalances;
            }
        } else if (messageType == "txFailed") {
            if (hopIndex <= 0 || static_cast<size_t>(hopIndex) >= path.size()) {
                continue;
            }

            const int currentIndex = hopIndex - 1;
            if (currentIndex < 0 || path[currentIndex].get<interfaceId>() != publicId()) {
                continue;
            }

            const interfaceId downstreamHop = path[hopIndex].get<interfaceId>();
            releaseOutgoing(downstreamHop, value);

            if (currentIndex == 0) {
                if (!rebalanceTx && tx.value("walletReserved", false)) {
                    fundsAvailable += value;
                    tx["walletReserved"] = false;
                }
                queueTransaction(tx, true);
                continue;
            }

            const interfaceId previousHop = path[currentIndex - 1].get<interfaceId>();
            tx["hopIndex"] = currentIndex;
            json response;
            response["type"] = "txFailed";
            response["tx"] = tx;
            unicastTo(response, previousHop);
        }
    }
}

} // namespace quantas
