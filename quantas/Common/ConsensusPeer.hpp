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
    ~ConsensusPeer                            () {};

    // perform one step of the Algorithm with the messages in inStream
    void                 performComputation() override {
        std::cout << "Peer: " << publicId() << std::endl;
        while (!inStreamEmpty()) {
            Packet packet = popInStream();
            json msg = packet.getMessage();
            
            std::cout << msg << std::endl;
            if (!msg.contains("type")) {
                std::cout << "Message requires a type" << std::endl;
                continue;
            }
            if (!msg.contains("consensusId")) {
                std::cout << "Message requires a consensusId" << std::endl;
                continue;
            }
            if (msg["type"] == "Request") {
                int targetId = msg["consensusId"];
                auto it = consensuses.find(targetId);
                if (it != consensuses.end()) {
                    Consensus* target = it->second;
                    target->_unhandledRequests.push_back(msg);
                }
            } else if (msg["type"] == "Consensus") {
                int targetId = msg["consensusId"];
                auto it = consensuses.find(targetId);
                if (it != consensuses.end()) {
                    Consensus* target = it->second;
                    if (!msg.contains("seqNum")) {
                        std::cout << "Message requires a  a seqNum" << std::endl;
                        continue;
                    }
                    int seq = msg["seqNum"];
                    if (!msg.contains("MessageType")) {
                        std::cout << "Message requires a  a MessageType" << std::endl;
                        continue;
                    }
                    string type = msg["MessageType"];
                    target->_receivedMessages[seq].insert(std::make_pair(type, msg));
                } else {
                    std::cout << "message lost" << std::endl;
                }
            } else {
                std::cout << "Other?" << std::endl;
            }
            std::cout << std::endl;
        }

        for (auto consensus : consensuses) {
            std::cout << "Run consensus phase" << std::endl;
            consensus.second->runPhase(this);
        }
    };
    // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
    // void                 endOfRound(vector<Peer*>& _peers) override;

    // map of currently involved consensus instances
    std::map<int, Consensus*> consensuses;

};
}
#endif /* CONSENSUS_PEER_hpp */
