/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "PBFTPeerV2.hpp"

namespace quantas {

class EquivocateFault : public Fault {
public:
    virtual ~EquivocateFault() = default;

    bool overridesSendType(const std::string& sendType) const override { 
		if (sendType == "broadcast")
			return true;
		else 
			return false;
	}

    virtual bool onSend(Peer* peer, json& msg, const std::string& sendType, const std::set<interfaceId>& targets = std::set<interfaceId>()) {
		if (sendType == "broadcast") {
			if (msg.contains("value")) {
				auto neighbors = peer->neighbors();
				std::vector<interfaceId> temp(neighbors.begin(), neighbors.end());  // Copy set to vector
				std::shuffle(temp.begin(), temp.end(), threadLocalEngine());  // Shuffle vector
				std::set<interfaceId> subset1(temp.begin(), temp.begin() + temp.size() / 2);  // Take the first 'count' elements
				std::set<interfaceId> subset2(temp.begin() + temp.size() / 2, temp.end());
				json msg_copy = msg;
				peer->multicast(msg, subset1);
				msg_copy["value"] = !msg_copy["value"];
				peer->multicast(msg_copy, subset2);
				return true; 
			} else 
				return false;
		} else 
			return false;
	}
};

static bool registerPBFT = [](){
	PeerRegistry::registerPeerType("PBFTPeerV2", 
		[](interfaceId pubId){ return new PBFTPeerV2(new NetworkInterfaceAbstract(pubId)); });
	return true;
}();

int PBFTPeerV2::currentTransaction = 1;

PBFTPeerV2::~PBFTPeerV2() {

}

PBFTPeerV2::PBFTPeerV2(const PBFTPeerV2& rhs) : ByzantinePeer(rhs) {
	
}

PBFTPeerV2::PBFTPeerV2(NetworkInterface* networkInterface) : ByzantinePeer(networkInterface) {

}

void PBFTPeerV2::initParameters(const std::vector<Peer*>& _peers, json parameters) {
	const vector<PBFTPeerV2*> peers = reinterpret_cast<vector<PBFTPeerV2*> const&>(_peers);

	for (auto peer : peers) {
	    peer->faultManager.addFault(new EquivocateFault());
	}
}

void PBFTPeerV2::performComputation() {
	if (publicId() == getLeader() && RoundManager::currentRound() == 1) {
		submitTrans();
	}
	if (true)
		checkInStrm();

	if (true)
		checkContents();
}

void PBFTPeerV2::endOfRound(vector<Peer*>& _peers) {
	const vector<PBFTPeerV2*> peers = reinterpret_cast<vector<PBFTPeerV2*> const&>(_peers);

	/////////////////////////////////////////////
	// int totalPeers = peers.size();
	// int newLeader = (getLeader() + 1) % totalPeers;
	// for (auto peer : peers) {
	//     peer->updateLeader(newLeader);
	// }
	////////////////////////////////////////////
	
	double length = peers[0]->confirmedTrans.size();
	LogWriter::pushValue("latency", latency / length);
	LogWriter::pushValue("throughput", length);
}

void PBFTPeerV2::checkInStrm() {
	while (!inStreamEmpty()) {
		Packet packet = popInStream();
		json newMsg = packet.getMessage();
		if (newMsg["value"] == false) {
			// std::cout << "equivocated\n";
		}
		if (newMsg["messageType"] == "trans") {
			transactions.push_back(newMsg);
			// cout << "recieved transaction" << endl;
		}
		else {
			while (receivedMessages.size() < int(newMsg["sequenceNum"]) + 1) {
				receivedMessages.push_back(vector<json>());
			}
			receivedMessages[newMsg["sequenceNum"]].push_back(newMsg);
		}
	}
}
void PBFTPeerV2::checkContents() {
	
	if (publicId() == getLeader() && status == "pre-prepare") {
		for (int i = 0; i < transactions.size(); i++) {
			bool skip = false;
			for (int j = 0; j < confirmedTrans.size(); j++) {
				if (transactions[i]["trans"] == confirmedTrans[j]["trans"]) {
					skip = true;
					break;
				}
			}
			if (!skip) {
				status = "prepare";
				json message = transactions[i];
				message["messageType"] = "pre-prepare";
				message["Id"] = publicId();
				message["sequenceNum"] = sequenceNum;
				broadcast(message);
				if (receivedMessages.size() < sequenceNum + 1) {
					receivedMessages.push_back(vector<json>());
				}
				receivedMessages[sequenceNum].push_back(message);
				break;
			}
		}
	} else if (status == "pre-prepare" && receivedMessages.size() >= sequenceNum + 1) {
		for (int i = 0; i < receivedMessages[sequenceNum].size(); i++) {
			json message = receivedMessages[sequenceNum][i];
			if (message["messageType"] == "pre-prepare") {
				status = "prepare";
				json newMsg = message;
				newMsg["messageType"] = "prepare";
				newMsg["Id"] = publicId();
				receivedMessages[sequenceNum].push_back(newMsg);
				broadcast(newMsg);
			}
		}
	}

	if (status == "prepare") {
		int count = 0;
		for (int i = 0; i < receivedMessages[sequenceNum].size(); i++) {
			json message = receivedMessages[sequenceNum][i];
			if (message["messageType"] == "prepare") {
				count++;
			}
		}
		if (count > (neighbors().size() * 2 / 3)) {
			status = "commit";
			json newMsg = receivedMessages[sequenceNum][0];
			newMsg["messageType"] = "commit";
			newMsg["Id"] = publicId();
			receivedMessages[sequenceNum].push_back(newMsg);
			broadcast(newMsg);
		}
	}

	if (status == "commit") {
		int count = 0;
		for (int i = 0; i < receivedMessages[sequenceNum].size(); i++) {
			json message = receivedMessages[sequenceNum][i];
			if (message["messageType"] == "commit") {
				count++;
			}
		}
		if (count > (neighbors().size() * 2 / 3)) {
			status = "pre-prepare";
			confirmedTrans.push_back(receivedMessages[sequenceNum][0]);
			latency += RoundManager::currentRound() - int(receivedMessages[sequenceNum][0]["roundSubmitted"]);
			sequenceNum++;
			if (publicId() == getLeader()) {
				submitTrans();
			}
			checkContents();
		}
	}

}

void PBFTPeerV2::submitTrans() {
	json message;
	message["messageType"] = "trans";
	message["trans"] = currentTransaction;
	message["Id"] = publicId();
	message["roundSubmitted"] = RoundManager::currentRound();
	message["value"] = true;
	transactions.push_back(message);
	broadcast(message);
	currentTransaction++;
}

}
