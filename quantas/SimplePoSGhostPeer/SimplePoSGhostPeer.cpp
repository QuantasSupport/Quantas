/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QUANTAS is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <cmath>
#include <map>
#include <mutex>
#include <set>
#include "SimplePoSGhostPeer.hpp"
#include "../Common/OutputWriter.hpp"
#include "../Common/RandomUtil.hpp"
#include "../Common/RoundManager.hpp"

namespace quantas {

static bool registerSimplePoSGhostPeer = []() {
    return PeerRegistry::registerPeerType(
        "SimplePoSGhostPeer",
        [](interfaceId pubId) { return new SimplePoSGhostPeer(new NetworkInterfaceAbstract(pubId)); });
}();

// Settings shared by all validators, plus a record of every block used only for the metrics.
struct SimplePoSShared {
    int n = 0;
    int slotLength = 4;
    int voteOffset = 2;
    std::vector<long long> stake;  // per validator, in integer units
    long long totalStake = 0;
    uint64_t seed = 0;

    struct BlockRecord {
        std::string parent;
        int slot = 0;
        interfaceId proposer = 0;
        size_t proposedRound = 0;
        int acceptedBy = 0;          // validators that have this block in their view
        size_t lastAcceptRound = 0;  // round the last of them got it
    };
    std::mutex mutex;
    std::map<std::string, BlockRecord> blocks;
};

namespace {

const char* const kGenesis = "GENESIS";

uint64_t splitmix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

// Every validator gets the same answer: a stake-weighted draw from the experiment seed and slot.
interfaceId proposerFor(const SimplePoSShared& s, int slot) {
    long long pick = static_cast<long long>(
        splitmix64(s.seed ^ splitmix64(static_cast<uint64_t>(slot))) % static_cast<uint64_t>(s.totalStake));
    for (int id = 0; id < s.n; ++id) {
        pick -= s.stake[id];
        if (pick < 0) return id;
    }
    return s.n - 1;
}

} // namespace

SimplePoSGhostPeer::SimplePoSGhostPeer(NetworkInterface* networkInterface)
    : Peer(networkInterface) {}

// Parameters: slotLength (rounds per slot, default 4), voteOffset (round in the slot when
// validators vote, default the middle) and zipfExponent (stake of validator i is
// proportional to 1/(i+1)^exponent; 0, the default, gives every validator equal stake).
void SimplePoSGhostPeer::initParameters(const std::vector<Peer*>& peers, json parameters) {
    auto shared = std::make_shared<SimplePoSShared>();
    const int n = static_cast<int>(peers.size());
    shared->n = n;
    shared->slotLength = std::max(1, parameters.value("slotLength", 4));
    shared->voteOffset = std::clamp(parameters.value("voteOffset", shared->slotLength / 2),
                                    0, shared->slotLength - 1);
    shared->seed = random_detail::baseSeed().load();

    // Total stake is n validators' worth, one validator's worth being 1,000,000 units.
    const double exponent = parameters.value("zipfExponent", 0.0);
    double sum = 0.0;
    for (int i = 0; i < n; ++i) sum += 1.0 / std::pow(i + 1.0, exponent);
    for (int i = 0; i < n; ++i) {
        shared->stake.push_back(std::llround(1.0 / std::pow(i + 1.0, exponent) / sum * n * 1000000));
        shared->totalStake += shared->stake.back();
    }
    shared->blocks[kGenesis] = {};

    for (Peer* peer : peers) {
        auto* validator = static_cast<SimplePoSGhostPeer*>(peer);
        validator->_shared = shared;
        validator->_blocks.assign(1, Block{});
        validator->_blocks[0].hash = kGenesis;
        validator->_index = {{kGenesis, 0}};
        validator->_latest.assign(n, Vote{});
        validator->_waitingBlocks.clear();
        validator->_waitingVotes.clear();
    }
}

void SimplePoSGhostPeer::performComputation() {
    while (!inStreamEmpty()) {
        const json msg = popInStream().getMessage();
        const std::string type = msg.value("type", "");
        if (type == "block") {
            receiveBlock(msg);
        } else if (type == "vote") {
            receiveVote(msg["validator"].get<interfaceId>(), msg["slot"].get<int>(), msg["block"].get<std::string>());
        }
    }

    const size_t round = RoundManager::currentRound();
    const int slot = static_cast<int>((round - 1) / _shared->slotLength) + 1;
    const int offset = static_cast<int>((round - 1) % _shared->slotLength);
    if (offset == 0 && proposerFor(*_shared, slot) == publicId()) propose(slot);
    if (offset == _shared->voteOffset) vote(slot);
}

// A block joins the view once its parent is in it; until then it waits.
void SimplePoSGhostPeer::receiveBlock(const json& block) {
    std::vector<json> ready{block};
    while (!ready.empty()) {
        const json current = ready.back();
        ready.pop_back();
        const std::string hash = current["hash"];
        const std::string parentHash = current["parent"];
        if (_index.count(hash)) continue;
        auto parent = _index.find(parentHash);
        if (parent == _index.end()) {
            _waitingBlocks[parentHash].push_back(current);
            continue;
        }

        Block added;
        added.hash = hash;
        added.parent = parent->second;
        added.height = _blocks[parent->second].height + 1;
        const int index = static_cast<int>(_blocks.size());
        _blocks.push_back(added);
        _blocks[parent->second].children.push_back(index);
        _index[hash] = index;
        {
            std::lock_guard<std::mutex> lock(_shared->mutex);
            SimplePoSShared::BlockRecord& record = _shared->blocks[hash];
            ++record.acceptedBy;
            record.lastAcceptRound = RoundManager::currentRound();
        }

        auto children = _waitingBlocks.find(hash);
        if (children != _waitingBlocks.end()) {
            for (const json& child : children->second) ready.push_back(child);
            _waitingBlocks.erase(children);
        }
        auto votes = _waitingVotes.find(hash);
        if (votes != _waitingVotes.end()) {
            for (const auto& [validator, slot] : votes->second) countVote(validator, slot, index);
            _waitingVotes.erase(votes);
        }
    }
}

// A vote counts once its block is in the view; until then it waits.
void SimplePoSGhostPeer::receiveVote(interfaceId validator, int slot, const std::string& target) {
    auto block = _index.find(target);
    if (block == _index.end()) {
        _waitingVotes[target].push_back({validator, slot});
    } else {
        countVote(validator, slot, block->second);
    }
}

// LMD: only a validator's newest vote counts. Moving it takes its stake off the old
// block and its ancestors and adds it to the new block and its ancestors, stopping
// where the two paths meet (blocks above that point keep the stake either way).
void SimplePoSGhostPeer::countVote(interfaceId validator, int slot, int block) {
    Vote& latest = _latest[validator];
    if (slot <= latest.slot) return;
    const long long stake = _shared->stake[validator];
    int from = latest.block;
    int to = block;
    if (from < 0) {
        for (int b = to; b >= 0; b = _blocks[b].parent) _blocks[b].weight += stake;
    } else {
        while (from != to) {
            if (_blocks[from].height >= _blocks[to].height) {
                _blocks[from].weight -= stake;
                from = _blocks[from].parent;
            } else {
                _blocks[to].weight += stake;
                to = _blocks[to].parent;
            }
        }
    }
    latest = {slot, block};
}

// LMD GHOST: from genesis, keep stepping into the heaviest child (ties: smaller hash).
int SimplePoSGhostPeer::head() const {
    int current = 0;
    while (!_blocks[current].children.empty()) {
        int best = _blocks[current].children.front();
        for (int child : _blocks[current].children) {
            if (_blocks[child].weight > _blocks[best].weight ||
                (_blocks[child].weight == _blocks[best].weight && _blocks[child].hash < _blocks[best].hash)) {
                best = child;
            }
        }
        current = best;
    }
    return current;
}

void SimplePoSGhostPeer::propose(int slot) {
    const std::string hash = "S" + std::to_string(slot) + "P" + std::to_string(publicId());
    const json block = {{"type", "block"}, {"hash", hash}, {"parent", _blocks[head()].hash}, {"slot", slot}};
    {
        std::lock_guard<std::mutex> lock(_shared->mutex);
        SimplePoSShared::BlockRecord& record = _shared->blocks[hash];
        record.parent = block["parent"];
        record.slot = slot;
        record.proposer = publicId();
        record.proposedRound = RoundManager::currentRound();
    }
    receiveBlock(block);
    broadcast(block);
}

void SimplePoSGhostPeer::vote(int slot) {
    const int target = head();
    countVote(publicId(), slot, target);
    broadcast({{"type", "vote"}, {"validator", publicId()}, {"slot", slot}, {"block", _blocks[target].hash}});
}

// Outputs per test:
//   orphanRate       share of proposed blocks that are not on the canonical chain
//   forks            blocks that ended up with two or more children
//   avgBlockLatency  rounds from proposing a block until every validator has it
//   maxBlockLatency  the slowest such block
//   blockTree        every proposed block, for plotBlockTree.py
void SimplePoSGhostPeer::endOfExperiment(std::vector<Peer*>& peers) {
    // The canonical chain ends at the head most validators hold.
    std::map<std::string, int> heads;
    for (Peer* peer : peers) {
        const auto* validator = static_cast<SimplePoSGhostPeer*>(peer);
        ++heads[validator->_blocks[validator->head()].hash];
    }
    std::string canonicalHead = kGenesis;
    int mostVotes = 0;
    for (const auto& [hash, count] : heads) {
        if (count > mostVotes) {
            canonicalHead = hash;
            mostVotes = count;
        }
    }

    std::lock_guard<std::mutex> lock(_shared->mutex);
    std::set<std::string> canonicalChain;
    for (std::string hash = canonicalHead; hash != kGenesis; hash = _shared->blocks[hash].parent) canonicalChain.insert(hash);
    const long canonical = static_cast<long>(canonicalChain.size());

    long proposed = 0;
    std::map<std::string, int> children;
    long latencySum = 0;
    long fullyPropagated = 0;
    size_t maxLatency = 0;
    json blockTree = json::array();
    for (const auto& [hash, record] : _shared->blocks) {
        if (hash == kGenesis) continue;
        ++proposed;
        ++children[record.parent];
        long latency = -1;  // -1: some validator never got the block
        if (record.acceptedBy == _shared->n) {
            latency = static_cast<long>(record.lastAcceptRound - record.proposedRound);
            latencySum += latency;
            maxLatency = std::max(maxLatency, static_cast<size_t>(latency));
            ++fullyPropagated;
        }
        blockTree.push_back({{"hash", hash}, {"parent", record.parent}, {"slot", record.slot},
                             {"proposer", record.proposer}, {"proposedRound", record.proposedRound},
                             {"latency", latency}, {"canonical", canonicalChain.count(hash) > 0}});
    }
    long forks = 0;
    for (const auto& entry : children) {
        if (entry.second > 1) ++forks;
    }

    OutputWriter::setTestValue("slots", static_cast<long>(RoundManager::currentRound() / _shared->slotLength));
    OutputWriter::setTestValue("blocksProposed", proposed);
    OutputWriter::setTestValue("canonicalBlocks", canonical);
    OutputWriter::setTestValue("orphanRate", proposed > 0 ? 1.0 - static_cast<double>(canonical) / proposed : 0.0);
    OutputWriter::setTestValue("forks", forks);
    OutputWriter::setTestValue("avgBlockLatency", fullyPropagated > 0 ? static_cast<double>(latencySum) / fullyPropagated : 0.0);
    OutputWriter::setTestValue("maxBlockLatency", static_cast<long>(maxLatency));
    OutputWriter::setTestValue("blockTree", blockTree);
}

} // namespace quantas
