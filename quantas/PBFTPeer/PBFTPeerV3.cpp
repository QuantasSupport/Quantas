/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "PBFTPeerV3.hpp"

namespace quantas {

static bool registerPBFT = [](){
	PeerRegistry::registerPeerType("PBFTPeerV3", 
		[](interfaceId pubId){ return new PBFTPeerV3(new NetworkInterfaceAbstract(pubId)); });
	return true;
}();


class EquivocateFault : public Fault {
public:
    virtual ~EquivocateFault() = default;

    bool overridesSendType(const std::string& sendType) const override { 
		if (sendType == "multicast")
			return true;
		else 
			return false;
	}

    virtual bool onSend(Peer* peer, json& msg, const std::string& sendType, const std::set<interfaceId>& targets = std::set<interfaceId>()) {
        if (sendType == "multicast") {
			if (msg.contains("MessageType") && msg["MessageType"] == "pre-prepare" && msg.contains("proposal") && msg["proposal"].contains("value")) {
				auto cast_peer = dynamic_cast<PBFTPeerV3*>(peer);
                // if (peer->leader == peer->getId()) {
                auto neighbors = targets;
                std::vector<interfaceId> temp(neighbors.begin(), neighbors.end());  // Copy set to vector
                std::shuffle(temp.begin(), temp.end(), threadLocalEngine());  // Shuffle vector
                std::set<interfaceId> subset1(temp.begin(), temp.begin() + temp.size() / 2);  // Take the first 'count' elements
                std::set<interfaceId> subset2(temp.begin() + temp.size() / 2, temp.end());
                json msg_copy = msg;
                cast_peer->getNetworkInterface()->multicast(msg, subset2);
                msg_copy["proposal"]["value"] = !msg_copy["proposal"]["value"];
                cast_peer->getNetworkInterface()->multicast(msg_copy, subset1);
                return true; 
                // } else {
                //     json msg_copy = msg;
                //     msg_copy["proposal"]["value"] = !msg_copy["proposal"]["value"];
                //     cast_peer->getNetworkInterface()->multicast(msg_copy, targets);
                // }
			} else 
				return false;
		} else 
			return false;
	}
};

class PBFTConsensus : public Consensus {
public:
    PBFTConsensus(Committee* committee);
    

    std::pair<int, json> getCount(string phase) {
        int seqNum = _confirmedTrans.size();
        if (_receivedMessages.find(seqNum) == _receivedMessages.end()) {
            return {0, json()};
        }
        auto range = _receivedMessages[seqNum].equal_range(phase);
        std::set<interfaceId> uniquePeers;
        json exampleMsg;
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second.contains("proposal") && 
            it->second["proposal"].contains("view") &&
            it->second["proposal"]["view"] == view &&
            it->second.contains("from_id")) {

                if (it->second["proposal"].contains("value")) {

                    if (it->second["proposal"]["value"] == value) {
                        interfaceId newId = it->second["from_id"];
                        uniquePeers.insert(newId);
                        if (exampleMsg.empty()) exampleMsg = it->second;
                    }
                } else {
                    // for view changes
                    interfaceId newId = it->second["from_id"];
                    uniquePeers.insert(newId);
                }
            }
        }
        return {uniquePeers.size(), exampleMsg};
    }

    void submitRequest(Peer* peer) {
        json msg = {
            {"type", "Request"},
            {"requestId", _currentClientRequestId++},
            {"submitterId", peer->publicId()},
            {"consensusId", getId()},
            {"roundSubmitted", RoundManager::currentRound()}
        };
        
        peer->multicast(msg, getMembers());
        _unhandledRequests.insert({RoundManager::currentRound(),msg});
    }

    void requestViewChange(Peer* peer);

	interfaceId leader = NO_PEER_ID;
    int view = 0;
    bool value = true;
    int viewChangeTimer = 10;
    int viewChangeDelay = 10;

};

// Consider adding the check commits, check prepares, etc. to a base phase class
// since it is needed in multiple

// class PBFTPhase : public Phase {
// public:
//     static Phase* instance() {
//         static PBFTPrePreparePhase instance;
//         return &instance;
//     }

//     void runPhase(Consensus* con, Peer* peer) override;
// };

class PBFTPrePreparePhase : public Phase {
public:
    static Phase* instance() {
        static PBFTPrePreparePhase instance;
        return &instance;
    }

