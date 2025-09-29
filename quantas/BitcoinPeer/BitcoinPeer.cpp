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
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "BitcoinPeer.hpp"
#include "PoWBitcoin.hpp"
#include "../Common/Committee.hpp"
#include "../Common/LogWriter.hpp"
#include "../Common/ParasiteFault.hpp"
#include "../Common/RandomUtil.hpp"
#include "../Common/RoundManager.hpp"
#include "../Common/Abstract/NetworkInterfaceAbstract.hpp"

namespace quantas {

static bool registerBitcoinPeer = []() {
    return PeerRegistry::registerPeerType(
        "BitcoinPeer",
        [](interfaceId pubId) { return new BitcoinPeer(new NetworkInterfaceAbstract(pubId)); });
}();

BitcoinPeer::BitcoinPeer(NetworkInterface* interfacePtr)
    : PoWPeer(interfacePtr) {}

BitcoinPeer::BitcoinPeer(const BitcoinPeer& rhs)
    : PoWPeer(rhs) {}

// Delegates to the reusable mining step so faults can trigger the same logic.
void BitcoinPeer::performComputation() {
    runProtocolStep();
}

// Core mining routine shared by honest and faulty peers.  Parasite faults can call
// this directly after forcing custom parents.
void BitcoinPeer::runProtocolStep(const std::vector<std::string>& overrideParents) {
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

    ++minedBlocks;

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

std::vector<std::string> BitcoinPeer::getParents(const PoW& group) const {
    std::vector<std::string> parents = group.parentsForNextBlock();
    if (parents.empty()) {
        parents.push_back("GENESIS");
    }
    return parents;
}

void BitcoinPeer::initParameters(const std::vector<Peer*>& _peers, json parameters) {
    const std::vector<BitcoinPeer*>& peers = reinterpret_cast<const std::vector<BitcoinPeer*>&>(_peers);

    if (!parameters.is_object() || parameters.is_null()) return;

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
        // Each peer only needs the aggregate rate of other miners; capping the numerator keeps
        // the probability within [0,1].
        int cappedRate = std::min(localRate, denominator);
        peers[idx]->_mineRate = cappedRate;
        peers[idx]->_mineDenominator = denominator;
    }

    // Build a single committee shared by every peer; the simulator only needs one group.
    Committee committee(0);
    for (auto* peerPtr : peers) {
        committee.addMember(peerPtr->publicId());
    }

    for (auto* peerPtr : peers) {
        if (!peerPtr->pow()) {
            peerPtr->setPoW(new PoWBitcoin(new Committee(committee)));
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
            BitcoinPeer* attacker = peers[idx];
            std::set<interfaceId> collaborators = parasiteIds;
            collaborators.erase(attacker->publicId());
            attacker->faultManager.addFault(new ParasiteFault(leadThreshold, collaborators));
        }
    }
}


void BitcoinPeer::endOfRound(std::vector<Peer*>& _peers) {
    const std::vector<BitcoinPeer*>& peers = reinterpret_cast<const std::vector<BitcoinPeer*>&>(_peers);
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
    aggregatedChildren["GENESIS"]; // ensure genesis exists in the adjacency map

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

    int longestChain = 0;
    for (const auto& [hash, height] : computedHeight) {
        (void)hash;
        longestChain = std::max(longestChain, height);
    }

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
    if (commonRoot != "GENESIS") {
        std::string current = commonRoot;
        while (current != "GENESIS") {
            auto itBlock = aggregatedBlocks.find(current);
            if (itBlock == aggregatedBlocks.end()) break;
            if (itBlock->second.parasite) {
                ++parasitesOnCommonPath;
            }
            const auto& parents = itBlock->second.parents;
            int currentHeight = -1;
            if (auto itHeight = computedHeight.find(current); itHeight != computedHeight.end()) {
                currentHeight = itHeight->second;
            }
            std::string nextParent;
            for (const auto& parent : parents) {
                auto parentHeightIt = computedHeight.find(parent);
                if (parentHeightIt != computedHeight.end() &&
                    (currentHeight < 0 || parentHeightIt->second == currentHeight - 1)) {
                    nextParent = parent;
                    break;
                }
            }
            if (nextParent.empty()) break;
            current = nextParent;
        }
    }

    size_t totalBlocks = aggregatedBlocks.size();
    if (aggregatedBlocks.count("GENESIS") && totalBlocks > 0) {
        --totalBlocks;
    }

    // Fork analysis summarises how many branching points exist in the aggregated DAG
    // and how long the competing branches grow from each fork.  We work with the
    // `aggregatedChildren` adjacency (mapping parent hash -> set of child hashes) and
    // the `computedHeight` map that holds each node's distance from genesis.
    size_t forkPoints = 0; // distinct vertices that have more than one child in the union view
    std::unordered_map<int, size_t> forkLengthCounts; // length of each losing branch -> number of occurrences across all forks
    json forkLocations = json::array(); // detailed records for each losing branch {height, length}
    std::unordered_map<std::string, int> depthMemo; // cached longest distance (in edges) from node to any descendant
    std::unordered_set<std::string> depthActive; // detects pathological cycles so we can short-circuit safely

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
                if (!computedHeight.count(child)) continue; // skip children not connected to genesis
                best = std::max(best, 1 + longestDepthFrom(child));
            }
        }
        depthActive.erase(hash);
        depthMemo[hash] = best;
        return best;
    };

    for (const auto& [hash, children] : aggregatedChildren) {
        if (children.size() <= 1) continue; // only care about genuine forks with 2+ outgoing edges
        auto baseHeightIt = computedHeight.find(hash);
        if (baseHeightIt == computedHeight.end()) continue; // skip nodes that are unreachable in the aggregated height map
        std::vector<std::pair<std::string, int>> branchLengths;
        branchLengths.reserve(children.size());
        for (const auto& child : children) {
            if (!computedHeight.count(child)) continue;
            const int branchLen = 1 + longestDepthFrom(child); // number of edges from fork node through this child to deepest descendant
            branchLengths.emplace_back(child, branchLen);
        }
        if (branchLengths.size() <= 1) {
            continue; // not enough valid children to treat as a fork after filtering
        }
        ++forkPoints;
        int bestLength = 0;
        for (const auto& [_, len] : branchLengths) {
            bestLength = std::max(bestLength, len);
        }
        for (const auto& entry : branchLengths) {
            const int len = entry.second;
            if (len >= bestLength) continue; // skip branches that match the winning length
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

void BitcoinPeer::checkInStrm() {
    PoW* group = pow();
    if (!group) return;

    while (!inStreamEmpty()) {
        Packet packet = popInStream();
        json msg = packet.getMessage();
        if (!msg.contains("type") || msg["type"] != "PoW") continue;

        const std::string messageType = msg.value("messageType", std::string());
        if (messageType == "transaction") {
            // Cache the transaction locally so we can mine it later.
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
            // Import the announced block so our local view stays in sync with the network.
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
                hash = std::to_string(miner) + ":" + parents.front();
            }

            // Log the block metadata exactly as advertised; parasite flags are passed through for visibility only.
            group->registerBlock(hash,
                                 parents,
                                 miner,
                                 static_cast<int>(RoundManager::currentRound()),
                                 minedRound,
                                 blkJson.value("parasite", false));
            // we shouldn't do this like this.
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

bool BitcoinPeer::guardSubmit() const {
    // Simple Bernoulli trial used by the simulator to throttle transaction volume.
    return submitRate > 0 && randMod(submitRate) == 0;
}


bool BitcoinPeer::guardMine() const {
    if (_mineRate <= 0) return false;
    if (_mineDenominator <= 0) return false;
    if (_queue.empty()) return false;
    // Mining success probability is _mineRate / _mineDenominator as described in the spec.
    return randMod(_mineDenominator) < _mineRate;
}

BitcoinPeer::PendingTx BitcoinPeer::makeTransaction() {
    // Each peer tracks how many transactions it has submitted so IDs remain unique without locks.
    PendingTx pending;
    pending.roundSubmitted = static_cast<int>(RoundManager::currentRound());
    pending.submitter = publicId();
    pending.id = ++_localSubmitted;
    return pending;
}

json BitcoinPeer::buildTransactionMessage(const PendingTx& pending) const {
    // Transactions are tiny JSON envelopes so other peers can add them to their own queues.
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

json BitcoinPeer::buildBlockMessage(const PoW::BlockRecord& record,
                                       const std::vector<std::string>& parents,
                                       int minedRound,
                                       const PendingTx& pending) const {
    // Include the entire parent list so downstream peers can reconstruct arbitrary DAG edges.
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
            {"parasite", record.parasite}, // surface the parasite flag for observers and faults
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
