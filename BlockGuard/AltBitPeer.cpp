//
//  AltBitPeer.cpp
//  BlockGuard
//
//  Created by Joseph Oglio on 4/14/22.
//  Copyright © 2022 Kent State University. All rights reserved.
//

#include <iostream>
#include "AltBitPeer.hpp"

namespace blockguard {

	int AltBitPeer::currentTransaction = 1;

	AltBitPeer::~AltBitPeer() {

	}

	AltBitPeer::AltBitPeer(const AltBitPeer& rhs) : Peer<AltBitMessage>(rhs) {
		_counter = rhs._counter;
	}

	AltBitPeer::AltBitPeer(long id) : Peer(id) {
		_counter = 0;
	}

	void AltBitPeer::performComputation() {
		if (alive) {
			if (_counter == 0 && id() == 0) {
				submitTrans(currentTransaction);
			}
			if (previousMessageRound + timeOutRate < _counter) {// resend lost message
				if (id() == 0) {
					AltBitMessage message;
					message.action = "data";
					message.roundSubmitted = _counter; // if message lost roundSubmitted isn't accurate
					message.messageNum = ns;
					sendMessage(1, message);
				}
				else {
					AltBitMessage message;
					message.action = "ack";
					message.roundSubmitted = _counter; // if message lost roundSubmitted isn't accurate
					message.messageNum = ns;
					sendMessage(0, message);
				}
			}
			while (!inStreamEmpty()) {
				Packet<AltBitMessage> packet = popInStream();
				long source = packet.sourceId();
				AltBitMessage message = packet.getMessage();
				if (rand() % messageLossDen < messageLossNum) { // current hack for message loss
					continue;
				}
				if (message.action == "ack") {
					if (message.messageNum == ns) {
						previousMessageRound = _counter;
						requestsSatisfied++;
						ns++;
						submitTrans(currentTransaction);
					}

				}
				else if (message.action == "data") {
					previousMessageRound = _counter;
					ns = message.messageNum;
					message.action = "ack";
					sendMessage(0, message);

				}
			}
			_counter++;
		}
	}
	void AltBitPeer::endOfRound(const vector<Peer<AltBitMessage>*>& _peers) {
		const vector<AltBitPeer*> peers = reinterpret_cast<vector<AltBitPeer*> const&>(_peers);
		if (_counter == 100) {
			int satisfied = 0;
			double messages = 0;
			for (int i = 0; i < peers.size(); i++) {
				satisfied += peers[i]->requestsSatisfied;
				messages += peers[i]->messagesSent;
			}

			LogWritter::instance()->data["tests"][LogWritter::instance()->getTest()]["utility"].push_back(satisfied / messages);
		}
	}

	void AltBitPeer::sendMessage(long peer, AltBitMessage message) {
		Packet<AltBitMessage> newMessage(_counter, peer, id());
		newMessage.setMessage(message);
		pushToOutSteam(newMessage);
		messagesSent++;
	}

	void AltBitPeer::submitTrans(int tranID) {
		AltBitMessage message;
		message.action = "data";
		message.roundSubmitted = _counter;
		message.messageNum = ns;
		sendMessage(1, message);
		currentTransaction++;
	}

	std::ostream& AltBitPeer::printTo(std::ostream& out)const {
		Peer<AltBitMessage>::printTo(out);

		out << id() << std::endl;
		out << "counter:" << _counter << std::endl;

		return out;
	}

	std::ostream& operator<< (std::ostream& out, const AltBitPeer& peer) {
		peer.printTo(out);
		return out;
	}

}