    void runPhase(Consensus* con, Peer* peer) override;
};

class PBFTPreparePhase : public Phase {
public:
    static Phase* instance() {
        static PBFTPreparePhase instance;
        return &instance;
    }

    void runPhase(Consensus* con, Peer* peer) override;
};

class PBFTCommitPhase : public Phase {
public:
    static Phase* instance() {
        static PBFTCommitPhase instance;
        return &instance;
    }

    void runPhase(Consensus* con, Peer* peer) override;
};

class PBFTViewChangePhase : public Phase {
    public:
        static Phase* instance() {
            static PBFTViewChangePhase instance;
            return &instance;
        }
    
        void runPhase(Consensus* con, Peer* peer) override;
    };

PBFTConsensus::PBFTConsensus(Committee* committee) {
    _committee = committee;
    _phase = PBFTPrePreparePhase::instance();  // Start here
}

void PBFTConsensus::requestViewChange(Peer* peer) {
    // This guy may fall behind in commits
    // need to fix this 

    // std::cout << "requesting a view change" << std::endl;
    json msg = {
        {"type", "Consensus"},
        {"consensusId", getId()},
        {"MessageType", "viewChange"},
        {"seqNum", _confirmedTrans.size()},
        {"proposal", {
            {"view", ++view}
        }},
        {"from_id", peer->publicId()}
    };
    auto neighbors = getMembers();
    std::vector<interfaceId> temp(neighbors.begin(), neighbors.end());
    leader = temp[view % temp.size()];
    peer->multicast(msg, getMembers());
    _receivedMessages[msg["seqNum"]].insert({ "viewChange", msg });
    _phase->changePhase(this, PBFTViewChangePhase::instance());
}

void PBFTPrePreparePhase::runPhase(Consensus* con, Peer* peer) {
    PBFTConsensus* consensus = dynamic_cast<PBFTConsensus*>(con);
    if (!consensus) return;
    if (consensus->viewChangeTimer < RoundManager::currentRound()) {
        consensus->requestViewChange(peer);
        return;
    }
    
    // Leader creates pre-prepare
    if (peer->publicId() == consensus->leader) {
        if (consensus->_unhandledRequests.empty()) {
            consensus->submitRequest(peer);
        }

        auto tx = consensus->_unhandledRequests.begin()->second;

        json msg = {
            {"type", "Consensus"},
            {"consensusId",consensus->getId()},
            {"MessageType", "pre-prepare"},
            {"seqNum", consensus->_confirmedTrans.size()},
            {"proposal", {
                {"Request", tx},
                {"view", consensus->view},
                {"value", true}
            }},
            {"from_id", peer->publicId()}
        };
        
        peer->multicast(msg, consensus->getMembers());
        consensus->_receivedMessages[msg["seqNum"]].insert({ "pre-prepare", msg });

        changePhase(consensus, PBFTPreparePhase::instance());
    } else {
        int seqNum = consensus->_confirmedTrans.size();
        if (consensus->_receivedMessages.find(seqNum) == consensus->_receivedMessages.end()) {
            return;
        }
        auto range = consensus->_receivedMessages[seqNum].equal_range("pre-prepare");
        json prePrepareMsg;
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second.contains("proposal") && 
            it->second["proposal"].contains("view") &&
            it->second["proposal"]["view"] == consensus->view &&
            it->second.contains("from_id") &&
            it->second["from_id"] == consensus->leader) {
                prePrepareMsg = it->second;
                break;
            }
        }
        if (!prePrepareMsg.empty()) {
            
            json newMsg = prePrepareMsg;
            newMsg["MessageType"] = "prepare";
            newMsg["from_id"] = peer->publicId();
            consensus->value = newMsg["proposal"]["value"];

            peer->multicast(newMsg, consensus->getMembers());
            consensus->_receivedMessages[seqNum].insert({ "prepare", newMsg });

            changePhase(consensus, PBFTPreparePhase::instance());
            consensus->runPhase(peer);
        } else {
            // check prepare phase
            auto result = consensus->getCount("prepare");
            int neighborCount = consensus->getMembers().size();
            if (result.first <= (2 * neighborCount / 3)) {
                consensus->value = !consensus->value;
                result = consensus->getCount("prepare");
            }
            
            if (result.first > (2 * neighborCount / 3)) {
                json newMsg = result.second;
                newMsg["MessageType"] = "prepare";
                newMsg["from_id"] = peer->publicId();

                peer->multicast(newMsg, consensus->getMembers());
                consensus->_receivedMessages[seqNum].insert({ "prepare", newMsg });

                newMsg["MessageType"] = "commit";

                peer->multicast(newMsg, consensus->getMembers());
                consensus->_receivedMessages[seqNum].insert({ "commit", newMsg });
                
                changePhase(consensus, PBFTCommitPhase::instance());
                consensus->runPhase(peer);
            } else {
                // Check commits
                auto result = consensus->getCount("commit");
                if (result.first <= (2 * neighborCount / 3)) {
                    consensus->value = !consensus->value;
                    result = consensus->getCount("commit");
                }
                if (result.first > (2 * neighborCount / 3)) {
                    json confirmed = result.second;
                    confirmed["MessageType"] = "commit";
                    confirmed["from_id"] = peer->publicId();

                    peer->multicast(confirmed, consensus->getMembers());
                    
                    // Add to confirmed
                    consensus->_confirmedTrans.push_back(confirmed["proposal"]["Request"]);
                    consensus->_latency += RoundManager::currentRound() - confirmed["proposal"]["Request"]["roundSubmitted"].get<int>();
                    consensus->viewChangeTimer = RoundManager::currentRound() + consensus->viewChangeDelay;
                    // clear some memory
                    if (confirmed["proposal"]["value"] == false) {
                        consensus->_confirmedZero = true;
                        json notes;
                        notes["round"] = RoundManager::currentRound();
                        notes["committee"] = consensus->getId();
                        notes["peer"] = peer->publicId();
                        LogWriter::pushValue("safetyViolation", notes);
                    }

                    // remove the request as it has now been handled
                    for (auto it = consensus->_unhandledRequests.begin(); it != consensus->_unhandledRequests.end(); ++it) {
                        if (it->second == confirmed["proposal"]["Request"]) {
                            consensus->_unhandledRequests.erase(it);
                            break;
                        }
                    }

                    // Reset to pre prepare
                    changePhase(consensus, PBFTPrePreparePhase::instance());
                    consensus->runPhase(peer);
                }
            }
        }
    }
}

