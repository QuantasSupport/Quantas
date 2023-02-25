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

	int PBFTRequest::numbersUsed = 0;

	void PBFTRequest::updateConsensus(EyeWitnessMessage m)
	{
		// TODO: each instance of this class only needs to deal with one transaction (but could still store received messages)
		// if (id() == 0 && status == "pre-prepare") {
		//     for (int i = 0; i < transactions.size(); i++) {
		//         bool skip = false;
		//         for (int j = 0; j < confirmedTrans.size(); j++) {
		//             if (transactions[i].trans == confirmedTrans[j].trans) {
		//                 skip = true;
		//                 break;
		//             }
		//         }
		//         if (!skip) {
		//             status = "prepare";
		//             EyeWitnessMessage message = transactions[i];
		//             message.messageType = "pre-prepare";
		//             message.Id = id();
		//             message.sequenceNum = sequenceNum;
		//             broadcast(message);
		//             if (receivedMessages.size() < sequenceNum + 1) {
		//                 receivedMessages.push_back(vector<EyeWitnessMessage>());
		//             }
		//             receivedMessages[sequenceNum].push_back(message);
		//             break;
		//         }
		//     }
		// } else if (status == "pre-prepare" && receivedMessages.size() >= sequenceNum + 1) {
		//     for (int i = 0; i < receivedMessages[sequenceNum].size(); i++) {
		//         EyeWitnessMessage message = receivedMessages[sequenceNum][i];
		//         if (message.messageType == "pre-prepare") {
		//             status = "prepare";
		//             EyeWitnessMessage newMsg = message;
		//             newMsg.messageType = "prepare";
		//             newMsg.Id = id();
		//             broadcast(newMsg);
		//             receivedMessages[sequenceNum].push_back(newMsg);
		//         }
		//     }
		// }

		// if (status == "prepare") {
		//     int count = 0;
		//     for (int i = 0; i < receivedMessages[sequenceNum].size(); i++) {
		//         EyeWitnessMessage message = receivedMessages[sequenceNum][i];
		//         if (message.messageType == "prepare") {
		//             count++;
		//         }
		//     }
		//     if (count > (neighbors().size() * 2 / 3)) {
		//         status = "commit";
		//         EyeWitnessMessage newMsg = receivedMessages[sequenceNum][0];
		//         newMsg.messageType = "commit";
		//         newMsg.Id = id();
		//         broadcast(newMsg);
		//         receivedMessages[sequenceNum].push_back(newMsg);
		//     }
		// }

		// if (status == "commit") {
		//     int count = 0;
		//     for (int i = 0; i < receivedMessages[sequenceNum].size(); i++) {
		//         EyeWitnessMessage message = receivedMessages[sequenceNum][i];
		//         if (message.messageType == "commit") {
		//             count++;
		//         }
		//     }
		//     if (count > (neighbors().size() * 2 / 3)) {
		//         status = "pre-prepare";
		//         confirmedTrans.push_back(receivedMessages[sequenceNum][0]);
		//         latency += getRound() - receivedMessages[sequenceNum][0].roundSubmitted;
		//         sequenceNum++;
		//         if (id() == 0) {
		//             submitTrans(currentTransaction);
		//         }
		//         checkContents();
		//     }
		// }
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
				s->second.updateConsensus(message);
				if (s->second.consensusSucceeded()) {
					// TODO: define == for neighborhoods? or simplify implementation of them
					// if (message.trans.receiver.storedBy == message.trans.sender.storedBy) {
						// local transaction; commit changes, update wallets
					// } else {
						// nonlocal transaction; 
					// }
				} else {
					while (!s->second.outboxEmpty()) {
						// choose message recipients and relay
						// TODO: need way of determining this peer's neighborhood for the purposes of this transaction
					}
				}
			} else if ((s = superRequests.find(seqNum)) != superRequests.end()) {
				s->second.updateConsensus(message);
				if (s->second.consensusSucceeded()) {
					// update internal coin snapshots for transaction; if this
					// is in the receiver neighborhood, add new wallet and then
					// conduct internal transaction
				}else {
					while (!s->second.outboxEmpty()) {
						// choose message recipients and relay
					}
				}
			}
		}
	}

	void EyeWitnessPeer::broadcastTo(EyeWitnessMessage m, Neighborhood n) {
		for (const long& i : n.memberIDs) {
			unicastTo(m, i);
		}
	}

	Simulation<quantas::EyeWitnessMessage, quantas::EyeWitnessPeer> *generateSim()
	{

		Simulation<quantas::EyeWitnessMessage, quantas::EyeWitnessPeer> *sim = new Simulation<quantas::EyeWitnessMessage, quantas::EyeWitnessPeer>;
		return sim;
	}
}
