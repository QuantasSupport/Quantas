/*
Copyright 2024

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include "KademliaPeer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "../Common/Abstract/NetworkInterfaceAbstract.hpp"
#include "../Common/Concrete/NetworkInterfaceConcrete.hpp"
#include "../Common/LogWriter.hpp"
#include "../Common/RandomUtil.hpp"
#include "../Common/RoundManager.hpp"

namespace quantas {

namespace {
std::uint64_t xorDistance(interfaceId lhs, interfaceId rhs) {
    return static_cast<std::uint64_t>(static_cast<std::uint64_t>(lhs) ^ static_cast<std::uint64_t>(rhs));
}
}  // namespace

static bool registerKademliaAbstract = []() {
    return PeerRegistry::registerPeerType(
        "KademliaPeer",
        [](interfaceId pubId) { return new KademliaPeer(new NetworkInterfaceAbstract(pubId)); });
}();

static bool registerKademliaConcrete = []() {
    return PeerRegistry::registerPeerType(
        "KademliaPeerConcrete",
        [](interfaceId /*pubId*/) { return new KademliaPeer(new NetworkInterfaceConcrete()); });
}();

int KademliaPeer::s_currentTransactionId = 1;
std::vector<interfaceId> KademliaPeer::s_allPeerIds;
int KademliaPeer::s_binaryIdSize = 1;

KademliaPeer::~KademliaPeer() = default;

KademliaPeer::KademliaPeer(const KademliaPeer& rhs)
    : Peer(rhs),
      _binaryIdSize(rhs._binaryIdSize),
      _binaryId(rhs._binaryId),
      _allPeerIds(rhs._allPeerIds),
      _fingers(rhs._fingers),
      _lastNeighborFingerprint(rhs._lastNeighborFingerprint),
      _requestsSatisfied(rhs._requestsSatisfied),
      _totalHops(rhs._totalHops),
      _latency(rhs._latency),
      _alive(rhs._alive),
      _initialized(rhs._initialized) {}

KademliaPeer::KademliaPeer(NetworkInterface* networkInterface)
    : Peer(networkInterface) {}

void KademliaPeer::initParameters(const std::vector<Peer*>& peers, json /*parameters*/) {
    s_allPeerIds.clear();
    s_allPeerIds.reserve(peers.size());
    for (const auto* base : peers) {
        s_allPeerIds.push_back(base->publicId());
    }

    size_t peerCount = std::max<size_t>(1, s_allPeerIds.size());
    s_binaryIdSize = static_cast<int>(std::ceil(std::log2(static_cast<double>(peerCount))));
    if (s_binaryIdSize <= 0) s_binaryIdSize = 1;

    const int maxBits = static_cast<int>(sizeof(std::uint64_t) * 8);
    if (s_binaryIdSize > maxBits) {
        s_binaryIdSize = maxBits;
    }

    for (auto* base : peers) {
        auto* peer = static_cast<KademliaPeer*>(base);
        peer->applyGlobalParameters();
        peer->_requestsSatisfied = 0;
        peer->_totalHops = 0;
        peer->_latency = 0;
        peer->_fingers.clear();
        peer->_lastNeighborFingerprint = 0;
    }
}

void KademliaPeer::performComputation() {
    if (!_alive) return;

    ensureInitialized();
    if (!_initialized) return;

    const std::set<interfaceId> neighborSet = neighbors();
    const size_t fingerprint = neighborFingerprint(neighborSet);
    if (_fingers.empty() || fingerprint != _lastNeighborFingerprint) {
        rebuildFingerTable(neighborSet);
    }

    checkInStrm();
}

void KademliaPeer::checkInStrm() {
    while (!inStreamEmpty()) {
        Packet packet = popInStream();
        json message = packet.getMessage();
        if (!message.is_object()) continue;
        if (message.value("type", std::string()) != "Kademlia") continue;
        if (message.value("messageType", std::string()) != "lookup") continue;
        handleLookup(std::move(message));
    }
}

