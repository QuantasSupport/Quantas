/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "EyeWitnessPeer.hpp"

namespace quantas {

	int EyeWitnessPeer::currentTransaction = 1;

	EyeWitnessPeer::~EyeWitnessPeer() {

	}

	EyeWitnessPeer::EyeWitnessPeer(const EyeWitnessPeer& rhs) : Peer<EyeWitnessPeerMessage>(rhs) {
		
	}

	EyeWitnessPeer::EyeWitnessPeer(long id) : Peer(id) {

	}

	void EyeWitnessPeer::performComputation() {
		if (id() == 0 && getRound() == 0) {
			submitTrans(currentTransaction);
		}
		if (true)
			checkInStrm();

		if (true)
			checkContents();

	}

	void EyeWitnessPeer::endOfRound(const vector<Peer<EyeWitnessPeerMessage>*>& _peers) {
		const vector<EyeWitnessPeer*> peers = reinterpret_cast<vector<EyeWitnessPeer*> const&>(_peers);
		double length = peers[0]->confirmedTrans.size();
		LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["latency"].push_back(latency / length);
	}

	void EyeWitnessPeer::checkInStrm() {
		while (!inStreamEmpty()) {
			Packet<EyeWitnessPeerMessage> newMsg = popInStream();
			
			if (newMsg.getMessage().messageType == "trans") {
				transactions.push_back(newMsg.getMessage());
			}
			else {
				while (receivedMessages.size() < newMsg.getMessage().sequenceNum + 1) {
					receivedMessages.push_back(vector<EyeWitnessPeerMessage>());
				}
				receivedMessages[newMsg.getMessage().sequenceNum].push_back(newMsg.getMessage());
			}
		}
	}
	void EyeWitnessPeer::checkContents() {
		if (id() == 0 && status == "pre-prepare") {
			for (int i = 0; i < transactions.size(); i++) {
				bool skip = false;
				for (int j = 0; j < confirmedTrans.size(); j++) {
					if (transactions[i].trans == confirmedTrans[j].trans) {
						skip = true;
						break;
					}
				}
				if (!skip) {
					status = "prepare";
					EyeWitnessPeerMessage message = transactions[i];
					message.messageType = "pre-prepare";
					message.Id = id();
					message.sequenceNum = sequenceNum;
					broadcast(message);
					if (receivedMessages.size() < sequenceNum + 1) {
						receivedMessages.push_back(vector<EyeWitnessPeerMessage>());
					}
					receivedMessages[sequenceNum].push_back(message);
					break;
				}
			}
		} else if (status == "pre-prepare" && receivedMessages.size() >= sequenceNum + 1) {
			for (int i = 0; i < receivedMessages[sequenceNum].size(); i++) {
				EyeWitnessPeerMessage message = receivedMessages[sequenceNum][i];
				if (message.messageType == "pre-prepare") {
					status = "prepare";
					EyeWitnessPeerMessage newMsg = message;
					newMsg.messageType = "prepare";
					newMsg.Id = id();
					broadcast(newMsg);
					receivedMessages[sequenceNum].push_back(newMsg);
				}
			}
		}

		if (status == "prepare") {
			int count = 0;
			for (int i = 0; i < receivedMessages[sequenceNum].size(); i++) {
				EyeWitnessPeerMessage message = receivedMessages[sequenceNum][i];
				if (message.messageType == "prepare") {
					count++;
				}
			}
			if (count > (neighbors().size() * 2 / 3)) {
				status = "commit";
				EyeWitnessPeerMessage newMsg = receivedMessages[sequenceNum][0];
				newMsg.messageType = "commit";
				newMsg.Id = id();
				broadcast(newMsg);
				receivedMessages[sequenceNum].push_back(newMsg);
			}
		}

		if (status == "commit") {
			int count = 0;
			for (int i = 0; i < receivedMessages[sequenceNum].size(); i++) {
				EyeWitnessPeerMessage message = receivedMessages[sequenceNum][i];
				if (message.messageType == "commit") {
					count++;
				}
			}
			if (count > (neighbors().size() * 2 / 3)) {
				status = "pre-prepare";
				confirmedTrans.push_back(receivedMessages[sequenceNum][0]);
				latency += getRound() - receivedMessages[sequenceNum][0].roundSubmitted;
				sequenceNum++;
				if (id() == 0) {
					submitTrans(currentTransaction);
				}
				checkContents();
			}
		}

	}

	void EyeWitnessPeer::submitTrans(int tranID) {
		EyeWitnessPeerMessage message;
		message.messageType = "trans";
		message.trans = tranID;
		message.Id = id();
		message.roundSubmitted = getRound();
		broadcast(message);
		transactions.push_back(message);
		currentTransaction++;
	}

	ostream& EyeWitnessPeer::printTo(ostream& out)const {
		Peer<EyeWitnessPeerMessage>::printTo(out);

		out << id() << endl;
		out << "counter:" << getRound() << endl;

		return out;
	}

	ostream& operator<< (ostream& out, const EyeWitnessPeer& peer) {
		peer.printTo(out);
		return out;
	}

	Simulation<quantas::EyeWitnessPeerMessage, quantas::EyeWitnessPeer>* generateSim() {
        
        Simulation<quantas::EyeWitnessPeerMessage, quantas::EyeWitnessPeer>* sim = new Simulation<quantas::EyeWitnessPeerMessage, quantas::EyeWitnessPeer>;
        return sim;
    }
}