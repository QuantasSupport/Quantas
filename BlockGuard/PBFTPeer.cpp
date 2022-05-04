//
//  BitcoinPeer.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/15/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <iostream>
#include "PBFTPeer.hpp"

namespace blockguard {

	int PBFTPeer::currentTransaction = 1;

	PBFTPeer::~PBFTPeer() {

	}

	PBFTPeer::PBFTPeer(const PBFTPeer& rhs) : Peer<PBFTPeerMessage>(rhs) {
		_counter = rhs._counter;
	}

	PBFTPeer::PBFTPeer(long id) : Peer(id) {
		_counter = 0;
	}

	void PBFTPeer::performComputation() {
		if (id() == 0 && _counter == 0) {
			submitTrans(currentTransaction);
		}
		if (true)
			checkInStrm();

		if (true)
			checkContents();

		_counter++;
	}

	void PBFTPeer::endOfRound(const vector<Peer<PBFTPeerMessage>*>& _peers) {
		if (_counter == 1000) {
			const vector<PBFTPeer*> peers = reinterpret_cast<vector<PBFTPeer*> const&>(_peers);
			double length = peers[0]->confirmedTrans.size();
			LogWritter::instance()->data["tests"][LogWritter::instance()->getTest()]["latency"].push_back(latency / length);
		}
	}

	void PBFTPeer::checkInStrm() {
		while (!inStreamEmpty()) {
			Packet<PBFTPeerMessage> newMsg = popInStream();
			
			if (newMsg.getMessage().messageType == "trans") {
				transactions.push_back(newMsg.getMessage());
			}
			else {
				while (receivedMessages.size() < newMsg.getMessage().sequenceNum + 1) {
					receivedMessages.push_back(vector<PBFTPeerMessage>());
				}
				receivedMessages[newMsg.getMessage().sequenceNum].push_back(newMsg.getMessage());
			}
		}
	}
	void PBFTPeer::checkContents() {
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
					PBFTPeerMessage message = transactions[i];
					message.messageType = "pre-prepare";
					message.Id = id();
					message.sequenceNum = sequenceNum;
					broadcast(message);
					if (receivedMessages.size() < sequenceNum + 1) {
						receivedMessages.push_back(vector<PBFTPeerMessage>());
					}
					receivedMessages[sequenceNum].push_back(message);
					break;
				}
			}
		} else if (status == "pre-prepare" && receivedMessages.size() >= sequenceNum + 1) {
			for (int i = 0; i < receivedMessages[sequenceNum].size(); i++) {
				PBFTPeerMessage message = receivedMessages[sequenceNum][i];
				if (message.messageType == "pre-prepare") {
					status = "prepare";
					PBFTPeerMessage newMsg = message;
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
				PBFTPeerMessage message = receivedMessages[sequenceNum][i];
				if (message.messageType == "prepare") {
					count++;
				}
			}
			if (count > (neighbors().size() * 2 / 3) + 1) {
				status = "commit";
				PBFTPeerMessage newMsg = receivedMessages[sequenceNum][0];
				newMsg.messageType = "commit";
				newMsg.Id = id();
				broadcast(newMsg);
				receivedMessages[sequenceNum].push_back(newMsg);
			}
		}

		if (status == "commit") {
			int count = 0;
			for (int i = 0; i < receivedMessages[sequenceNum].size(); i++) {
				PBFTPeerMessage message = receivedMessages[sequenceNum][i];
				if (message.messageType == "commit") {
					count++;
				}
			}
			if (count > (neighbors().size() * 2 / 3) + 1) {
				status = "pre-prepare";
				confirmedTrans.push_back(receivedMessages[sequenceNum][0]);
				latency += _counter - receivedMessages[sequenceNum][0].roundSubmitted;
				sequenceNum++;
				if (id() == 0) {
					submitTrans(currentTransaction);
				}
				checkContents();
			}
		}

	}

	void PBFTPeer::submitTrans(int tranID) {
		PBFTPeerMessage message;
		message.messageType = "trans";
		message.trans = tranID;
		message.Id = id();
		message.roundSubmitted = _counter;
		broadcast(message);
		transactions.push_back(message);
		currentTransaction++;
	}

	ostream& PBFTPeer::printTo(ostream& out)const {
		Peer<PBFTPeerMessage>::printTo(out);

		out << id() << endl;
		out << "counter:" << _counter << endl;

		return out;
	}

	ostream& operator<< (ostream& out, const PBFTPeer& peer) {
		peer.printTo(out);
		return out;
	}
}