void KademliaPeer::handleLookup(json msg) {
    interfaceId targetId = msg.value("targetId", NO_PEER_ID);
    if (targetId == NO_PEER_ID) return;

    if (targetId == publicId()) {
        ++_requestsSatisfied;
        _totalHops += msg.value("hops", 0);
        int submitted = msg.value("roundSubmitted", static_cast<int>(RoundManager::currentRound()));
        _latency += static_cast<int>(RoundManager::currentRound()) - submitted;
        return;
    }

    const std::set<interfaceId> neighborSet = neighbors();
    if (neighborSet.empty()) return;

    std::string targetBinary = msg.value("targetBinaryId", std::string());
    if (targetBinary.empty()) {
        targetBinary = getBinaryId(targetId);
        msg["targetBinaryId"] = targetBinary;
    }

    interfaceId nextHop = findRoute(targetBinary, targetId, neighborSet);
    if (nextHop == NO_PEER_ID || nextHop == publicId()) {
        return;
    }

    msg["hops"] = msg.value("hops", 0) + 1;
    msg["lastHop"] = publicId();
    unicastTo(std::move(msg), nextHop);
}

void KademliaPeer::submitLookup(int transactionId) {
    if (!_initialized) return;

    interfaceId targetId = publicId();
    if (!_allPeerIds.empty()) {
        int index = randMod(static_cast<int>(_allPeerIds.size()));
        targetId = _allPeerIds[static_cast<size_t>(index)];
    }

    std::string targetBinary = getBinaryId(targetId);
    json msg = makeLookupMessage(targetId, targetBinary, transactionId);

    if (targetId == publicId()) {
        ++_requestsSatisfied;
        return;
    }

    const std::set<interfaceId> neighborSet = neighbors();
    if (neighborSet.empty()) return;

    interfaceId nextHop = findRoute(targetBinary, targetId, neighborSet);
    if (nextHop == NO_PEER_ID || nextHop == publicId()) return;

    msg["hops"] = msg.value("hops", 0) + 1;
    msg["lastHop"] = publicId();
    unicastTo(std::move(msg), nextHop);
}

json KademliaPeer::makeLookupMessage(interfaceId targetId,
                                     const std::string& targetBinaryId,
                                     int transactionId) const {
    json msg = {
        {"type", "Kademlia"},
        {"messageType", "lookup"},
        {"transactionId", transactionId},
        {"originId", publicId()},
        {"targetId", targetId},
        {"targetBinaryId", targetBinaryId},
        {"roundSubmitted", static_cast<int>(RoundManager::currentRound())},
        {"hops", 0}
    };
    return msg;
}

void KademliaPeer::applyGlobalParameters() {
    if (s_binaryIdSize <= 0 || s_allPeerIds.empty()) return;
    _binaryIdSize = s_binaryIdSize;
    _allPeerIds = s_allPeerIds;
    _binaryId = getBinaryId(publicId());
    _initialized = true;
}

void KademliaPeer::ensureInitialized() {
    if (!_initialized) {
        applyGlobalParameters();
    } else {
        if (s_binaryIdSize > 0 && _binaryIdSize != s_binaryIdSize) {
            _binaryIdSize = s_binaryIdSize;
            _binaryId = getBinaryId(publicId());
        }
        if (!s_allPeerIds.empty()) {
            _allPeerIds = s_allPeerIds;
        }
    }
}

std::string KademliaPeer::getBinaryId(interfaceId id) const {
    if (_binaryIdSize <= 0) return std::string();
    std::string result;
    result.reserve(static_cast<size_t>(_binaryIdSize));
    std::uint64_t value = static_cast<std::uint64_t>(id);
    for (int bit = _binaryIdSize - 1; bit >= 0; --bit) {
        if (bit >= static_cast<int>(sizeof(std::uint64_t) * 8)) {
            result.push_back('0');
            continue;
        }
        std::uint64_t mask = static_cast<std::uint64_t>(1) << bit;
        result.push_back((value & mask) ? '1' : '0');
    }
    return result;
}

interfaceId KademliaPeer::findRoute(const std::string& targetBinaryId,
                                    interfaceId targetId,
                                    const std::set<interfaceId>& neighborSet) const {
    if (targetBinaryId.empty() || _binaryId.empty()) {
        return selectClosestByDistance(targetId, neighborSet);
    }

    int group = firstDifferentBit(targetBinaryId, _binaryId);
    if (group < 0) {
        return selectClosestByDistance(targetId, neighborSet);
    }

    interfaceId finger = selectFingerForGroup(group, neighborSet);
    if (finger != NO_PEER_ID) {
        return finger;
    }

    return selectClosestByDistance(targetId, neighborSet);
}

