/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "PBFTPeer.hpp"

namespace quantas {

	static bool registerPBFT = [](){
		PeerRegistry::registerPeerType("PBFTPeer", 
			[](interfaceId pubId){ return new PBFTPeer(pubId); });
		return true;
	}();

	int PBFTPeer::currentTransaction = 1;

	PBFTPeer::~PBFTPeer() {
		for_each(receivedMessages.begin(), receivedMessages.end(), [] (auto v) {
			for_each(v.begin(), v.end(), [](PBFTPeerMessage* msg) {
				delete msg;
			});
		});
		for_each(transactions.begin(), transactions.end(), [](PBFTPeerMessage* msg) {
			delete msg;
		});
		for_each(confirmedTrans.begin(), confirmedTrans.end(), [](PBFTPeerMessage* msg) {
			delete msg;
		});
	}

	PBFTPeer::PBFTPeer(const PBFTPeer& rhs) : Peer(rhs) {
		
	}

	PBFTPeer::PBFTPeer(interfaceId id) : Peer(id) {

	}

	void PBFTPeer::performComputation() {
		if (publicId() == getLeader() && RoundManager::currentRound() == 1) {
			submitTrans();
		}
		if (true)
			checkInStrm();

		if (true)
			checkContents();
	}

	void PBFTPeer::endOfRound(const vector<Peer*>& _peers) {
		const vector<PBFTPeer*> peers = reinterpret_cast<vector<PBFTPeer*> const&>(_peers);

		/////////////////////////////////////////////
        // int totalPeers = peers.size();
        // int newLeader = (getLeader() + 1) % totalPeers;
        // for (auto peer : peers) {
        //     peer->updateLeader(newLeader);
        // }
        ////////////////////////////////////////////

		double length = peers[0]->confirmedTrans.size();
		LogWriter::pushValue("latency", latency / length);
	}

	void PBFTPeer::checkInStrm() {
		while (!inStreamEmpty()) {
			Packet packet = popInStream();
			PBFTPeerMessage* newMsg = dynamic_cast<PBFTPeerMessage*>(packet.getMessage());
			
			if (newMsg->messageType == "trans") {
				transactions.push_back(newMsg);
				// cout << "recieved transaction" << endl;
			}
			else {
				while (receivedMessages.size() < newMsg->sequenceNum + 1) {
					receivedMessages.push_back(vector<PBFTPeerMessage*>());
				}
				receivedMessages[newMsg->sequenceNum].push_back(newMsg);
			}
		}
	}
	void PBFTPeer::checkContents() {
		if (status == "pre-prepare" && receivedMessages.size() >= sequenceNum + 1) {
			for (int i = 0; i < receivedMessages[sequenceNum].size(); i++) {
				PBFTPeerMessage* message = receivedMessages[sequenceNum][i];
				if (message->messageType == "pre-prepare") {
					status = "prepare";
					PBFTPeerMessage* newMsg = message->clone();
					newMsg->messageType = "prepare";
					newMsg->Id = publicId();
					receivedMessages[sequenceNum].push_back(newMsg->clone());
					broadcast(newMsg);
				}
			}
		}

		if (status == "prepare") {
			int count = 0;
			for (int i = 0; i < receivedMessages[sequenceNum].size(); i++) {
				PBFTPeerMessage* message = receivedMessages[sequenceNum][i];
				if (message->messageType == "prepare") {
					count++;
				}
			}
			if (count > (neighbors().size() * 2 / 3)) {
				status = "commit";
				PBFTPeerMessage* newMsg = receivedMessages[sequenceNum][0]->clone();
				newMsg->messageType = "commit";
				newMsg->Id = publicId();
				receivedMessages[sequenceNum].push_back(newMsg->clone());
				broadcast(newMsg);
			}
		}

		if (status == "commit") {
			int count = 0;
			for (int i = 0; i < receivedMessages[sequenceNum].size(); i++) {
				PBFTPeerMessage* message = receivedMessages[sequenceNum][i];
				if (message->messageType == "commit") {
					count++;
				}
			}
			if (count > (neighbors().size() * 2 / 3)) {
				status = "pre-prepare";
				confirmedTrans.push_back(receivedMessages[sequenceNum][0]->clone());
				latency += RoundManager::currentRound() - receivedMessages[sequenceNum][0]->roundSubmitted;
				sequenceNum++;
				if (publicId() == 0) {
					submitTrans(currentTransaction);
				}
				checkContents();
			}
		}

	}

	void PBFTPeer::submitTrans() {
		PBFTPeerMessage* message = new PBFTPeerMessage();
		message->messageType = "trans";
		message->trans = currentTransaction;
		message->Id = publicId();
		message->roundSubmitted = RoundManager::currentRound();
		transactions.push_back(message->clone());
		broadcast(message);
		currentTransaction++;
	}

}
