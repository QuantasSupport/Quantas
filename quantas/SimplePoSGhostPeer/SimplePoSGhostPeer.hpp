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

#ifndef SIMPLEPOSGHOSTPEER_HPP
#define SIMPLEPOSGHOSTPEER_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "../Common/Peer.hpp"

namespace quantas {

struct SimplePoSShared;

// A minimal proof-of-stake chain with the LMD GHOST fork choice. Each slot one
// stake-weighted proposer builds a block on its head, and halfway through the slot
// every validator votes for its head. The head is found by starting at genesis and
// stepping into the child with the most stake behind it (each validator's latest vote).
// No attacks, no Casper, no block sync: a draft of PoSGhostPeer for a complete topology.
class SimplePoSGhostPeer : public Peer {
public:
    explicit SimplePoSGhostPeer(NetworkInterface* networkInterface);
    ~SimplePoSGhostPeer() override = default;

    void initParameters(const std::vector<Peer*>& peers, json parameters) override;
    void performComputation() override;
    void endOfExperiment(std::vector<Peer*>& peers) override;

private:
    struct Block {
        std::string hash;
        int parent = -1;         // index in _blocks, -1 for genesis
        int height = 0;
        long long weight = 0;    // stake of latest votes for this block or a descendant
        std::vector<int> children;
    };
    struct Vote {
        int slot = -1;
        int block = -1;
    };

    void receiveBlock(const json& block);
    void receiveVote(interfaceId validator, int slot, const std::string& target);
    void countVote(interfaceId validator, int slot, int block);
    int head() const;
    void propose(int slot);
    void vote(int slot);

    std::shared_ptr<SimplePoSShared> _shared;
    std::vector<Block> _blocks;                      // this validator's view of the block tree
    std::unordered_map<std::string, int> _index;     // hash -> index in _blocks
    std::vector<Vote> _latest;                       // latest vote of every validator
    std::unordered_map<std::string, std::vector<json>> _waitingBlocks;  // parent hash -> blocks
    std::unordered_map<std::string, std::vector<std::pair<interfaceId, int>>> _waitingVotes;  // block hash -> (validator, slot)
};

} // namespace quantas

#endif // SIMPLEPOSGHOSTPEER_HPP
