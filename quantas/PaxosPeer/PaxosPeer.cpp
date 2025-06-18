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

	// clears state by setting all values to default values
	void PaxosPeer::clearState() {
		Paper tmp;
		paperData = tmp;

		Ledger tmp2;
		ledgerData = tmp2;
	}

	void PaxosPeer::checkInStrm() {
		while(!inStreamEmpty()) {
			PaxosPeerMessage newMsg = popInStream().getMessage();

			if (newMsg.messageType == "NextBallot" && newMsg.slotNumber == ledgerData.currentSlot) {
				if (newMsg.ballotNum > ledgerData.nextBal && newMsg.ballotNum > ledgerData.lastTried) {
					if (paperData.status != Paper::IDLE) {
						paperData.status = Paper::IDLE;
						paperData.quorum.clear();
						paperData.prevVotes.clear();
						paperData.voters.clear();
						paperData.paperDecree = -1;
					}

					/* debug statements */
					std::cerr << "Peer " << id() << " received NextBallot with ballot number " << newMsg.ballotNum << std::endl;

					paperData.timer = 0;
					ledgerData.nextBal = newMsg.ballotNum;
					PaxosPeerMessage reply = lastMessage();
					sendMessage(newMsg.Id, reply);

					std::cerr << "Peer " << id() << " sending LastMessage with ballot number " << reply.ballotNum << std::endl;
				}
			}
			else if (newMsg.messageType == "LastMessage" && newMsg.slotNumber == ledgerData.currentSlot) {
				if (paperData.status == Paper::TRYING) {
					paperData.prevVotes.insert(newMsg.Id);

					std::cerr << "Peer " << id() << " received LastMessage with ballot number " << newMsg.ballotNum << std::endl;

					if (paperData.prevVotes.size() > neighbors().size()/2 + 1) {
						// beginBallot function sets status to polling
						// so this will only occur once
						
						PaxosPeerMessage poll = beginBallot();
						std::cerr << "Peer " << id() << " sending BeginBallot with ballot number " << poll.ballotNum << std::endl;
						paperData.quorum = paperData.prevVotes;
						for (auto i : paperData.quorum) {
							sendMessage(i, poll);
						}
					}
				}
			}
			else if (newMsg.messageType == "BeginBallot" && newMsg.slotNumber == ledgerData.currentSlot) {
				std::cerr << "Peer " << id() << " received BeginBallot with ballot number " << newMsg.ballotNum << std::endl;
				if (newMsg.ballotNum == ledgerData.nextBal && newMsg.ballotNum > ledgerData.prevBal) {
					ledgerData.prevBal = newMsg.ballotNum;
					paperData.timer = 0;

					// probably need mroe in depth checks for voting
					// to make sure the decree is correct
					PaxosPeerMessage reply = voted();
					std::cerr << "Peer " << id() << " sending Voted with ballot number " << reply.ballotNum << std::endl;
					sendMessage(newMsg.Id, reply);
				}
			}
			else if (newMsg.messageType == "Voted" && newMsg.slotNumber == ledgerData.currentSlot) {
				// submitBallot function deals with checking if all members of the quorum have voted
				std::cerr << "Peer " << id() << " received Voted with ballot number " << newMsg.ballotNum << std::endl;
				if (paperData.status == Paper::POLLING) {
					paperData.voters.insert(newMsg.Id);
					if (paperData.voters == paperData.prevVotes) {
						ledgerData.outcome = newMsg.decree;
						latency = getRound() - roundSent;

						// success message currently handled in submitBallot function
						// which might be worth changing
					}
				}
			}
			else if (newMsg.messageType == "Success" ) {
				ledgerData.outcome = newMsg.decree;
				confirmedTrans.insert(std::make_pair(ledgerData.currentSlot,newMsg));
				++ledgerData.currentSlot;
				std::cerr << "Peer " << id() << " received Success with ballot number " << newMsg.ballotNum << std::endl;
				clearState();
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
		message.slotNumber = ledgerData.currentSlot;
		
		paperData.status = Paper::TRYING;
		paperData.prevVotes.clear();
		paperData.quorum.clear();
		paperData.voters.clear();

		return message;
	}

	// initializes and returns a lastMessage message
	PaxosPeerMessage PaxosPeer::lastMessage() {
		PaxosPeerMessage message;
		message.messageType = "LastMessage";
		message.lastVoted = ledgerData.prevBal;
		message.ballotNum = ledgerData.nextBal;
		message.Id = id();
		message.slotNumber = ledgerData.currentSlot;
		return message;
	}

	// initializes and returns a beginBallot message
	PaxosPeerMessage PaxosPeer::beginBallot() {
		PaxosPeerMessage message;
		message.messageType = "BeginBallot";
		message.ballotNum = ledgerData.lastTried;
		message.decree = paperData.paperDecree;
		message.Id = id();
		message.slotNumber = ledgerData.currentSlot;

		paperData.status = Paper::POLLING;
		paperData.voters.clear();

		return message;
	}

	PaxosPeerMessage PaxosPeer::voted() {
		PaxosPeerMessage message;
		message.messageType = "Voted";
		message.ballotNum = ledgerData.nextBal;
		message.decree = paperData.paperDecree;
		message.Id = id();
		message.slotNumber = ledgerData.currentSlot;

		ledgerData.prevBal = ledgerData.nextBal;
		
		return message;
	}

	void PaxosPeer::sendMessage(long peer, PaxosPeerMessage message) {
		Packet<PaxosPeerMessage> newMessage(getRound(), peer, id());
		newMessage.setMessage(message);
		pushToOutSteam(newMessage);
	}

	void PaxosPeer::submitBallot() {
		// checks that peer can submit a new ballot and that peer is waiting on ballot
		if (paperData.status == Paper::IDLE && ledgerData.nextBal != -1) {
			// currently this wait time is arbitrarily decided
			if (paperData.timer > 11) {
				PaxosPeerMessage ballot = nextBallot();
				std::cerr << "Peer " << id() << " sending NextBallot with ballot number " << ballot.ballotNum << std::endl;
				broadcast(ballot);
				roundSent = getRound();
			}
			else
				++paperData.timer;
		}
		else if (paperData.status == Paper::IDLE && ledgerData.nextBal == -1) {
			PaxosPeerMessage ballot = nextBallot();
			std::cerr << "Peer " << id() << " sending NextBallot with ballot number " << ballot.ballotNum << std::endl;
			broadcast(ballot);
			roundSent = getRound();
		}
		// this might end up placed in the checkInStrm function
		else if (paperData.status == Paper::POLLING && paperData.quorum == paperData.voters) {
			PaxosPeerMessage successMessage;
			successMessage.ballotNum = ledgerData.lastTried;
			successMessage.decree = ledgerData.outcome;
			successMessage.Id = id();
			successMessage.messageType = "Success";
			successMessage.slotNumber = ledgerData.currentSlot;
			broadcast(successMessage);

			// once consensus is achieved the peer starts working on next slot
			confirmedTrans.insert(std::make_pair(ledgerData.currentSlot,successMessage));
			++ledgerData.currentSlot;
			clearState();
		}
	}

	void PaxosPeer::performComputation() {
		if (true)
			checkInStrm();
		if (true)
			submitBallot();

	}

	void PaxosPeer::endOfRound(const vector<Peer<PaxosPeerMessage>*>& _peers) {
		const vector<PaxosPeer*> peers = reinterpret_cast<vector<PaxosPeer*> const&>(_peers);
		double satisfied = peers[0]->confirmedTrans.size();
		double lat = 0;
		// might be possible for number of latencys to not match number of 
		// confirmed transactions. probably rare but could be worth looking into
		for (int i = 0; i < peers.size(); i++) {
			if (satisfied < peers[i]->confirmedTrans.size())
				satisfied = peers[i]->confirmedTrans.size();
			lat += peers[i]->latency;
		}
		LogWriter::getTestLog()["latency"].push_back(lat / satisfied);
	}

	
	
	Simulation<quantas::PaxosPeerMessage, quantas::PaxosPeer>* generateSim() {
        
        Simulation<quantas::PaxosPeerMessage, quantas::PaxosPeer>* sim = new Simulation<quantas::PaxosPeerMessage, quantas::PaxosPeer>;
        return sim;
    }
}
