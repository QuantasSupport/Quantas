//
//  BitcoinPeer.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/15/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <iostream>
#include "RaftPeer.hpp"

namespace blockguard {

	int RaftPeer::currentTransaction = 1;

	RaftPeer::~RaftPeer() {

	}

	RaftPeer::RaftPeer(const RaftPeer& rhs) : Peer<RaftPeerMessage>(rhs) {
		
	}

	RaftPeer::RaftPeer(long id) : Peer(id) {
		
	}

	void RaftPeer::performComputation() {
		if (true)
			checkInStrm();

		if (getRound() == 0) {
			submitTrans(currentTransaction);
		}

		if (timeOutRound <= getRound()) {
			RaftPeerMessage newMsg;
			newMsg.messageType = "elect";
			newMsg.Id = id();
			candidate = id();
			leaderId = -1;
			newMsg.termNum = ++term;
			votes.clear();
			resetTimer();
			broadcast(newMsg);
		}
	}

	void RaftPeer::endOfRound(const vector<Peer<RaftPeerMessage>*>& _peers) {
		if (getRound() == 1000) {
			const vector<RaftPeer*> peers = reinterpret_cast<vector<RaftPeer*> const&>(_peers);
			double satisfied = 0;
			double lat = 0;
			for (int i = 0; i < peers.size(); i++) {
				satisfied += peers[i]->requestsSatisfied;
				lat += peers[i]->latency;
			}
			LogWritter::instance()->data["tests"][LogWritter::instance()->getTest()]["latency"].push_back(lat / satisfied);
		}
	}

	void RaftPeer::checkInStrm() {
		while (!inStreamEmpty()) {
			RaftPeerMessage Msg = popInStream().getMessage();
			if (Msg.messageType == "request") {
				if (term <= Msg.termNum) {
					term = Msg.termNum;
					leaderId = Msg.Id;
					votes.clear();
					candidate = -1;
					resetTimer();
					RaftPeerMessage newMsg;
					newMsg.messageType = "respondRequest";
					newMsg.trans = Msg.trans;
					newMsg.Id = id();
					newMsg.roundSubmitted = Msg.roundSubmitted;
					sendMessage(Msg.Id, newMsg);
				}
			}
			else if (Msg.messageType == "respondRequest") {
				replys[Msg.trans].push_back(Msg.Id);
				if (replys[Msg.trans].size() == neighbors().size() / 2) {
					requestsSatisfied++;
					latency += getRound() - Msg.roundSubmitted;
					submitTrans(currentTransaction);
				}
			}
			else if (Msg.messageType == "vote") {
				if (leaderId != id()) {
					if (Msg.trans == id()) {
						votes.push_back(Msg.Id);

						// Get enough votes and win the election
						if (votes.size() > neighbors().size() / 2) {
							resetTimer();
							leaderId = id();
							candidate == -1;
							votes.clear();
							submitTrans(currentTransaction);
						}
					}
				}
			}
			else if (Msg.messageType == "elect") {
				if (term < Msg.termNum) {
					// Reset timeout
					leaderId = Msg.Id;
					term = Msg.termNum;
					votes.clear();
					candidate = Msg.Id;
					resetTimer();

					RaftPeerMessage newMsg;
					newMsg.messageType = "vote";
					newMsg.Id = id();
					newMsg.termNum = Msg.termNum;
					newMsg.roundSubmitted = Msg.roundSubmitted;
					newMsg.trans = Msg.Id; // also used to indicate who the vote is for
					sendMessage(Msg.Id, newMsg);
				}
			}
		}
	}

	void RaftPeer::submitTrans(int tranID) {
		if (leaderId == id()) {
			RaftPeerMessage message;
			message.messageType = "request";
			message.trans = tranID;
			message.Id = id();
			message.termNum = term;
			message.roundSubmitted = getRound();
			broadcast(message);
			resetTimer();
			currentTransaction++;
		}
	}

	void RaftPeer::resetTimer() {
		timeOutRound = (rand() % timeOutRandom) + timeOutSpacing + getRound();
	}

	void RaftPeer::sendMessage(long peer, RaftPeerMessage message) {
		Packet<RaftPeerMessage> newMessage(getRound(), peer, id());
		newMessage.setMessage(message);
		pushToOutSteam(newMessage);
	}

	ostream& RaftPeer::printTo(ostream& out)const {
		Peer<RaftPeerMessage>::printTo(out);

		out << id() << endl;
		out << "counter:" << getRound() << endl;

		return out;
	}

	ostream& operator<< (ostream& out, const RaftPeer& peer) {
		peer.printTo(out);
		return out;
	}
}