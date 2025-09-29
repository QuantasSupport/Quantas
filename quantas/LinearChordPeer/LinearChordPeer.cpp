/*
Copyright 2024

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include "LinearChordPeer.hpp"

#include <algorithm>
#include <string>
#include <utility>

#include "../Common/Abstract/NetworkInterfaceAbstract.hpp"
#include "../Common/Json.hpp"
#include "../Common/LogWriter.hpp"
#include "../Common/RandomUtil.hpp"
#include "../Common/RoundManager.hpp"

namespace quantas {

static bool registerLinearChordPeer = []() {
    return PeerRegistry::registerPeerType(
        "LinearChordPeer",
        [](interfaceId pubId) { return new LinearChordPeer(new NetworkInterfaceAbstract(pubId)); });
}();

int LinearChordPeer::s_nextTransactionId = 1;

LinearChordPeer::LinearChordPeer(NetworkInterface* interfacePtr)
    : Peer(interfacePtr) {}

LinearChordPeer::LinearChordPeer(const LinearChordPeer& rhs)
    : Peer(rhs),
      _ringOrder(rhs._ringOrder),
      _indexById(rhs._indexById),
      _fingers(rhs._fingers),
      _selfIndex(rhs._selfIndex),
      _initialized(rhs._initialized),
      _requestsSatisfied(rhs._requestsSatisfied),
      _totalHops(rhs._totalHops),
      _totalLatency(rhs._totalLatency) {}

void LinearChordPeer::initParameters(const std::vector<Peer*>& peers, json /*parameters*/) {
    std::vector<interfaceId> ringOrder;
    ringOrder.reserve(peers.size());
    for (const auto* peerPtr : peers) {
        ringOrder.push_back(peerPtr->publicId());
    }
    std::sort(ringOrder.begin(), ringOrder.end());

    std::unordered_map<interfaceId, size_t> indexById;
    for (size_t idx = 0; idx < ringOrder.size(); ++idx) {
        indexById[ringOrder[idx]] = idx;
    }

    for (auto* basePtr : peers) {
        auto* peerPtr = static_cast<LinearChordPeer*>(basePtr);
        peerPtr->_ringOrder = ringOrder;
        peerPtr->_indexById = indexById;
        auto it = indexById.find(peerPtr->publicId());
        peerPtr->_selfIndex = (it != indexById.end()) ? it->second : 0;
        peerPtr->_requestsSatisfied = 0;
        peerPtr->_totalHops = 0;
        peerPtr->_totalLatency = 0;
        peerPtr->_initialized = true;
        peerPtr->buildFingerTable();
    }
}

void LinearChordPeer::performComputation() {
    if (!_initialized) return;
    checkInStrm();
}

void LinearChordPeer::checkInStrm() {
    while (!inStreamEmpty()) {
        Packet packet = popInStream();
        json msg = packet.getMessage();
        if (!msg.contains("type") || msg["type"] != "LinearChord") continue;
        const std::string messageType = msg.value("messageType", std::string());
        if (messageType == "lookup") {
            handleLookup(std::move(msg));
        }
    }
}

void LinearChordPeer::handleLookup(json msg) {
    interfaceId target = msg.value("targetId", NO_PEER_ID);
    if (target == NO_PEER_ID) return;

    if (target == publicId()) {
        ++_requestsSatisfied;
        _totalHops += msg.value("hops", 0);
        int submitted = msg.value("roundSubmitted", static_cast<int>(RoundManager::currentRound()));
        _totalLatency += static_cast<int>(RoundManager::currentRound()) - submitted;
        return;
    }

    const std::set<interfaceId> neighborSet = neighbors();
    interfaceId nextHop = selectFinger(target, neighborSet);
    if (nextHop == NO_PEER_ID) {
        nextHop = chooseClockwiseNeighbor(target, neighborSet);
    }
    if (nextHop == NO_PEER_ID) return;

    dispatchLookup(std::move(msg), nextHop, neighborSet);
}

void LinearChordPeer::submitLookup(int transactionId) {
    if (!_initialized || _ringOrder.empty()) return;

    interfaceId target = pickRandomTarget();
    json msg = makeLookupTemplate(target, transactionId);
    if (target == publicId()) {
        ++_requestsSatisfied;
        return;
    }

    const std::set<interfaceId> neighborSet = neighbors();
    interfaceId nextHop = selectFinger(target, neighborSet);
    if (nextHop == NO_PEER_ID) {
        nextHop = chooseClockwiseNeighbor(target, neighborSet);
    }
    if (nextHop == NO_PEER_ID) return;

    dispatchLookup(std::move(msg), nextHop, neighborSet);
}

json LinearChordPeer::makeLookupTemplate(interfaceId target, int transactionId) const {
    json msg = {
        {"type", "LinearChord"},
        {"messageType", "lookup"},
        {"transactionId", transactionId},
        {"originId", publicId()},
        {"targetId", target},
        {"roundSubmitted", static_cast<int>(RoundManager::currentRound())},
        {"hops", 0}
    };
    return msg;
}

interfaceId LinearChordPeer::pickRandomTarget() const {
    if (_ringOrder.empty()) return publicId();
    int index = randMod(static_cast<int>(_ringOrder.size()));
    interfaceId candidate = _ringOrder[static_cast<size_t>(index)];
    if (candidate == publicId() && _ringOrder.size() > 1) {
        size_t nextIndex = (static_cast<size_t>(index) + 1) % _ringOrder.size();
        candidate = _ringOrder[nextIndex];
    }
    return candidate;
}

