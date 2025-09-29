/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CONSENSUS_PEER_hpp
#define CONSENSUS_PEER_hpp

#include <deque>
#include <vector>
#include "Peer.hpp"
#include "ByzantinePeer.hpp"
#include "Consensus.hpp"

namespace quantas{

using std::vector;

class ConsensusPeer : public ByzantinePeer {
public:
    // methods that must be defined when deriving from Peer
    ConsensusPeer                             (NetworkInterface* networkInterface) : ByzantinePeer(networkInterface) {};
    ConsensusPeer                             (const ConsensusPeer &rhs) {};
    virtual ~ConsensusPeer                            () {
        for (auto consensus : consensuses) {
            delete consensus.second;
        }
    };
    
    // map of currently involved consensus instances
    std::map<int, Consensus*> consensuses;

};
}
#endif /* CONSENSUS_PEER_hpp */
