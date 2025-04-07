/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "LinearChordPeer.hpp"

namespace quantas {

	static bool registerLinearChord = [](){
		registerPeerType("LinearChordPeer", 
			[](interfaceId pubId){ return new LinearChordPeer(pubId); });
		return true;
	}();

	int LinearChordPeer::currentTransaction = 1;
	int LinearChordPeer::numberOfNodes = 0;

	LinearChordPeer::~LinearChordPeer() {

	}

	LinearChordPeer::LinearChordPeer(const LinearChordPeer& rhs) : Peer<LinearChordMessage>(rhs) {
		
	}

	LinearChordPeer::LinearChordPeer(interfaceId id) : Peer(id) {
		
	}

	void LinearChordPeer::performComputation() {
		if (alive) {
			
			if (RoundManager::currentRound() == 1 && successor.size() == 0 && predecessor.size() == 0) {
				for (int i = 0; i < neighbors().size(); i++) {
					if (neighbors()[i] < publicId()) {
						LinearChordFinger newNode;
						newNode.Id = neighbors()[i];
						newNode.roundUpdated = RoundManager::currentRound();
						predecessor.insert(predecessor.begin(), newNode);
					}
					else if (neighbors()[i] > publicId()) {
						LinearChordFinger newNode;
						newNode.Id = neighbors()[i];
						newNode.roundUpdated = RoundManager::currentRound();
						successor.insert(successor.begin(), newNode);
					}
				}
			}

			if (RoundManager::currentRound() % 5 == 0) {
				heartBeat();
			}

			while (!inStreamEmpty()) {
				Packet packet = popInStream();
				interfaceId source = packet.sourceId();
				LinearChordMessage message = packet.getMessage();
				interfaceId reqId = message.reqId;
				if (message.action == "R") {
					if (publicId() == reqId) {
						requestsSatisfied++;
						latency += RoundManager::currentRound() - message.roundSubmitted;
						totalHops += message.hops;
					}
					else if (reqId > publicId()) {
						if (successor.size() > 0) {
							if (publicId() < reqId && (reqId < successor[0].Id || successor[0].Id < publicId())) {
								requestsSatisfied++;
								latency += RoundManager::currentRound() - message.roundSubmitted;
								totalHops += message.hops;
							}
							else {
								sendMessage(successor[0].Id, message);
							}
						}
					}
					else if (predecessor.size() > 0) {
						sendMessage(predecessor[0].Id, message);
					}
					else {
						sendMessage(publicId(), message);
					}
				}
				else if (reqId != publicId()) {
					bool added = false;
					int position;
					if (reqId > publicId()) {
						if (successor.size() == 0) {
							LinearChordFinger newNode;
							newNode.Id = reqId;
							newNode.roundUpdated = RoundManager::currentRound();
							successor.insert(successor.begin(), newNode);
							added = true;
						}
						else if (successor.size() == 1 && reqId != successor[0].Id) {
							LinearChordFinger newNode;
							newNode.Id = reqId;
							newNode.roundUpdated = RoundManager::currentRound();
							if (reqId < successor[0].Id) {
								successor.insert(successor.begin(), newNode);
							}
							else {
								successor.push_back(newNode);
							}
							added = true;
						}
						else {
							for (int i = 0; i < successor.size(); i++) {
								if (reqId < successor[i].Id) {
									LinearChordFinger newNode;
									newNode.Id = reqId;
									newNode.roundUpdated = RoundManager::currentRound();
									successor.insert(successor.begin() + i, newNode);
									added = true;
									position = i;
									break;
								}
								else if (reqId == successor[i].Id) {
									successor[i].roundUpdated = RoundManager::currentRound();
									break;
								}
							}

							if (!added) {
								sendMessage(successor[0].Id, message);
							}
						}
					}
					else if (reqId < publicId()) {
						if (predecessor.size() == 0) {
							LinearChordFinger newNode;
							newNode.Id = reqId;
							newNode.roundUpdated = RoundManager::currentRound();
							predecessor.insert(predecessor.begin(), newNode);
						}
						if (predecessor.size() == 1 && reqId != predecessor[0].Id) {
							LinearChordFinger newNode;
							newNode.Id = reqId;
							newNode.roundUpdated = RoundManager::currentRound();
							if (reqId > predecessor[0].Id) {
								predecessor.insert(predecessor.begin(), newNode);
							}
							else {
								predecessor.push_back(newNode);
							}

						}
						else {
							for (int i = 0; i < predecessor.size(); i++) {
								if (reqId > predecessor[i].Id) {
									LinearChordFinger newNode;
									newNode.Id = reqId;
									newNode.roundUpdated = RoundManager::currentRound();
									predecessor.insert(predecessor.begin() + i, newNode);
									added = true;
									position = i;
									break;
								}
								else if (reqId == predecessor[i].Id) {
									predecessor[i].roundUpdated = RoundManager::currentRound();
									break;
								}
							}
							if (!added) {
								sendMessage(predecessor[0].Id, message);
							}
						}
					}

					if (added) {
						LinearChordMessage response;
						response.action = "N";
						for (int i = 0; i < successor.size(); i++) {
							if (successor[i].Id != reqId) {
								sendMessage(successor[i].Id, message);
								response.reqId = successor[i].Id;
								sendMessage(reqId, response);
							}
							else {
								response.reqId = publicId();
								sendMessage(reqId, response);
							}
						}

						for (int i = 0; i < predecessor.size(); i++) {
							if (predecessor[i].Id != reqId) {
								sendMessage(predecessor[i].Id, message);
								response.reqId = predecessor[i].Id;
								sendMessage(reqId, response);
							}
							else {
								response.reqId = publicId();
								sendMessage(reqId, response);
							}
						}
						for (int i = redundantSize; i < successor.size(); i++) {
							successor.erase(successor.end() - 1);
						}
						for (int i = redundantSize; i < predecessor.size(); i++) {
							predecessor.erase(predecessor.end() - 1);
						}
					}
				}
			}
		}
	}

