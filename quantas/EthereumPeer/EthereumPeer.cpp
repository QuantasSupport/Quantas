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
#include <deque>
#include <functional>
#include <limits>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "EthereumPeer.hpp"
#include "PoWEthereum.hpp"
#include "../Common/Committee.hpp"
#include "../Common/LogWriter.hpp"
#include "../Common/ParasiteFault.hpp"
#include "../Common/RandomUtil.hpp"
#include "../Common/RoundManager.hpp"
#include "../Common/Abstract/NetworkInterfaceAbstract.hpp"

namespace quantas {

static bool registerEthereumPeer = []() {
    return PeerRegistry::registerPeerType(
        "EthereumPeer",
        [](interfaceId pubId) { return new EthereumPeer(new NetworkInterfaceAbstract(pubId)); });
}();

EthereumPeer::EthereumPeer(NetworkInterface* interfacePtr)
    : PoWPeer(interfacePtr) {}

EthereumPeer::EthereumPeer(const EthereumPeer& rhs)
    : PoWPeer(rhs) {}

void EthereumPeer::performComputation() {
    runProtocolStep();
}

void EthereumPeer::runProtocolStep(const std::vector<std::string>& overrideParents) {
    PoW* group = pow();
    if (!group) return;

    checkInStrm();

    if (guardSubmit()) {
        PendingTx pending = makeTransaction();
        _queue.push_back(pending);
        _knownTransactions.insert({pending.submitter, pending.id});
        broadcast(buildTransactionMessage(pending));
    }

    if (!guardMine()) return;

    PendingTx pending = _queue.front();
    _queue.pop_front();

    std::vector<std::string> parents = overrideParents.empty() ? getParents(*group) : overrideParents;
    if (parents.empty()) {
        parents.push_back("GENESIS");
    }

    const int minedRound = static_cast<int>(RoundManager::currentRound());
    std::string hash = std::to_string(publicId()) + ":" + std::to_string(pending.id) + ":" + std::to_string(minedRound);

    PoW::BlockRecord record = group->registerBlock(hash,
                                                      parents,
                                                      publicId(),
                                                      static_cast<int>(RoundManager::currentRound()),
                                                      minedRound,
                                                      !overrideParents.empty());

    broadcast(buildBlockMessage(record, record.parents, minedRound, pending));
}

std::vector<std::string> EthereumPeer::getParents(const PoW& group) const {
    std::vector<std::string> parents = group.parentsForNextBlock();
    if (parents.empty()) {
        parents.push_back("GENESIS");
    }
    return parents;
}

void EthereumPeer::initParameters(const std::vector<Peer*>& _peers, json parameters) {
    const std::vector<EthereumPeer*>& peers = reinterpret_cast<const std::vector<EthereumPeer*>&>(_peers);

    submitRate = parameters.value("submitRate", submitRate);

    int defaultRate = parameters.value("mineRate", _mineRate);
    int mineScaler = parameters.value("mineScaler", 1);
    if (mineScaler < 1) mineScaler = 1;

    std::vector<int> configuredRates(peers.size(), defaultRate);
    if (parameters.contains("mineRates") && parameters["mineRates"].is_array()) {
        const auto& arr = parameters["mineRates"];
        for (size_t i = 0; i < peers.size() && i < arr.size(); ++i) {
            if (arr[i].is_number_integer()) {
                configuredRates[i] = arr[i].get<int>();
            }
        }
    }

    int totalRate = 0;
    for (int rate : configuredRates) {
        totalRate += std::max(0, rate);
    }
    if (totalRate <= 0) totalRate = 0;

    for (size_t idx = 0; idx < peers.size(); ++idx) {
        int localRate = std::max(0, configuredRates[idx]);
        int others = std::max(0, totalRate - localRate);
        int denominator = std::max(1, others * mineScaler);
        int cappedRate = std::min(localRate, denominator);
        peers[idx]->_mineRate = cappedRate;
        peers[idx]->_mineDenominator = denominator;
    }

    Committee committee(0);
    for (auto* peerPtr : peers) {
        committee.addMember(peerPtr->publicId());
    }

    for (auto* peerPtr : peers) {
        if (!peerPtr->pow()) {
            peerPtr->setPoW(new PoWEthereum(new Committee(committee)));
        }
    }

    if (parameters.contains("parasiteFault") && parameters["parasiteFault"].is_object()) {
        const auto& parasiteCfg = parameters["parasiteFault"];
        int leadThreshold = parasiteCfg.value("leadThreshold", 1);
        std::set<size_t> parasiteIndices;
        if (parasiteCfg.contains("peerIndices") && parasiteCfg["peerIndices"].is_array()) {
            for (const auto& idxVal : parasiteCfg["peerIndices"]) {
                if (idxVal.is_number_integer()) {
                    parasiteIndices.insert(static_cast<size_t>(std::max(0, idxVal.get<int>())));
                }
            }
        } else {
            size_t count = static_cast<size_t>(parasiteCfg.value("count", 0));
            for (size_t i = 0; i < count && i < peers.size(); ++i) {
                parasiteIndices.insert(i);
            }
        }

        std::set<interfaceId> parasiteIds;
        for (size_t idx : parasiteIndices) {
            if (idx < peers.size()) {
                parasiteIds.insert(peers[idx]->publicId());
            }
        }

        for (size_t idx : parasiteIndices) {
            if (idx >= peers.size()) continue;
            EthereumPeer* attacker = peers[idx];
            std::set<interfaceId> collaborators = parasiteIds;
            collaborators.erase(attacker->publicId());
            attacker->faultManager.addFault(new ParasiteFault(leadThreshold, collaborators));
        }
    }
}

void EthereumPeer::endOfRound(std::vector<Peer*>& _peers) {
    const std::vector<EthereumPeer*>& peers = reinterpret_cast<const std::vector<EthereumPeer*>&>(_peers);
    
    if (peers.empty()) return;

    // Log only on the final round to reduce overhead
    // Can be changed to log every round by commenting out
    if (RoundManager::lastRound() > RoundManager::currentRound()) return;

    std::unordered_set<PoW*> uniqueLedgers;
    std::vector<PoW*> ledgers;
    ledgers.reserve(peers.size());
    for (auto* peerPtr : peers) {
        PoW* ledger = peerPtr->pow();
        if (!ledger) continue;
        if (uniqueLedgers.insert(ledger).second) {
            ledgers.push_back(ledger);
        }
    }
    if (ledgers.empty()) return;

    struct BlockAggregate {
        std::string hash;
        std::vector<std::string> parents;
        bool parasite = false;
        size_t seen = 0;
    };

    std::unordered_map<std::string, BlockAggregate> aggregatedBlocks;
    std::unordered_map<std::string, std::unordered_set<std::string>> aggregatedChildren;

    const size_t ledgerCount = ledgers.size();

    for (auto* ledger : ledgers) {
        for (const auto& record : ledger->allBlocks()) {
            auto& aggregate = aggregatedBlocks[record.hash];
            if (aggregate.seen == 0) {
                aggregate.hash = record.hash;
                aggregate.parents = record.parents;
            } else {
                for (const auto& parent : record.parents) {
                    if (std::find(aggregate.parents.begin(), aggregate.parents.end(), parent) == aggregate.parents.end()) {
                        aggregate.parents.push_back(parent);
                    }
                }
            }
            aggregate.parasite = aggregate.parasite || record.parasite;
            ++aggregate.seen;

            aggregatedChildren[record.hash];
            for (const auto& parent : record.parents) {
                aggregatedChildren[parent].insert(record.hash);
            }
        }
    }


    if (!aggregatedBlocks.count("GENESIS")) {
        BlockAggregate genesis;
        genesis.hash = "GENESIS";
        genesis.seen = ledgerCount;
        aggregatedBlocks.emplace("GENESIS", std::move(genesis));
    }
    aggregatedChildren["GENESIS"];

    std::unordered_map<std::string, int> computedHeight;
    std::deque<std::string> queue;
    queue.push_back("GENESIS");
    computedHeight["GENESIS"] = 0;

    while (!queue.empty()) {
        const std::string current = queue.front();
        queue.pop_front();
        const int baseHeight = computedHeight[current];
        auto childIt = aggregatedChildren.find(current);
        if (childIt == aggregatedChildren.end()) continue;
        for (const auto& child : childIt->second) {
            if (!aggregatedBlocks.count(child)) continue;
            const int candidateHeight = baseHeight + 1;
            auto [entry, inserted] = computedHeight.emplace(child, candidateHeight);
            if (inserted) {
                queue.push_back(child);
            } else if (candidateHeight > entry->second) {
                entry->second = candidateHeight;
                queue.push_back(child);
            }
        }
    }

    // Pre-compute subtree weights for GHOST decisions.
    std::unordered_map<std::string, int> weightMemo;
    std::unordered_set<std::string> weightActive;
    std::function<int(const std::string&)> subtreeWeight = [&](const std::string& hash) -> int {
        if (weightActive.count(hash)) {
            return 0;
        }
        auto it = weightMemo.find(hash);
        if (it != weightMemo.end()) return it->second;
        weightActive.insert(hash);
        int total = 1;
        auto childIt = aggregatedChildren.find(hash);
        if (childIt != aggregatedChildren.end()) {
            for (const auto& child : childIt->second) {
                if (!aggregatedBlocks.count(child)) continue;
                total += subtreeWeight(child);
            }
        }
        weightActive.erase(hash);
        weightMemo[hash] = total;
        return total;
    };

    // Build the GHOST chain (heaviest subtree at every fork).
    std::vector<std::string> ghostPath;
    ghostPath.push_back("GENESIS");
    std::string cursor = "GENESIS";
    while (true) {
        auto it = aggregatedChildren.find(cursor);
        if (it == aggregatedChildren.end() || it->second.empty()) break;
        std::string bestChild;
        int bestWeight = -1;
        int bestHeight = -1;
        for (const auto& child : it->second) {
            if (!aggregatedBlocks.count(child)) continue;
            int weight = subtreeWeight(child);
            int height = 0;
            if (auto hIt = computedHeight.find(child); hIt != computedHeight.end()) {
                height = hIt->second;
            }
            if (weight > bestWeight ||
                (weight == bestWeight && height > bestHeight) ||
                (weight == bestWeight && height == bestHeight && child < bestChild)) {
                bestChild = child;
                bestWeight = weight;
                bestHeight = height;
            }
        }
        if (bestChild.empty()) break;
        ghostPath.push_back(bestChild);
        cursor = bestChild;
    }

    const int longestChain = static_cast<int>(ghostPath.size()) - 1;
    const std::string bestHash = ghostPath.back();

    std::string commonRoot = "GENESIS";
    int commonRootHeight = 0;
    for (const auto& [hash, aggregate] : aggregatedBlocks) {
        if (aggregate.seen != ledgerCount) continue;
        auto itHeight = computedHeight.find(hash);
        if (itHeight == computedHeight.end()) continue;
        if (itHeight->second > commonRootHeight ||
            (itHeight->second == commonRootHeight && hash < commonRoot)) {
            commonRoot = hash;
            commonRootHeight = itHeight->second;
        }
    }

    size_t parasitesOnCommonPath = 0;
    if (!ghostPath.empty()) {
        for (size_t idx = 1; idx < ghostPath.size(); ++idx) {
            const auto& current = ghostPath[idx];
            auto itBlock = aggregatedBlocks.find(current);
            if (itBlock != aggregatedBlocks.end() && itBlock->second.parasite) {
                ++parasitesOnCommonPath;
            }
        }
    }

    const int longestFromCommonRoot = std::max(0, longestChain - commonRootHeight);

    size_t totalBlocks = aggregatedBlocks.size();
    if (aggregatedBlocks.count("GENESIS") && totalBlocks > 0) {
        --totalBlocks;
    }

    size_t forkPoints = 0;
    std::unordered_map<int, size_t> forkLengthCounts;
    json forkLocations = json::array();
    std::unordered_map<std::string, int> depthMemo;
    std::unordered_set<std::string> depthActive;

    std::function<int(const std::string&)> longestDepthFrom = [&](const std::string& hash) -> int {
        if (depthActive.count(hash)) {
            return 0;
        }
        auto memoIt = depthMemo.find(hash);
        if (memoIt != depthMemo.end()) return memoIt->second;

        depthActive.insert(hash);
        int best = 0;
        auto childIt = aggregatedChildren.find(hash);
        if (childIt != aggregatedChildren.end()) {
            for (const auto& child : childIt->second) {
                if (!computedHeight.count(child)) continue;
                best = std::max(best, 1 + longestDepthFrom(child));
            }
        }
        depthActive.erase(hash);
        depthMemo[hash] = best;
        return best;
    };

    for (const auto& [hash, children] : aggregatedChildren) {
        if (children.size() <= 1) continue;
        auto baseHeightIt = computedHeight.find(hash);
        if (baseHeightIt == computedHeight.end()) continue;

        std::vector<std::pair<std::string, int>> branchLengths;
        branchLengths.reserve(children.size());
        for (const auto& child : children) {
            if (!computedHeight.count(child)) continue;
            const int branchLen = 1 + longestDepthFrom(child);
            branchLengths.emplace_back(child, branchLen);
        }
        if (branchLengths.size() <= 1) {
            continue;
        }

        ++forkPoints;
        int bestWeight = -1;
        for (const auto& entry : branchLengths) {
            bestWeight = std::max(bestWeight, subtreeWeight(entry.first));
        }

        for (const auto& entry : branchLengths) {
            const std::string& child = entry.first;
            const int len = entry.second;
            const int weight = subtreeWeight(child);
            if (weight >= bestWeight) continue;
            forkLengthCounts[len]++;
            forkLocations.push_back({
                {"height", baseHeightIt->second},
                {"length", len}
            });
        }
    }

    json forkSummary = json::object();
    for (const auto& [length, count] : forkLengthCounts) {
        forkSummary[std::to_string(length)] = count;
    }

    LogWriter::pushValue("minedBlocks", static_cast<double>(totalBlocks));
    LogWriter::pushValue("dagLongestChainLength", static_cast<double>(longestChain));
    LogWriter::pushValue("dagCommonRootHeight", static_cast<double>(commonRootHeight));
    LogWriter::pushValue("dagCommonRootParasites", static_cast<double>(parasitesOnCommonPath));
    LogWriter::pushValue("dagTotalForkPoints", static_cast<double>(forkPoints));
    LogWriter::pushValue("dagForks", forkSummary);
    if (!forkLocations.empty()) {
        LogWriter::pushValue("dagForkLocations", forkLocations);
    }
}

void EthereumPeer::checkInStrm() {
    PoW* group = pow();
    if (!group) return;

    while (!inStreamEmpty()) {
        Packet packet = popInStream();
        json msg = packet.getMessage();
        if (!msg.contains("type") || msg["type"] != "PoW") continue;

        const std::string messageType = msg.value("messageType", std::string());
        if (messageType == "transaction") {
            const json& txJson = msg["transaction"];
            int txId = txJson.value("id", -1);
            interfaceId submitter = txJson.value("submitter", msg.value("from_id", NO_PEER_ID));
            if (txId < 0 || submitter == NO_PEER_ID) continue;
            std::pair<interfaceId, int> key{submitter, txId};
            if (_knownTransactions.insert(key).second) {
                PendingTx pending;
                pending.id = txId;
                pending.roundSubmitted = txJson.value("roundSubmitted", -1);
                pending.submitter = submitter;
                _queue.push_back(pending);
            }
        } else if (messageType == "block") {
            const json& blkJson = msg["block"];
            std::string hash = blkJson.value("hash", std::string());

            std::vector<std::string> parents;
            if (blkJson.contains("parents")) {
                for (const auto& parent : blkJson["parents"]) {
                    parents.push_back(parent.get<std::string>());
                }
            }

            const interfaceId miner = blkJson.value("miner", NO_PEER_ID);
            const int minedRound = blkJson.value("roundMined", static_cast<int>(RoundManager::currentRound()));
            if (hash.empty()) {
                hash = std::to_string(miner) + ":" + (parents.empty() ? std::string("GENESIS") : parents.front());
            }

            group->registerBlock(hash,
                                 parents,
                                 miner,
                                 static_cast<int>(RoundManager::currentRound()),
                                 minedRound,
                                 blkJson.value("parasite", false));

            if (blkJson.contains("transaction")) {
                const json& txJson = blkJson["transaction"];
                int txId = txJson.value("id", -1);
                interfaceId submitter = txJson.value("submitter", blkJson.value("miner", NO_PEER_ID));
                if (txId >= 0 && submitter != NO_PEER_ID) {
                    std::pair<interfaceId, int> key{submitter, txId};
                    _knownTransactions.insert(key);
                    for (auto itTx = _queue.begin(); itTx != _queue.end(); ++itTx) {
                        if (itTx->id == txId && itTx->submitter == submitter) {
                            _queue.erase(itTx);
                            break;
                        }
                    }
                }
            }
        }
    }
}

bool EthereumPeer::guardSubmit() const {
    return submitRate > 0 && randMod(submitRate) == 0;
}

bool EthereumPeer::guardMine() const {
    if (_mineRate <= 0) return false;
    if (_mineDenominator <= 0) return false;
    if (_queue.empty()) return false;
    return randMod(_mineDenominator) < _mineRate;
}

EthereumPeer::PendingTx EthereumPeer::makeTransaction() {
    PendingTx pending;
    pending.roundSubmitted = static_cast<int>(RoundManager::currentRound());
    pending.submitter = publicId();
    pending.id = ++_localSubmitted;
    return pending;
}

json EthereumPeer::buildTransactionMessage(const PendingTx& pending) const {
    return json{
        {"type", "PoW"},
        {"powId", 0},
        {"messageType", "transaction"},
        {"transaction", {
            {"id", pending.id},
            {"roundSubmitted", pending.roundSubmitted},
            {"submitter", pending.submitter}
        }},
        {"from_id", publicId()}
    };
}

json EthereumPeer::buildBlockMessage(const PoW::BlockRecord& record,
                                        const std::vector<std::string>& parents,
                                        int minedRound,
                                        const PendingTx& pending) const {
    json parentArray = json::array();
    for (const auto& parent : parents) {
        parentArray.push_back(parent);
    }
    return json{
        {"type", "PoW"},
        {"powId", 0},
        {"messageType", "block"},
        {"block", {
            {"hash", record.hash},
            {"parents", parentArray},
            {"miner", publicId()},
            {"length", record.height},
            {"parasite", record.parasite},
            {"transaction", {
                {"id", pending.id},
                {"roundSubmitted", pending.roundSubmitted},
                {"submitter", pending.submitter}
            }},
            {"roundMined", minedRound}
        }},
        {"from_id", publicId()}
    };
}

}