void PBFTPreparePhase::runPhase(Consensus* con, Peer* peer) {
    PBFTConsensus* consensus = dynamic_cast<PBFTConsensus*>(con);
    if (!consensus) return;
    if (consensus->viewChangeTimer < RoundManager::currentRound()) {
        consensus->requestViewChange(peer);
        return;
    }
    
    auto result = consensus->getCount("prepare");
    int neighborCount = consensus->getMembers().size();
    if (result.first > (2 * neighborCount / 3)) {
        json commitMsg = result.second;
        commitMsg["MessageType"] = "commit";
        commitMsg["from_id"] = peer->publicId();

        peer->multicast(commitMsg, consensus->getMembers());
        int seqNum = consensus->_confirmedTrans.size();
        consensus->_receivedMessages[seqNum].insert({ "commit", commitMsg });

        changePhase(consensus, PBFTCommitPhase::instance());
        consensus->runPhase(peer);
    } else {
        // Check commits
        auto result = consensus->getCount("commit");
        if (result.first > (2 * neighborCount / 3)) {
            json confirmed = result.second;
            confirmed["MessageType"] = "commit";
            confirmed["from_id"] = peer->publicId();

            peer->multicast(confirmed, consensus->getMembers());
            
            // Add to confirmed
            consensus->_confirmedTrans.push_back(confirmed["proposal"]["Request"]);
            consensus->_latency += RoundManager::currentRound() - confirmed["proposal"]["Request"]["roundSubmitted"].get<int>();
            consensus->viewChangeTimer = RoundManager::currentRound() + consensus->viewChangeDelay;
            // clear some memory
            if (confirmed["proposal"]["value"] == false) {
                consensus->_confirmedZero = true;
                json notes;
                notes["round"] = RoundManager::currentRound();
                notes["committee"] = consensus->getId();
                notes["peer"] = peer->publicId();
                LogWriter::pushValue("safetyViolation", notes);
            }

            // remove the request as it has now been handled
            for (auto it = consensus->_unhandledRequests.begin(); it != consensus->_unhandledRequests.end(); ++it) {
                if (it->second == confirmed["proposal"]["Request"]) {
                    consensus->_unhandledRequests.erase(it);
                    break;
                }
            }

            // Reset to pre prepare
            changePhase(consensus, PBFTPrePreparePhase::instance());
            consensus->runPhase(peer);
        }
    }
}

