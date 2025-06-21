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
					//std::cerr << "Peer " << id() << " received NextBallot with ballot number " << newMsg.ballotNum << std::endl;

					paperData.timer = 0;
					ledgerData.nextBal = newMsg.ballotNum;
					PaxosPeerMessage reply = lastMessage();
					sendMessage(newMsg.Id, reply);

					//std::cerr << "Peer " << id() << " sending LastMessage with ballot number " << reply.ballotNum << std::endl;
				}
			}
			else if (newMsg.messageType == "LastMessage" && newMsg.slotNumber == ledgerData.currentSlot) {
				if (paperData.status == Paper::TRYING) {
					paperData.prevVotes.push_back(newMsg);
					//std::cerr << "Peer " << id() << " received LastMessage with ballot number " << newMsg.ballotNum << std::endl;

					// only need half because the peer will vote for itself
					// therefore we get neighbors().size()/2 + 1 as majority
					if (paperData.prevVotes.size() >= neighbors().size()/2) {
						// beginBallot function sets status to polling
						// so this will only occur once
						
						PaxosPeerMessage poll = beginBallot();
						//std::cerr << "Peer " << id() << " sending BeginBallot with ballot number " << poll.ballotNum << std::endl;
						
						// adding ids to quorum set
						for (int i = 0; i < paperData.prevVotes.size(); ++i) {
							paperData.quorum.insert(paperData.prevVotes[i].Id);
						}

						for (auto i : paperData.quorum) {
							sendMessage(i, poll);
						}
					}
				}
			}
			else if (newMsg.messageType == "BeginBallot" && newMsg.slotNumber == ledgerData.currentSlot) {
				//std::cerr << "Peer " << id() << " received BeginBallot with ballot number " << newMsg.ballotNum << std::endl;
				if (newMsg.ballotNum == ledgerData.nextBal && newMsg.ballotNum > ledgerData.prevBal) {
					ledgerData.prevBal = newMsg.ballotNum;
					ledgerData.ledgerDecree = newMsg.decree;

					paperData.timer = 0;

					PaxosPeerMessage reply = voted();
					//std::cerr << "Peer " << id() << " sending Voted with ballot number " << reply.ballotNum << "for decree "<< newMsg.decree<< std::endl;
					sendMessage(newMsg.Id, reply);
				}
			}
			else if (newMsg.messageType == "Voted" && newMsg.slotNumber == ledgerData.currentSlot) {
				// submitBallot function deals with checking if all members of the quorum have voted
				//std::cerr << "Peer " << id() << " received Voted with ballot number " << newMsg.ballotNum << std::endl;
				if (paperData.status == Paper::POLLING) {
					paperData.voters.insert(newMsg.Id);
					if (paperData.voters.size() == paperData.prevVotes.size()) {
						ledgerData.outcome = newMsg.decree;
						PaxosPeerMessage reply = success();
						broadcast(reply);

						latency += getRound() - roundSent;
						++throughput;

						// once consensus is achieved the peer starts working on next slot

						confirmedTrans.insert(std::make_pair(ledgerData.currentSlot,ledgerData.outcome));
						++ledgerData.currentSlot;
						int tmpSlot = ledgerData.currentSlot;
						clearState();
						ledgerData.currentSlot = tmpSlot;
					}
				}
			}
			else if (newMsg.messageType == "Success" ) {
				std::cerr << "Peer " << id() << " received Success with ballot number " << newMsg.ballotNum.first << " " << newMsg.ballotNum.second << std::endl;
				ledgerData.outcome = newMsg.decree;
				std::cerr << "outcome was: " << ledgerData.outcome << " for slot " << ledgerData.currentSlot << std::endl;
				std::pair<int,string> tmp(newMsg.slotNumber,newMsg.decree);
				confirmedTrans.insert(tmp);
				++ledgerData.currentSlot;
				int tmpSlot = ledgerData.currentSlot; 
				clearState(); // sets ledger to default values so slot must be set again
				ledgerData.currentSlot = tmpSlot;
			}
		}
	}

	// initializes and returns a nextBallot message
	PaxosPeerMessage PaxosPeer::nextBallot() {
		// each peer needs own disjoint set of ballot numbers
		/* implement as in leslie lamport paper MAYBE */
		while (ballotIndex <= ledgerData.nextBal.first) {
			++ballotIndex;
		}

		if (ledgerData.nextBal.first == -1)
			ballotIndex = 1;

		ledgerData.lastTried.first = ballotIndex;
		ledgerData.lastTried.second = id();

		PaxosPeerMessage message;
		message.messageType = "NextBallot";
		message.ballotNum = ledgerData.lastTried;
		message.slotNumber = ledgerData.currentSlot;
		message.Id = id();
		
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
		message.slotNumber = ledgerData.currentSlot;
		message.decree = ledgerData.ledgerDecree;
		message.Id = id();
		return message;
	}

	// initializes and returns a beginBallot message
	PaxosPeerMessage PaxosPeer::beginBallot() {
		PaxosPeerMessage message;
		message.messageType = "BeginBallot";
		message.ballotNum = ledgerData.lastTried;
		message.slotNumber = ledgerData.currentSlot;
		message.Id = id();

		// decree selection takes decree of most recent vote
		// from prevVotes set
		PaxosPeerMessage largest = paperData.prevVotes[0];
		for (int i = 0; i < paperData.prevVotes.size(); ++i) {
			if (paperData.prevVotes[i].ballotNum > largest.ballotNum) {
				largest = paperData.prevVotes[i];
			}
		}
		if (largest.decree != "") {
			//std::cerr << "largest decree: " << largest.decree << std::endl;
			message.decree = largest.decree;
		}
		else {
			char randDecree = 'A' + randMod(26);
			message.decree.push_back(randDecree);
			std::cerr << "char: " << randDecree << std::endl;
		}
		//std::cerr << "Chosen decree: " << message.decree << std::endl;

		paperData.status = Paper::POLLING;
		paperData.voters.clear();

		return message;
	}

	PaxosPeerMessage PaxosPeer::voted() {
		PaxosPeerMessage message;
		message.messageType = "Voted";
		message.ballotNum = ledgerData.nextBal;
		message.decree = ledgerData.ledgerDecree;
		message.Id = id();
		message.slotNumber = ledgerData.currentSlot;

		ledgerData.prevBal = ledgerData.nextBal;
		
		return message;
	}

	PaxosPeerMessage PaxosPeer::success() {
		PaxosPeerMessage message;
		message.messageType = "Success";
		message.ballotNum = ledgerData.lastTried;
		message.decree = ledgerData.outcome;
		message.slotNumber = ledgerData.currentSlot;
		message.Id = id();

		return message;
	}

	void PaxosPeer::sendMessage(long peer, PaxosPeerMessage message) {
		Packet<PaxosPeerMessage> newMessage(getRound(), peer, id());
		newMessage.setMessage(message);
		pushToOutSteam(newMessage);
	}

	void PaxosPeer::submitBallot() {
		// checks that peer can submit a new ballot and that peer is waiting on ballot
		if (paperData.status == Paper::IDLE && ledgerData.nextBal.first != -1) {
			// currently this wait time is arbitrarily decided
			if (paperData.timer > 8) {
				//std::cerr << "timer ran out" << std::endl;
				PaxosPeerMessage ballot = nextBallot();
				//std::cerr << "Peer " << id() << " sending NextBallot with ballot number " << ballot.ballotNum.first << " " << ballot.ballotNum.second << std::endl;
				broadcast(ballot);
				roundSent = getRound();
			}
			else
				++paperData.timer;
		}
		else if (paperData.status == Paper::IDLE && ledgerData.nextBal.first == -1) {
			PaxosPeerMessage ballot = nextBallot();
			//std::cerr << "Peer " << id() << " sending NextBallot with ballot number " << ballot.ballotNum << std::endl;
			broadcast(ballot);
			roundSent = getRound();
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
		double lat = 0;
		double satisfied = 0;
		// might be possible for number of latencys to not match number of 
		// confirmed transactions. probably rare but could be worth looking into
		for (int i = 0; i < peers.size(); i++) {
			satisfied += peers[i]->throughput;
			lat += peers[i]->latency;
		}
		LogWriter::getTestLog()["latency"].push_back(lat / satisfied);
		LogWriter::getTestLog()["throughput"].push_back(satisfied);

		/*
		if(id() == 0) {
			
			std::cerr << "Chosen decrees: size: " << confirmedTrans.size() << std::endl;
			std::cerr << "Current slot: " << ledgerData.currentSlot << std::endl;
			for (auto it = confirmedTrans.begin(); it != confirmedTrans.end(); ++it) {
				std::cerr << " Decree: " << it->second << " ";
			}
			
		}
			*/
	}

	
	
	Simulation<quantas::PaxosPeerMessage, quantas::PaxosPeer>* generateSim() {
        
        Simulation<quantas::PaxosPeerMessage, quantas::PaxosPeer>* sim = new Simulation<quantas::PaxosPeerMessage, quantas::PaxosPeer>;
        return sim;
    }
}
