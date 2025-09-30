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

#ifndef POW_HPP
#define POW_HPP

#include <algorithm>
#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Committee.hpp"
#include "Packet.hpp"

namespace quantas {

// Lightweight ledger used by PoW peers to remember block ancestry and miner metadata.
// The class intentionally avoids making chain-quality decisions; callers decide which
// branch to extend while we simply record their choices.
class PoW {
public:
    // Minimal snapshot of a block that higher layers can inspect.
    struct BlockRecord {
        std::string hash;
        std::vector<std::string> parents;
        interfaceId miner = NO_PEER_ID;
        int height = 0;
        bool parasite = false; // informational flag carried by miners; not interpreted here
    };

    PoW(Committee* committee)
        : _committee(committee) {
        BlockRecord genesis;
        genesis.hash = "GENESIS";
        genesis.parents = {};
        genesis.miner = NO_PEER_ID;
        genesis.height = 0;
        genesis.parasite = false;
        _blocks.emplace(genesis.hash, genesis);
        _children[genesis.hash] = {};
        _bestHash = genesis.hash;
    }

    virtual ~PoW() { delete _committee; }

    const std::set<interfaceId>& members() const { return _committee->getMembers(); }
    int id() const { return _committee->getId(); }

    // Record a block and return the stored metadata.  The caller specifies which parents
    // were used as well as whether the block should be tagged as "parasite" for logging
    // purposes (e.g., miners cooperating in an attack).
    BlockRecord registerBlock(const std::string& hash,
                              const std::vector<std::string>& parents,
                              interfaceId miner,
                              int /*seenRound*/,
                              int /*minedRound*/,
                              bool parasiteFlag = false) {
        auto existing = _blocks.find(hash);
        if (existing != _blocks.end()) {
            existing->second.parasite = existing->second.parasite || parasiteFlag;
            return existing->second;
        }

        BlockRecord record;
        record.hash = hash;
        record.parents = parents;
        record.miner = miner;
        record.height = 0; // recalculated after insertion once parents are available
        record.parasite = parasiteFlag;
        _blocks[hash] = record;

        _children[hash];
        for (const auto& parent : parents) {
            auto& vec = _children[parent];
            if (std::find(vec.begin(), vec.end(), hash) == vec.end()) {
                vec.push_back(hash);
            }
        }

        propagateUpdates(hash);
        return _blocks.at(hash);
    }

    // Convenience helper for consumers that follow the tallest chain.
    const BlockRecord* bestTip() const {
        auto it = _blocks.find(_bestHash);
        return (it == _blocks.end()) ? nullptr : &it->second;
    }

    // Look up any previously recorded block by hash.
    const BlockRecord* findBlock(const std::string& hash) const {
        auto it = _blocks.find(hash);
        return (it == _blocks.end()) ? nullptr : &it->second;
    }

    int bestHeight() const {
        auto it = _blocks.find(_bestHash);
        return it == _blocks.end() ? 0 : it->second.height;
    }

    const std::string& bestHash() const { return _bestHash; }

    const std::unordered_map<std::string, std::vector<std::string>>& children() const { return _children; }

    std::vector<std::string> parentsForNextBlock() const {
        return selectParentsForNextBlock(bestTip());
    }

    std::vector<BlockRecord> allBlocks() const {
        std::vector<BlockRecord> records;
        records.reserve(_blocks.size());

        for (const auto& entry : _blocks) {
            records.push_back(entry.second);
        }
        return records;
    }

    std::vector<std::string> tips() const {
        std::vector<std::string> result;
        for (const auto& [hash, _] : _blocks) {
            auto it = _children.find(hash);
            if (it == _children.end() || it->second.empty()) {
                result.push_back(hash);
            }
        }
        return result;
    }

    std::vector<std::string> chainToGenesis(const std::string& tip) const {
        std::vector<std::string> path;
        std::string currentHash = tip;
        const BlockRecord* current = findBlock(currentHash);
        while (current) {
            path.push_back(currentHash);
            if (current->parents.empty()) break;
            currentHash = current->parents.front();
            current = findBlock(currentHash);
        }
        if (path.empty() || path.back() != "GENESIS") {
            path.push_back("GENESIS");
        }
        std::reverse(path.begin(), path.end());
        return path;
    }

protected:
    virtual bool preferCandidate(const BlockRecord& candidate,
                                 const BlockRecord* incumbent) const {
        if (!incumbent) return true;
        return candidate.height > incumbent->height;
    }

    virtual std::vector<std::string> selectParentsForNextBlock(const BlockRecord* best) const {
        if (best) {
            return {best->hash};
        }
        return {"GENESIS"};
    }