void PBFTCommitPhase::runPhase(Consensus* con, Peer* peer) {
    PBFTConsensus* consensus = dynamic_cast<PBFTConsensus*>(con);
    if (!consensus) return;
    if (consensus->viewChangeTimer < RoundManager::currentRound()) {
        consensus->requestViewChange(peer);
        return;
    }
    
    auto result = consensus->getCount("commit");
    int neighborCount = consensus->getMembers().size();
    if (result.first > (2 * neighborCount / 3)) {
        // Add to confirmed
        json confirmed = result.second;
        consensus->_confirmedTrans.push_back(confirmed["proposal"]["Request"]);
        consensus->_latency += RoundManager::currentRound() - confirmed["proposal"]["Request"]["roundSubmitted"].get<int>();
        consensus->viewChangeTimer = RoundManager::currentRound() + consensus->viewChangeDelay;
        // clear some memory
        if (confirmed["proposal"]["value"] == false) {
            consensus->_confirmedZero = true;
            json notes;
            notes["round"] = RoundManager::currentRound();
            notes["committee"] = consensus->getId();
            notes["peer"] = peer->publicId();
            LogWriter::pushValue("safetyViolation", notes);
        }

        // remove the request as it has now been handled
        for (auto it = consensus->_unhandledRequests.begin(); it != consensus->_unhandledRequests.end(); ++it) {
            if (it->second == confirmed["proposal"]["Request"]) {
                consensus->_unhandledRequests.erase(it);
                break;
            }
        }

        // Reset to pre prepare
        changePhase(consensus, PBFTPrePreparePhase::instance());
        consensus->runPhase(peer);
    }
}

void PBFTViewChangePhase::runPhase(Consensus* con, Peer* peer) {
    PBFTConsensus* consensus = dynamic_cast<PBFTConsensus*>(con);
    if (!consensus) return;

    auto result = consensus->getCount("viewChange");
    int neighborCount = consensus->getMembers().size();
    if (result.first > (2 * neighborCount / 3)) {
        consensus->viewChangeTimer = RoundManager::currentRound() + consensus->viewChangeDelay;
        // Reset to pre prepare
        changePhase(consensus, PBFTPrePreparePhase::instance());
        consensus->runPhase(peer);
        
    }
}

PBFTPeerV3::~PBFTPeerV3() {

}

PBFTPeerV3::PBFTPeerV3(const PBFTPeerV3& rhs) : ConsensusPeer(rhs) {
	
}

PBFTPeerV3::PBFTPeerV3(NetworkInterface* networkInterface) : ConsensusPeer(networkInterface) {

}

void PBFTPeerV3::initParameters(const std::vector<Peer*>& _peers, json parameters) {
	const vector<PBFTPeerV3*> peers = reinterpret_cast<vector<PBFTPeerV3*> const&>(_peers);

	int committeeId = 0;
    interfaceId leaderId = peers[0]->publicId();
    peers[0]->faultManager.addFault(new EquivocateFault());
    // Assign a PBFTConsensus instance using this committee to each peer
    for (auto peer : peers) {
		// Create and configure a committee
		Committee* committeePtr = new Committee(committeeId);
		for (auto peer : peers) {
			committeePtr->addMember(peer->publicId());
		}
        PBFTConsensus* pbft = new PBFTConsensus(committeePtr);
		pbft->leader = leaderId;
        peer->consensuses[committeeId] = pbft;
	    
	}
}

void PBFTPeerV3::endOfRound(vector<Peer*>& _peers) {
	const vector<PBFTPeerV3*> peers = reinterpret_cast<vector<PBFTPeerV3*> const&>(_peers);
    double length = 0;
    double latency = 0;
    bool confirmedZero = false;
    for (auto& consensus : peers[0]->consensuses) {
        length += consensus.second->_confirmedTrans.size();
        latency += consensus.second->_latency;
    }
    if (length > 0) {
        LogWriter::pushValue("latency", latency / length);
    } else {
        LogWriter::pushValue("latency", 0);
    }
	LogWriter::pushValue("throughput", length);
}

}
