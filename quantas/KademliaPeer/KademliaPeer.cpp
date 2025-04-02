/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <cmath>
#include "KademliaPeer.hpp"

namespace quantas {

	static bool registerKademlia = [](){
		registerPeerType("KademliaPeer", 
			[](interfaceId pubId){ return new KademliaPeer(pubId); });
		return true;
	}();

	int KademliaPeer::currentTransaction = 1;

	KademliaPeer::~KademliaPeer() {

	}

	KademliaPeer::KademliaPeer(const KademliaPeer& rhs) : Peer<KademliaMessage>(rhs) {
		
	}

	KademliaPeer::KademliaPeer(interfaceId id) : Peer(id) {
		
	}

	void KademliaPeer::performComputation() {
		if (alive) {

			if (RoundManager::currentRound() == 1 && fingers.size() == 0) {
				binaryIdSize = std::ceil(std::log2(neighbors().size()));
				binaryId = getBinaryId(publicId());
				vector<vector<KademliaFinger>> groupedNeighbors(binaryIdSize);
				for (int i = 0; i < neighbors().size(); i++) {
					interfaceId id = neighbors()[i];
					string binId = getBinaryId(id);
					int group = -1;
					for (int j = 0; j < binId.size(); j++) {
						if (binId[j] != binaryId[j]) {
							KademliaFinger newNode;
							newNode.Id = id;
							newNode.group = j;
							newNode.binId = binId;
							groupedNeighbors[j].push_back(newNode);
							break;
						}
					}
				}
				for (int j = 0; j < groupedNeighbors.size(); j++) {
					if (groupedNeighbors[j].size() > 0) {
						int index = randMod(groupedNeighbors[j].size());
						fingers.insert(fingers.begin(), groupedNeighbors[j][index]);
					}
				}
			}

			while (!inStreamEmpty()) {
				Packet packet = popInStream();
				interfaceId source = packet.sourceId();
				KademliaMessage message = packet.getMessage();
				if (message.action == "R") {
					if (publicId() == message.reqId) {
						requestsSatisfied++;
						latency += RoundManager::currentRound() - message.roundSubmitted;
						totalHops += message.hops;
					}
					else {
						sendMessage(findRoute(message.binId), message);
					}
				}
			}
		}
	}

	void KademliaPeer::endOfRound(const vector<Peer*>& _peers) {
		const vector<KademliaPeer*> peers = reinterpret_cast<vector<KademliaPeer*> const&>(_peers);
		peers[randMod(neighbors().size()) + 1]->submitTrans(currentTransaction);
		double satisfied = 0;
		double hops = 0;
		for (int i = 0; i < peers.size(); i++) {
			satisfied += peers[i]->requestsSatisfied;
			hops += peers[i]->totalHops;
		}
		LogWriter::pushValue("averageHops", hops / satisfied);
		
	}

	string KademliaPeer::getBinaryId(interfaceId id) {
		string binId;
		for (int i = binaryIdSize - 1; i >= 0; i--) {
			if (static_cast<int>(id) / static_cast<int>(std::pow(2, i)) > 0) {
				id -= static_cast<int>(std::pow(2, i));
				binId += '1';
			}
			else {
				binId += '0';
			}
		}
		return binId;
	}

	void KademliaPeer::sendMessage(interfaceId peer, KademliaMessage message) {
		Packet newMessage(RoundManager::currentRound(), peer, publicId());
		message.hops++;
		newMessage.setMessage(message);
		pushToOutSteam(newMessage);
	}

	void KademliaPeer::submitTrans(int tranID) {
		KademliaMessage message;
		message.reqId = randMod(neighbors().size() + 1);
		string binId = getBinaryId(message.reqId);
		message.binId = binId;
		message.action = "R";
		message.roundSubmitted = RoundManager::currentRound();
		if (publicId() == message.reqId) {
			requestsSatisfied++;
			totalHops += message.hops;
		}
		else {
			sendMessage(findRoute(message.binId), message);
		}
		currentTransaction++;
	}

	interfaceId KademliaPeer::findRoute(string binId) {
		int group = -1;
		for (int j = 0; j < binId.size(); j++) {
			if (binId[j] != binaryId[j]) {
				group = j;
				break;
			}
		}
		for (int j = 0; j < fingers.size(); j++) {
			if (group == fingers[j].group) {
				return fingers[j].Id;
			}
		}
		// route was not found
		return -1;
	}
	
}
