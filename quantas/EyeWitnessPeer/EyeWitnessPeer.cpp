/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "EyeWitnessPeer.hpp"

namespace quantas
{
	// =============== implementation of underlying consensus algorithm ===============

	void PBFTRequest::updateConsensus()
	{
		if (leader && status == "pre-prepare") {
			status = "prepare";
			EyeWitnessMessage message;
			message.sequenceNum = sequenceNum;
			message.messageType = "pre-prepare";
			message.trans = transaction;
			addToConsensus(message);
			outbox.push_back(message);
		} else if (status == "pre-prepare") {
			if (statusCount["pre-prepare"] > 0) {
				status = "prepare";
				EyeWitnessMessage message;
				message.sequenceNum = sequenceNum;
				message.messageType = "prepare";
				message.trans = transaction;
				addToConsensus(message);
				outbox.push_back(message);
		    }
		}

		if (status == "prepare") {
		    if (statusCount["prepare"] > (neighborhoodSize * 2 / 3)) {
		        status = "commit";
				EyeWitnessMessage message;
				message.sequenceNum = sequenceNum;
		        message.messageType = "prepare";
				message.trans = transaction;
				addToConsensus(message);
				outbox.push_back(message);
		    }
		}

		if (status == "commit") {
		    if (statusCount["commit"] > (neighborhoodSize * 2 / 3)) {
		        status = "committed";
		    }
		}
	}

	void PBFTRequest::addToConsensus(EyeWitnessMessage message)
	{
		// increment the number of times we've seen this type of message for this request
		statusCount[message.messageType]++;
	}

	bool PBFTRequest::consensusSucceeded() const
	{
		return status == "committed";
	}

	// ================= peer class that participates directly in the network =================

	int EyeWitnessPeer::issuedCoins = 0;

	void EyeWitnessPeer::initParameters(const vector<Peer<EyeWitnessMessage>*>& _peers, json parameters) {
		neighborhoodSize = parameters["neighborhoodSize"];
		// we need a pool of valid wallets from the whole network to use in
		// simulated transactions
	};

	EyeWitnessPeer::EyeWitnessPeer() {
		// get this peer's valid wallets from the pool?
	}

	EyeWitnessPeer::~EyeWitnessPeer()
	{
	}

	EyeWitnessPeer::EyeWitnessPeer(const EyeWitnessPeer &rhs) : Peer<EyeWitnessMessage>(rhs)
	{
	}

	EyeWitnessPeer::EyeWitnessPeer(long id) : Peer(id)
	{
	}

	void EyeWitnessPeer::performComputation()
	{
		while (!inStreamEmpty())
		{
			Packet<EyeWitnessMessage> p = popInStream();
			EyeWitnessMessage message = p.getMessage();
			int seqNum = message.sequenceNum;
			std::unordered_map<int, StateChangeRequest>::iterator s;
			if ((s = localRequests.find(seqNum)) != localRequests.end()) {
				s->second.addToConsensus(message);
				if (s->second.consensusSucceeded()) {
					// if (message.trans.receiver.storedBy == message.trans.sender.storedBy) {
						// local transaction; commit changes, update wallets
					// } else {
						// nonlocal transaction; start state change request in superRequests
					// }
				} else {
					while (!s->second.outboxEmpty()) {
						// choose message recipients and relay
						// TODO: need way of determining this peer's neighborhood for the purposes of this transaction
					}
				}
			} else if ((s = superRequests.find(seqNum)) != superRequests.end()) {
				s->second.addToConsensus(message);
				if (s->second.consensusSucceeded()) {
					// update internal coin snapshots for transaction; if this
					// is in the receiver neighborhood, add new wallet and then
					// conduct internal transaction
				}else {
					while (!s->second.outboxEmpty()) {
						// choose message recipients and relay
					}
				}
			} else {
				// if this is a valid pre-prepare message or announcement
				// message, create a StateChangeRequest and start using it.
			}
		}
		if (oneInXChance(10)) {
			initiateTransaction();
		}
	}

	void EyeWitnessPeer::broadcastTo(EyeWitnessMessage m, Neighborhood n) {
		for (const long& i : n.memberIDs) {
			unicastTo(m, i);
		}
	}

	void EyeWitnessPeer::initiateTransaction(bool withinNeighborhood = true) {
		// choose random wallet
		// choose random coin
		// choose destination... within neighborhood for now?
		// create StateChangeRequest
		WalletLocation& sender = heldWallets[randMod(heldWallets.size())];
		
	}

	Simulation<quantas::EyeWitnessMessage, quantas::EyeWitnessPeer> *generateSim()
	{

		Simulation<quantas::EyeWitnessMessage, quantas::EyeWitnessPeer> *sim = new Simulation<quantas::EyeWitnessMessage, quantas::EyeWitnessPeer>;
		return sim;
	}
}
