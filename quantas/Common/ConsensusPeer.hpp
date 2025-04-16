/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ConsensusPeer_hpp
#define ConsensusPeer_hpp

#include <deque>
#include <vector>
#include "Peer.hpp"
#include "Consensus.hpp"

namespace quantas{

    using std::vector;

    class ConsensusPeer : public Peer {
    public:
        // methods that must be defined when deriving from Peer
        ConsensusPeer                             (NetworkInterface*);
        ConsensusPeer                             (const ConsensusPeer &rhs);
        ~ConsensusPeer                            ();

        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation() override {
            while (!inStreamEmpty()) {
                Packet packet = popInStream();
                json msg = packet.getMessage();
                if (msg["type"] == "Request") {
                    int targetId = msg["consensusId"];
                    auto it = consensuses.find(targetId);
                    if (it != consensuses.end()) {
                        Consensus* target = it->second;
                        target->transactions[seq].push_back(msg);
                    }
                } else if (msg["type"] == "Consensus") {
                    int targetId = msg["target"];
                    auto it = consensuses.find(targetId);
                    if (it != consensuses.end()) {
                        Consensus* target = it->second;
                        
                        int seq = msg["SeqNum"];
                        MessageType* type = msg["MessageType"];

                        target->_receivedMessages[seq].insert(std::make_pair(type, msg));
                    }
                }
            }

            for (auto element : consensuses) {
                element.second->run_phase();
            }
        };
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound(vector<Peer*>& _peers) override;

        // map of currently involved consensus instances
        std::map<int, Consensus*> consensuses;

    };
}
#endif /* ConsensusPeer_hpp */



// std::vector<interfaceId> temp(_neighbors.begin(), _neighbors.end());  // Copy set to vector
// std::shuffle(temp.begin(), temp.end(), threadLocalEngine());  // Shuffle vector
// std::set<interfaceId> subset(temp.begin(), temp.begin() + count);  // Take the first 'count' elements

// multicast(msg, subset);