    int subtreeSize(const std::string& hash) const {
        std::unordered_set<std::string> visited;
        std::vector<std::string> stack;
        stack.push_back(hash);
        int count = 0;
        while (!stack.empty()) {
            std::string current = stack.back();
            stack.pop_back();
            if (!visited.insert(current).second) continue;
            ++count;
            auto it = _children.find(current);
            if (it != _children.end()) {
                for (const auto& child : it->second) {
                    stack.push_back(child);
                }
            }
        }
        return count;
    }

    bool isAncestor(const std::string& ancestor, const std::string& descendant) const {
        if (ancestor == descendant) return false;
        std::vector<std::string> stack;
        std::unordered_set<std::string> visited;
        stack.push_back(descendant);
        while (!stack.empty()) {
            std::string current = stack.back();
            stack.pop_back();
            if (!visited.insert(current).second) continue;
            auto it = _blocks.find(current);
            if (it == _blocks.end()) continue;
            for (const auto& parent : it->second.parents) {
                if (parent == ancestor) {
                    return true;
                }
                stack.push_back(parent);
            }
        }
        return false;
    }

private:
    const BlockRecord* currentBest() const {
        auto it = _blocks.find(_bestHash);
        return (it == _blocks.end()) ? nullptr : &it->second;
    }

    bool hasFullAncestry(const std::string& hash) const {
        std::unordered_set<std::string> visited;
        std::vector<std::string> stack{hash};
        while (!stack.empty()) {
            std::string current = stack.back();
            stack.pop_back();
            auto it = _blocks.find(current);
            if (it == _blocks.end()) {
                return false;
            }
            for (const auto& parent : it->second.parents) {
                if (visited.insert(parent).second) {
                    stack.push_back(parent);
                }
            }
        }
        return true;
    }

    int recomputeHeight(const std::string& hash) {
        auto it = _blocks.find(hash);
        if (it == _blocks.end()) return 0;
        if (it->second.parents.empty()) {
            it->second.height = 0;
            return 0;
        }
        int maxParentHeight = -1;
        for (const auto& parent : it->second.parents) {
            auto parentIt = _blocks.find(parent);
            if (parentIt == _blocks.end()) {
                maxParentHeight = -1;
                break;
            }
            maxParentHeight = std::max(maxParentHeight, parentIt->second.height);
        }
        int newHeight = (maxParentHeight >= 0) ? maxParentHeight + 1 : 0;
        it->second.height = newHeight;
        return newHeight;
    }

    void maybePromoteBest(const std::string& hash) {
        auto it = _blocks.find(hash);
        if (it == _blocks.end()) return;
        if (!hasFullAncestry(hash)) return;
        if (hash == _bestHash) return;
        const BlockRecord* incumbent = currentBest();
        if (!incumbent || preferCandidate(it->second, incumbent)) {
            _bestHash = hash;
        }
    }

    void ensureBestValid() {
        auto it = _blocks.find(_bestHash);
        if (it != _blocks.end() && hasFullAncestry(_bestHash)) {
            return;
        }
        const BlockRecord* best = nullptr;
        std::string bestHash = "GENESIS";
        for (const auto& entry : _blocks) {
            const std::string& hash = entry.first;
            if (!hasFullAncestry(hash)) continue;
            const BlockRecord& record = entry.second;
            if (!best || preferCandidate(record, best)) {
                best = &record;
                bestHash = hash;
            }
        }
        if (best) {
            _bestHash = bestHash;
        } else {
            _bestHash = "GENESIS";
        }
    }

    void propagateUpdates(const std::string& root) {
        std::deque<std::string> queue;
        queue.push_back(root);
        std::unordered_set<std::string> visited;
        while (!queue.empty()) {
            std::string current = queue.front();
            queue.pop_front();
            if (!visited.insert(current).second) continue;
            recomputeHeight(current);
            maybePromoteBest(current);
            auto it = _children.find(current);
            if (it != _children.end()) {
                for (const auto& child : it->second) {
                    queue.push_back(child);
                }
            }
        }
        ensureBestValid();
    }
    // gets the height of a block from its blockRecord data
    int heightOf(const std::string& hash) const {
        auto it = _blocks.find(hash);
        return (it == _blocks.end()) ? 0 : it->second.height;
    }

    Committee* _committee; // peers in this PoW instance
    std::unordered_map<std::string, BlockRecord> _blocks; // record of all blocks this peer has heard of
    std::string _bestHash; // current best tip to mine on
    std::unordered_map<std::string, std::vector<std::string>> _children; // list of every child for every block we know
};

}

#endif // POW_HPP