interfaceId KademliaPeer::selectFingerForGroup(int group,
                                               const std::set<interfaceId>& neighborSet) const {
    std::vector<interfaceId> candidates;
    for (const auto& finger : _fingers) {
        if (finger.group != group) continue;
        if (!neighborSet.count(finger.Id)) continue;
        candidates.push_back(finger.Id);
    }

    if (candidates.empty()) return NO_PEER_ID;
    if (candidates.size() == 1) return candidates.front();

    int index = randMod(static_cast<int>(candidates.size()));
    return candidates[static_cast<size_t>(index)];
}

interfaceId KademliaPeer::selectClosestByDistance(interfaceId targetId,
                                                  const std::set<interfaceId>& neighborSet) const {
    std::uint64_t selfDistance = xorDistance(publicId(), targetId);
    std::uint64_t bestDistance = selfDistance;
    interfaceId best = NO_PEER_ID;

    for (interfaceId neighbor : neighborSet) {
        if (neighbor == publicId()) continue;
        std::uint64_t distance = xorDistance(neighbor, targetId);
        if (distance < bestDistance) {
            bestDistance = distance;
            best = neighbor;
        }
    }

    return best;
}

int KademliaPeer::firstDifferentBit(const std::string& lhs, const std::string& rhs) {
    const size_t limit = std::min(lhs.size(), rhs.size());
    for (size_t idx = 0; idx < limit; ++idx) {
        if (lhs[idx] != rhs[idx]) {
            return static_cast<int>(idx);
        }
    }
    return -1;
}

void KademliaPeer::rebuildFingerTable(const std::set<interfaceId>& neighborSet) {
    _fingers.clear();
    if (_binaryIdSize <= 0) {
        _lastNeighborFingerprint = neighborFingerprint(neighborSet);
        return;
    }

    std::vector<std::vector<KademliaFinger>> grouped(static_cast<size_t>(_binaryIdSize));

    for (interfaceId neighbor : neighborSet) {
        if (neighbor == publicId()) continue;
        KademliaFinger entry;
        entry.Id = neighbor;
        entry.binId = getBinaryId(neighbor);
        entry.group = firstDifferentBit(entry.binId, _binaryId);
        if (entry.group >= 0) {
            grouped[static_cast<size_t>(entry.group)].push_back(std::move(entry));
        }
    }

    for (auto& bucket : grouped) {
        if (bucket.empty()) continue;
        size_t chosen = 0;
        if (bucket.size() > 1) {
            chosen = static_cast<size_t>(randMod(static_cast<int>(bucket.size())));
        }
        _fingers.push_back(bucket[chosen]);
    }

    _lastNeighborFingerprint = neighborFingerprint(neighborSet);
}

size_t KademliaPeer::neighborFingerprint(const std::set<interfaceId>& neighborSet) const {
    size_t hash = neighborSet.size();
    const size_t magic = static_cast<size_t>(0x9e3779b97f4a7c15ULL);
    for (interfaceId id : neighborSet) {
        hash ^= static_cast<size_t>(id) + magic + (hash << 6) + (hash >> 2);
    }
    return hash;
}

void KademliaPeer::endOfRound(std::vector<Peer*>& peers) {
    if (peers.empty()) return;

    std::vector<KademliaPeer*> typed;
    typed.reserve(peers.size());
    for (auto* base : peers) {
        typed.push_back(static_cast<KademliaPeer*>(base));
    }

    if (!typed.empty()) {
        int index = randMod(static_cast<int>(typed.size()));
        typed[static_cast<size_t>(index)]->submitLookup(s_currentTransactionId++);
    }

    long long satisfied = 0;
    long long hops = 0;
    long long latency = 0;

    for (auto* peer : typed) {
        satisfied += peer->_requestsSatisfied;
        hops += peer->_totalHops;
        latency += peer->_latency;
    }

    if (satisfied > 0) {
        const double avgHops = static_cast<double>(hops) / satisfied;
        LogWriter::pushValue("kademliaAverageHops", avgHops);
        LogWriter::pushValue("kademliaAverageLatency", static_cast<double>(latency) / satisfied);
    }
    LogWriter::pushValue("kademliaRequestsSatisfied", static_cast<double>(satisfied));
}

}  // namespace quantas
