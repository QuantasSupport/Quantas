/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "PaxosPeer.hpp"

namespace quantas {

	PaxosPeer::~PaxosPeer() {

	}

	PaxosPeer::PaxosPeer(const PaxosPeer& rhs) : Peer<PaxosPeerMessage>(rhs) {
		
	}

	PaxosPeer::PaxosPeer(long id) : Peer(id) {

	}

	void PaxosPeer::checkInStrm() {
		while(!inStreamEmpty()) {
			PaxosPeerMessage newMsg = popInStream().getMessage();

			if (newMsg.messageType == "NextBallot") {
				if (newMsg.ballotNum > ledgerData.nextBal && newMsg.ballotNum > ledgerData.lastTried) {
					if (paperData.status != IDLE) {
						paperData.status = IDLE;
						paperData.quorum.clear();
						paperData.prevVotes.clear();
						paperData.voters.clear();
						paperData.paperDecree = -1;
					}

					ledgerData.nextBal = newMsg.ballotNum;
					PaxosPeerMessage reply = lastMessage();
					sendMessage(newMsg.Id, reply);
				}
			}
			else if (newMsg.messageType == "LastMessage") {
				if (paperData.status == TRYING) {
					paperData.prevVotes.insert(newMsg.Id);
				}
			}
			else if (newMsg.messageType == "BeginBallot") {
				if (newMsg.ballotNum == ledgerData.nextBal && newMsg.ballotNum > ledgerData.prevBal) {
					ledgerData.prevBal = newMsg.ballotNum;

					// probably need mroe in depth checks for voting
					// to make sure the decree is correct
					PaxosPeerMessage reply = voted();
					sendMessage(newMsg.Id, reply);
				}
			}
			else if (newMsg.messageType == "Voted") {
				// submitBallot function deals with checking if all members of the quorum have voted
				if (paperData.status == POLLING) {
					paperData.votes.insert(newMsg.Id);
					if (paperData.voters == paperData.quorum)
						ledgerData.outcome = newMsg.decree;
				}
			}
			else if (newMsg.messageType == "Success") {
				ledgerData.outcome = newMsg.decree;
			}
		}
	}

	// initializes and returns a nextBallot message
	PaxosPeerMessage PaxosPeer::nextBallot() {
		// each peer needs own disjoint set of ballot numbers
		// ensures no collisions for up to 1000 peers between ballot numbers
		while (id() + ballotIndex * 1000 < ledgerData.nextBal && id() + ballotIndex * 1000 < ledgerData.lastTried) {
			++ballotIndex;
		}
		ledgerData.lastTried = id() + ballotIndex * 1000;
		PaxosPeerMessage message;
		message.messageType = "NextBallot";
		message.ballotNum = ledgerData.lastTried;
		message.Id = id();
		
		paperData.status = TRYING;
		paperData.prevVotes.clear();
		paperData.quorum.clear();
		paperData.voters.clear();

		/* need better solution for selecting quorum */
		for (int i = 0; i < neighbors().size(); i++) {
			paperData.quorum.insert(neighbors()[i]);
		}

		return message;
	}

	// initializes and returns a lastMessage message
	PaxosPeerMessage PaxosPeer::lastMessage() {
		PaxosPeerMessage message;
		message.messageType = "LastMessage";
		message.lastVoted = ledgerData.prevBal;
		message.ballotNum = ledgerData.nextBal;
		message.Id = id();
		return message;
	}

	// initializes and returns a beginBallot message
	PaxosPeerMessage PaxosPeer::beginBallot() {
		PaxosPeerMessage message;
		message.messageType = "BeginBallot";
		message.ballotNum = lastTried;
		message.decree = paperData.paperDecree;
		message.Id = id();

		paperData.status = POLLING;
		paperData.voters.clear();

		return message;
	}

	PaxosPeerMessage PaxosPeer::voted() {
		PaxosPeerMessage message;
		message.messageType = "Voted";
		message.ballotNum = ledgerData.nextBal;
		message.decree = paperData.paperDecree;
		message.Id = id();

		ledgerData.prevBal = ledgerData.nextBal;
		
		return message;
	}

	void PaxosPeer::sendMessage(long peer, PaxosPeerMessage message) {
		Packet<PaxosPeerMessage> newMessage(getRound(), peer, id());
		newMessage.setMessage(message);
		pushToOutSteam(newMessage);
	}

	void PaxosPeer::submitBallot() {
		if (paperData.status == IDLE) {
			
			// need to figure out how long nodes wait
			// until the next ballot is sent so that ballots
			// have a chance of success and arent immediately overriden

		}
		// this might end up placed in the checkInStrm function
		else if (paperData.status == POLLING && paperData.quorum == paperData.voters) {
			PaxosPeerMessage successMessage;
			successMessage.ballotNum = ledgerData.lastTried;
			successMessage.decree = ledgerData.outcome;
			successMessage.Id = id();
			successMessage.messageType = "Success";
			broadcast(successMessage);
		}
	}

	void PaxosPeer::performComputation() {
		if (true)
			checkInStrm();
		if (true)
			submitBallot();

	}

	void PaxosPeer::endOfRound(const vector<Peer<PaxosPeerMessage>*>& _peers) {
		const vector<PBFTPeer*> peers = reinterpret_cast<vector<PBFTPeer*> const&>(_peers);
		double length = peers[0]->confirmedTrans.size();
		LogWriter::getTestLog()["latency"].push_back(latency / length);
	}

	
	/*
	Simulation<quantas::PBFTPeerMessage, quantas::PBFTPeer>* generateSim() {
        
        Simulation<quantas::PBFTPeerMessage, quantas::PBFTPeer>* sim = new Simulation<quantas::PBFTPeerMessage, quantas::PBFTPeer>;
        return sim;
    }
	*/
}
