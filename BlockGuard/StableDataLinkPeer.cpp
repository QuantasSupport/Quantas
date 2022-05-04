//
//  StableDataLinkPeer.cpp
//  BlockGuard
//
//  Created by Joseph Oglio on 4/14/22.
//  Copyright © 2022 Kent State University. All rights reserved.
//

#include <iostream>
#include "StableDataLinkPeer.hpp"

namespace blockguard {

	int StableDataLinkPeer::currentTransaction = 1;

	StableDataLinkPeer::~StableDataLinkPeer() {

	}

	StableDataLinkPeer::StableDataLinkPeer(const StableDataLinkPeer& rhs) : Peer<StableDataLinkMessage>(rhs) {
		
	}

	StableDataLinkPeer::StableDataLinkPeer(long id) : Peer(id) {
		
	}

	void StableDataLinkPeer::performComputation() {
		if (alive) {
			if (getRound() == 0 && id() == 0) {
				submitTrans(currentTransaction);
			}
			if (previousMessageRound + timeOutRate < getRound()) {// resend lost message
				if (id() == 0) {
					StableDataLinkMessage message;
					message.action = "data";
					message.roundSubmitted = getRound(); // if message lost roundSubmitted isn't accurate
					message.messageNum = currentTransaction - 1;
					previousMessageRound = getRound();
					sendMessage(1, message);
				}
				else {
					StableDataLinkMessage message;
					message.action = "ack";
					message.roundSubmitted = getRound(); // if message lost roundSubmitted isn't accurate
					message.messageNum = currentTransaction - 1;
					previousMessageRound = getRound();
					sendMessage(0, message);
				}
			}
			while (!inStreamEmpty()) {
				Packet<StableDataLinkMessage> packet = popInStream();
				long source = packet.sourceId();
				StableDataLinkMessage message = packet.getMessage();
				if (rand() % messageLossDen < messageLossNum) { // current hack for message loss
					continue;
				}
				if (message.action == "ack") {
					ack++;
					if (ack < 3 * c + 2) {
						message.action = "data";
						previousMessageRound = getRound();
						sendMessage(1, message);
					}
					else {
						requestsSatisfied++;
						previousMessageRound = getRound();
						submitTrans(currentTransaction);
						ack = 0;
					}
				}
				else if (message.action == "data") {
					message.action = "ack";
					previousMessageRound = getRound();
					sendMessage(0, message);
				}
			}
		}
	}

	void StableDataLinkPeer::endOfRound(const vector<Peer<StableDataLinkMessage>*>& _peers) {
		const vector<StableDataLinkPeer*> peers = reinterpret_cast<vector<StableDataLinkPeer*> const&>(_peers);
		if (getRound() == 100) {
			int satisfied = 0;
			double messages = 0;
			for (int i = 0; i < peers.size(); i++) {
				satisfied += peers[i]->requestsSatisfied;
				messages += peers[i]->messagesSent;
			}
			LogWritter::instance()->data["tests"][LogWritter::instance()->getTest()]["utility"].push_back(satisfied / messages * 100);
		}
	}

	void StableDataLinkPeer::sendMessage(long peer, StableDataLinkMessage message) {
		Packet<StableDataLinkMessage> newMessage(getRound(), peer, id());
		newMessage.setMessage(message);
		pushToOutSteam(newMessage);
		messagesSent++;
	}

	void StableDataLinkPeer::submitTrans(int tranID) {
		StableDataLinkMessage message;
		message.action = "data";
		message.roundSubmitted = getRound();
		message.messageNum = tranID;
		sendMessage(1, message);
		currentTransaction++;
	}

	std::ostream& StableDataLinkPeer::printTo(std::ostream& out)const {
		Peer<StableDataLinkMessage>::printTo(out);

		out << id() << std::endl;
		out << "counter:" << getRound() << std::endl;

		return out;
	}

	std::ostream& operator<< (std::ostream& out, const StableDataLinkPeer& peer) {
		peer.printTo(out);
		return out;
	}

}