	void LinearChordPeer::endOfRound(vector<Peer*>& _peers) {
		const vector<LinearChordPeer*> peers = reinterpret_cast<vector<LinearChordPeer*> const&>(_peers);
		numberOfNodes = peers.size();
		peers[randMod(numberOfNodes)]->submitTrans(currentTransaction);
		double satisfied = 0;
		double hops = 0;
		for (int i = 0; i < peers.size(); i++) {
			satisfied += peers[i]->requestsSatisfied;
			hops += peers[i]->totalHops;
		}
		
		LogWriter::pushValue("averageHops", hops / satisfied);
	}

	void LinearChordPeer::heartBeat() {
		if (alive) {
			LinearChordMessage message;
			message.action = "N";
			message.reqId = publicId();
			for (int i = 0; i < successor.size(); i++) {
				if (successor[i].roundUpdated + 20 > RoundManager::currentRound()) {
					sendMessage(successor[i].Id, message);
				}
				else {
					successor.erase(successor.begin() + i);
					i--;
				}
			}
			for (int i = 0; i < predecessor.size(); i++) {
				if (predecessor[i].roundUpdated + 20 > RoundManager::currentRound()) {
					sendMessage(predecessor[i].Id, message);
				}
				else {
					predecessor.erase(predecessor.begin() + i);
					i--;
				}
			}
		}
	}

	void LinearChordPeer::sendMessage(interfaceId peer, LinearChordMessage message) {
		Packet newMessage(RoundManager::currentRound(), peer, publicId());
		message.hops++;
		newMessage.setMessage(message);
		pushToOutSteam(newMessage);
	}

	void LinearChordPeer::submitTrans(int tranID) {
		LinearChordMessage message;
		message.reqId = randMod(numberOfNodes);
		interfaceId reqId = message.reqId;
		message.action = "R";
		message.roundSubmitted = RoundManager::currentRound();
		if (publicId() == reqId) {
			requestsSatisfied++;
			totalHops += message.hops;
		}
		else if (reqId > publicId()) {
			if (successor.size() > 0) {
				if (publicId() < reqId && (reqId < successor[0].Id || successor[0].Id < publicId())) {
					requestsSatisfied++;
					totalHops += message.hops;
				}
				else {
					sendMessage(successor[0].Id, message);
				}
			}
		}
		else if (predecessor.size() > 0) {
			sendMessage(predecessor[0].Id, message);
		}
		else {
			sendMessage(publicId(), message);
		}
		currentTransaction++;
	}

}