interfaceId LinearChordPeer::selectFinger(interfaceId target, const std::set<interfaceId>& neighborSet) const {
    const size_t ringSize = _ringOrder.size();
    if (ringSize <= 1) return NO_PEER_ID;

    auto targetIt = _indexById.find(target);
    if (targetIt == _indexById.end()) return NO_PEER_ID;
    size_t distanceToTarget = (targetIt->second + ringSize - _selfIndex) % ringSize;
    if (distanceToTarget == 0) return NO_PEER_ID;

    interfaceId best = NO_PEER_ID;
    size_t bestSkip = 0;

    for (const auto& entry : _fingers) {
        if (!neighborSet.count(entry.nodeId)) continue;
        if (entry.skipNormalized == 0) continue;
        if (entry.skipNormalized <= distanceToTarget && entry.skipNormalized > bestSkip) {
            best = entry.nodeId;
            bestSkip = entry.skipNormalized;
        }
    }

    return best;
}

interfaceId LinearChordPeer::chooseClockwiseNeighbor(interfaceId target,
                                                        const std::set<interfaceId>& neighborSet) const {
    const size_t ringSize = _ringOrder.size();
    if (ringSize <= 1 || neighborSet.empty()) return NO_PEER_ID;

    auto targetIt = _indexById.find(target);
    if (targetIt == _indexById.end()) return NO_PEER_ID;
    size_t targetDistance = (targetIt->second + ringSize - _selfIndex) % ringSize;
    if (targetDistance == 0) return NO_PEER_ID;

    interfaceId best = NO_PEER_ID;
    size_t bestDistance = ringSize;

    for (interfaceId neighbor : neighborSet) {
        auto it = _indexById.find(neighbor);
        if (it == _indexById.end()) continue;
        size_t distance = (it->second + ringSize - _selfIndex) % ringSize;
        if (distance == 0) continue;
        if (distance <= targetDistance && distance < bestDistance) {
            bestDistance = distance;
            best = neighbor;
        }
    }
    if (best != NO_PEER_ID) return best;

    for (interfaceId neighbor : neighborSet) {
        auto it = _indexById.find(neighbor);
        if (it == _indexById.end()) continue;
        size_t distance = (it->second + ringSize - _selfIndex) % ringSize;
        if (distance == 0) continue;
        if (distance < bestDistance) {
            bestDistance = distance;
            best = neighbor;
        }
    }
    return best;
}

void LinearChordPeer::dispatchLookup(json msg,
                                        interfaceId nextHop,
                                        const std::set<interfaceId>& neighborSet) {
    if (nextHop == NO_PEER_ID || nextHop == publicId()) return;
    if (!neighborSet.count(nextHop)) return;
    msg["hops"] = msg.value("hops", 0) + 1;
    msg["lastHop"] = publicId();
    unicastTo(msg, nextHop);
}

void LinearChordPeer::buildFingerTable() {
    _fingers.clear();
    const size_t ringSize = _ringOrder.size();
    if (ringSize <= 1) return;

    size_t maxSkip = ringSize - 1;
    size_t bits = 0;
    while ((static_cast<size_t>(1) << bits) <= maxSkip) {
        ++bits;
    }

    for (size_t k = 0; k < bits; ++k) {
        size_t skip = static_cast<size_t>(1) << k;
        size_t normalized = skip % ringSize;
        if (normalized == 0) continue;
        size_t idx = (_selfIndex + skip) % ringSize;
        interfaceId nodeId = _ringOrder[idx];
        if (nodeId == publicId()) continue;
        if (!_fingers.empty() && _fingers.back().nodeId == nodeId) continue;
        FingerEntry entry;
        entry.nodeId = nodeId;
        entry.ringIndex = idx;
        entry.skip = skip;
        entry.skipNormalized = normalized;
        _fingers.push_back(entry);
    }
}

void LinearChordPeer::endOfRound(std::vector<Peer*>& peers) {
    if (peers.empty()) return;

    std::vector<LinearChordPeer*> typed;
    typed.reserve(peers.size());
    for (auto* basePtr : peers) {
        typed.push_back(static_cast<LinearChordPeer*>(basePtr));
    }

    if (!typed.empty()) {
        int idx = randMod(static_cast<int>(typed.size()));
        typed[static_cast<size_t>(idx)]->submitLookup(s_nextTransactionId++);
    }

    long long totalSatisfied = 0;
    long long totalHops = 0;
    long long totalLatency = 0;

    for (auto* peerPtr : typed) {
        totalSatisfied += peerPtr->_requestsSatisfied;
        totalHops += peerPtr->_totalHops;
        totalLatency += peerPtr->_totalLatency;
    }

    if (totalSatisfied > 0) {
        LogWriter::pushValue("linearChordAverageHops", static_cast<double>(totalHops) / totalSatisfied);
        LogWriter::pushValue("linearChordAverageLatency", static_cast<double>(totalLatency) / totalSatisfied);
    }
    LogWriter::pushValue("linearChordRequestsSatisfied", static_cast<double>(totalSatisfied));
}

} // namespace quantas
