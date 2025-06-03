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
		std::cout << "onSend " << msg << std::endl;
        if (sendType == "multicast") {
			if (msg.contains("MessageType") && msg["MessageType"] == "pre-prepare" && msg.contains("proposal") && msg["proposal"].contains("value")) {
				auto cast_peer = reinterpret_cast<PBFTPeerV3*>(peer);
                // if (peer->getLeader() == peer->getId()) {
                auto neighbors = targets;
                std::vector<interfaceId> temp(neighbors.begin(), neighbors.end());  // Copy set to vector
                std::shuffle(temp.begin(), temp.end(), threadLocalEngine());  // Shuffle vector
                std::set<interfaceId> subset1(temp.begin(), temp.begin() + temp.size() / 6);  // Take the first 'count' elements
                std::set<interfaceId> subset2(temp.begin() + temp.size() / 6, temp.end());
                json msg_copy = msg;
                cast_peer->getNetworkInterface()->multicast(msg, subset1);
                msg_copy["proposal"]["value"] = !msg_copy["proposal"]["value"];
                cast_peer->getNetworkInterface()->multicast(msg_copy, subset2);
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

	interfaceId getLeader() override {return _leader;};

	interfaceId _leader = NO_PEER_ID;
};

class PBFTPrePreparePhase : public Phase {
public:
    static Phase* instance() {
        static PBFTPrePreparePhase instance;
        return &instance;
    }

    void runPhase(Consensus* consensus, Peer* peer) override;
};

class PBFTPreparePhase : public Phase {
public:
    static Phase* instance() {
        static PBFTPreparePhase instance;
        return &instance;
    }

    void runPhase(Consensus* consensus, Peer* peer) override;
};

class PBFTCommitPhase : public Phase {
public:
    static Phase* instance() {
        static PBFTCommitPhase instance;
        return &instance;
    }

    void runPhase(Consensus* consensus, Peer* peer) override;
};

PBFTConsensus::PBFTConsensus(Committee* committee) {
    _committee = committee;
    _phase = PBFTPrePreparePhase::instance();  // Start here
}

void PBFTPrePreparePhase::runPhase(Consensus* consensus, Peer* peer) {
    std::cout << "Peer " << peer->publicId() << " in prePrepare" << std::endl;
    // Leader creates pre-prepare
    if (peer->publicId() == consensus->getLeader()) {
        if (consensus->_unhandledRequests.empty()) {
            json msg = {
                {"type", "Request"},
                {"requestId", consensus->_currentClientRequestId++},
                {"submitterId", peer->publicId()},
                {"consensusId", consensus->getId()},
                {"roundSubmitted", RoundManager::currentRound()}
            };
            
            peer->multicast(msg, consensus->getMembers());
            consensus->_unhandledRequests.push_back(msg);
        }

        auto tx = consensus->_unhandledRequests.back();
        consensus->_unhandledRequests.pop_back();

        json msg = {
            {"type", "Consensus"},
            {"consensusId",consensus->getId()},
            {"MessageType", "pre-prepare"},
            {"seqNum", consensus->_confirmedTrans.size()},
            {"proposal", {
                {"Request", tx},
                {"value", true}
            }},
            {"from_id", peer->publicId()},
            {"roundSubmitted", RoundManager::currentRound()}
        };
        std::cout << "peer->multicast(msg, consensus->getMembers());" << std::endl;
        peer->multicast(msg, consensus->getMembers());
        std::cout << "consensus->" << std::endl;
        consensus->_receivedMessages[msg["seqNum"]].insert({ "pre-prepare", msg });

        std::cout << "moving to Prepare" << std::endl;
        changePhase(consensus, PBFTPreparePhase::instance());
        // consensus->runPhase(peer);
    } else {
        int seqNum = consensus->_confirmedTrans.size();
        auto& mm = consensus->_receivedMessages[seqNum];
        int count = static_cast<int>(mm.count("pre-prepare"));
        if (count > 0) {
            json prepareMsg = mm.find("pre-prepare")->second;
            json newMsg = prepareMsg;
            newMsg["MessageType"] = "prepare";
            newMsg["from_id"] = peer->publicId();

            peer->multicast(newMsg, consensus->getMembers());
            mm.insert({ "prepare", newMsg });

            std::cout << "moving to prepare" << std::endl;
            changePhase(consensus, PBFTPreparePhase::instance());
            consensus->runPhase(peer);
        }
    }
}

void PBFTPreparePhase::runPhase(Consensus* consensus, Peer* peer) {
    int seqNum = consensus->_confirmedTrans.size();
    auto& mm = consensus->_receivedMessages[seqNum];
    int prepareCount = static_cast<int>(mm.count("prepare"));
    int neighborCount = static_cast<int>(consensus->getMembers().size());
    if (prepareCount > (2 * neighborCount / 3)) {
        json prepareMsg = mm.find("prepare")->second;
        json commitMsg = prepareMsg;
        commitMsg["MessageType"] = "commit";
        commitMsg["from_id"] = peer->publicId();

        peer->multicast(commitMsg, consensus->getMembers());
        mm.insert({ "commit", commitMsg });
        std::cout << "moving to commit" << std::endl;
        changePhase(consensus, PBFTCommitPhase::instance());
        consensus->runPhase(peer);
    }
}

void PBFTCommitPhase::runPhase(Consensus* consensus, Peer* peer) {
    int seqNum = consensus->_confirmedTrans.size();
    auto& mm = consensus->_receivedMessages[seqNum];

    int commitCount = static_cast<int>(mm.count("commit"));
    int neighborCount = static_cast<int>(peer->neighbors().size());

    if (commitCount > (2 * neighborCount / 3)) {
        // Add to confirmed
        json confirmed = mm.find("commit")->second;
        consensus->_confirmedTrans.push_back(confirmed["proposal"]["Request"]);
        consensus->_latency += RoundManager::currentRound() - confirmed["roundSubmitted"].get<int>();
        if (confirmed["proposal"]["value"] == false) {
            consensus->_confirmedZero = true;
            json notes;
            notes["round"] = RoundManager::currentRound();
            notes["committee"] = consensus->getId();
            notes["peer"] = peer->publicId();
            LogWriter::pushValue("safetyViolation", notes);
        }
        std::cout << "moving to prePrepare" << std::endl;
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
    // Assign a PBFTConsensus instance using this committee to each peer
    for (auto peer : peers) {
		// Create and configure a committee
		Committee* committeePtr = new Committee(committeeId);
		for (auto peer : peers) {
			committeePtr->addMember(peer->publicId());
		}
        PBFTConsensus* pbft = new PBFTConsensus(committeePtr);
		pbft->_leader = leaderId;
        peer->consensuses[committeeId] = pbft;
	    peer->faultManager.addFault(new EquivocateFault());
	}
}

void PBFTPeerV3::endOfRound(vector<Peer*>& _peers) {
	const vector<PBFTPeerV3*> peers = reinterpret_cast<vector<PBFTPeerV3*> const&>(_peers);
	
    double length = 0;
    double latency = 0;
    bool confirmedZero = false;
    for (auto consensus : peers[0]->consensuses) {
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
