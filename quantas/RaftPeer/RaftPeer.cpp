/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "RaftPeer.hpp"

namespace quantas {

	static bool registerRaft = [](){
		registerPeerType("RaftPeer", 
			[](interfaceId pubId){ return new RaftPeer(pubId); });
		return true;
	}();

	int RaftPeer::currentTransaction = 1;

	RaftPeer::~RaftPeer() {

	}

	RaftPeer::RaftPeer(const RaftPeer& rhs) : Peer<RaftPeerMessage>(rhs) {
		
	}

	RaftPeer::RaftPeer(interfaceId id) : Peer(id) {
		
	}

	void RaftPeer::performComputation() {
		if (true)
			checkInStrm();

		if (RoundManager::instance()->currentRound() == 0) {
			submitTrans(currentTransaction);
		}

		if (timeOutRound <= RoundManager::instance()->currentRound()) {
			RaftPeerMessage newMsg;
			newMsg.messageType = "elect";
			newMsg.Id = publicId();
			candidate = publicId();
			leaderId = -1;
			newMsg.termNum = ++term;
			votes.clear();
			resetTimer();
			broadcast(newMsg);
		}
	}

	void RaftPeer::endOfRound(const vector<Peer*>& _peers) {
		const vector<RaftPeer*> peers = reinterpret_cast<vector<RaftPeer*> const&>(_peers);
		double satisfied = 0;
		double lat = 0;
		for (int i = 0; i < peers.size(); i++) {
			satisfied += peers[i]->requestsSatisfied;
			lat += peers[i]->latency;
		}
		LogWriter::getTestLog()["latency"].push_back(lat / satisfied);
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
					newMsg.Id = publicId();
					newMsg.roundSubmitted = Msg.roundSubmitted;
					sendMessage(Msg.Id, newMsg);
				}
			}
			else if (Msg.messageType == "respondRequest") {
				replys[Msg.trans].push_back(Msg.Id);
				if (replys[Msg.trans].size() == neighbors().size() / 2) {
					requestsSatisfied++;
					latency += RoundManager::instance()->currentRound()er::instance()->currentRound() - Msg.roundSubmitted;
					submitTrans(currentTransaction);
				}
			}
			else if (Msg.messageType == "vote") {
				if (leaderId != publicId()) {
					if (Msg.trans == publicId()) {
						votes.push_back(Msg.Id);

						// Get enough votes and win the election
						if (votes.size() > neighbors().size() / 2) {
							resetTimer();
							leaderId = publicId();
							candidate = -1;
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
					newMsg.Id = publicId();
					newMsg.termNum = Msg.termNum;
					newMsg.roundSubmitted = Msg.roundSubmitted;
					newMsg.trans = Msg.Id; // also used to indicate who the vote is for
					sendMessage(Msg.Id, newMsg);
				}
			}
		}
	}

	void RaftPeer::submitTrans(int tranID) {
		if (leaderId == publicId()) {
			RaftPeerMessage message;
			message.messageType = "request";
			message.trans = tranID;
			message.Id = publicId();
			message.termNum = term;
			message.roundSubmitted = RoundManager::instance()->currentRound();
			broadcast(message);
			resetTimer();
			currentTransaction++;
		}
	}

	void RaftPeer::resetTimer() {
		timeOutRound = (randMod(timeOutRandom)) + timeOutSpacing + RoundManager::instance()->currentRound()er::instance()->currentRound();
	}

	void RaftPeer::sendMessage(interfaceId peer, RaftPeerMessage message) {
		Packet newMessage(RoundManager::instance()->currentRound()er::instance()->currentRound(), peer, publicId());
		newMessage.setMessage(message);
		pushToOutSteam(newMessage);
	}

}
