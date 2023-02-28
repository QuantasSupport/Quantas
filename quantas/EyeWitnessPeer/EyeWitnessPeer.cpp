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

	template <typename ConsensusRequest>
	int EyeWitnessPeer<ConsensusRequest>::issuedCoins = 0;

	// divides the peers into neighborhood and gives each neighborhood a set of wallets.
	// postcondition: neighborhoodSize is set (from the simulation input
	// parameters); neighborhoodCount is set; walletsForNeighborhoods contains a
	// vector of LocalWallets for each neighborhood; peerIDToNeighborhoodIndex
	// maps a peer's id to the index of the neighborhood it's in
	template <typename ConsensusRequest>
	void EyeWitnessPeer<ConsensusRequest>::initParameters(const vector<Peer<EyeWitnessMessage>*>& _peers, json parameters) {
		// divide peers into neighborhoods: if the neighborhood size is 5, then
		// the first 5 peers are placed in a neighborhood, then the next 5
		// peers, and so on. if the total number of peers is not a multiple of
		// 5, an "extra" neighborhood stores the spares. then, each neighborhood
		// gets a set of wallets to take care of.

		neighborhoodSize = parameters["neighborhoodSize"];
		const int walletsPerNeighborhood = parameters["walletsPerNeighborhood"];
		neighborhoodCount = ceil(static_cast<float>(_peers.size()) / neighborhoodSize);
		vector<WalletLocation> all;
		for (int i=0; i<neighborhoodCount; ++i) {
			Neighborhood newNeighborhood;
			newNeighborhood.leader = i*neighborhoodSize;
			for (int n=0; n<neighborhoodSize&&i*neighborhoodSize+n<_peers.size();++n) {
				const long peerID = i*neighborhoodSize+n;
				newNeighborhood.memberIDs.insert(peerID);
				peerIDToNeighborhoodIndex[peerID] = i;
			}
			for (int j=0; j<walletsPerNeighborhood; ++j) {
				LocalWallet w = {all.size(), newNeighborhood, {}, {}};
				walletsForNeighborhoods[i].push_back(w);
				all.push_back({w.address, newNeighborhood});
			}
		}

		const int coinsPerWallet = 5;
		for (auto& wallets: walletsForNeighborhoods) {
			for (auto& wallet: wallets) {
				for (int i=0; i<coinsPerWallet; ++i) {
					// a history could be invented here
					Coin c = {issuedCoins++, {}};
					wallet.coins.push_back(c);
				}
			}
		}
	}

	template<typename ConsensusRequest>
	EyeWitnessPeer<ConsensusRequest>::EyeWitnessPeer() {
		neighborhoodID = peerIDToNeighborhoodIndex[id()];
		heldWallets = walletsForNeighborhoods[neighborhoodID];
	}

	template<typename ConsensusRequest>
	void EyeWitnessPeer<ConsensusRequest>::performComputation() {
		while (!inStreamEmpty())
		{
			Packet<EyeWitnessMessage> p = popInStream();
			EyeWitnessMessage message = p.getMessage();
			int seqNum = message.sequenceNum;
			typename std::unordered_map<int, ConsensusRequest>::iterator s;
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
				// message, create a ConsensusRequest and start using it.
			}
		}
		if (oneInXChance(10)) {
			initiateTransaction();
		}
	}

	template<typename ConsensusRequest>
	void EyeWitnessPeer<ConsensusRequest>::broadcastTo(EyeWitnessMessage m, Neighborhood n) {
		for (const long& i : n.memberIDs) {
			unicastTo(m, i);
		}
	}

	template<typename ConsensusRequest>
	void EyeWitnessPeer<ConsensusRequest>::initiateTransaction(bool withinNeighborhood) {
		LocalWallet sender = heldWallets[randMod(heldWallets.size())];
		Coin& c = sender.coins[randMod(sender.coins.size())];
		LocalWallet receiver = sender;
		// obtain a receiver that is not the sender
		do {
			if (withinNeighborhood) {
				receiver = heldWallets[randMod(heldWallets.size())];
			} else {
				// TODO: create cross-neighborhood transaction
			}
		} while (receiver.address == sender.address);
		OngoingTransaction t = {sender, receiver, c};
		++previousSequenceNumber;
		ConsensusRequest request = PBFTRequest(t, neighborhoodSize, previousSequenceNumber);
		localRequests[previousSequenceNumber] = request;
	}

	Simulation<quantas::EyeWitnessMessage, quantas::EyeWitnessPeer<PBFTRequest>> *generateSim()
	{

		Simulation<quantas::EyeWitnessMessage, quantas::EyeWitnessPeer<PBFTRequest>> *sim =
			new Simulation<quantas::EyeWitnessMessage, quantas::EyeWitnessPeer<PBFTRequest>>;
		return sim;
	}
}
