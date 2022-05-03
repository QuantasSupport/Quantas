//
//  LinearChordPeer.cpp
//  BlockGuard
//
//  Created by Joseph Oglio on 4/14/22.
//  Copyright © 2022 Kent State University. All rights reserved.
//

#include <iostream>
#include "LinearChordPeer.hpp"

namespace blockguard {

	int LinearChordPeer::currentTransaction = 1;
	int LinearChordPeer::numberOfNodes = 0;

	LinearChordPeer::~LinearChordPeer() {

	}

	LinearChordPeer::LinearChordPeer(const LinearChordPeer& rhs) : Peer<LinearChordMessage>(rhs) {
		_counter = rhs._counter;
	}

	LinearChordPeer::LinearChordPeer(long id) : Peer(id) {
		_counter = 0;
	}

	void LinearChordPeer::performComputation() {
		if (alive) {
			
			if (_counter == 0 && successor.size() == 0 && predecessor.size() == 0) {
				for (int i = 0; i < neighbors().size(); i++) {
					if (neighbors()[i] < id()) {
						LinearChordFinger newNode;
						newNode.Id = neighbors()[i];
						newNode.roundUpdated = _counter;
						predecessor.insert(predecessor.begin(), newNode);
					}
					else if (neighbors()[i] > id()) {
						LinearChordFinger newNode;
						newNode.Id = neighbors()[i];
						newNode.roundUpdated = _counter;
						successor.insert(successor.begin(), newNode);
					}
				}
			}

			if (_counter % 5 == 0) {
				heartBeat();
			}

			while (!inStreamEmpty()) {
				Packet<LinearChordMessage> packet = popInStream();
				long source = packet.sourceId();
				LinearChordMessage message = packet.getMessage();
				long reqId = message.reqId;
				if (message.action == "R") {
					if (id() == reqId) {
						requestsSatisfied++;
						latency += _counter - message.roundSubmitted;
						totalHops += message.hops;
					}
					else if (reqId > id()) {
						if (successor.size() > 0) {
							if (id() < reqId && (reqId < successor[0].Id || successor[0].Id < id())) {
								requestsSatisfied++;
								latency += _counter - message.roundSubmitted;
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
						sendMessage(id(), message);
					}
				}
				else if (reqId != id()) {
					bool added = false;
					int position;
					if (reqId > id()) {
						if (successor.size() == 0) {
							LinearChordFinger newNode;
							newNode.Id = reqId;
							newNode.roundUpdated = _counter;
							successor.insert(successor.begin(), newNode);
							added = true;
						}
						else if (successor.size() == 1 && reqId != successor[0].Id) {
							LinearChordFinger newNode;
							newNode.Id = reqId;
							newNode.roundUpdated = _counter;
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
									newNode.roundUpdated = _counter;
									successor.insert(successor.begin() + i, newNode);
									added = true;
									position = i;
									break;
								}
								else if (reqId == successor[i].Id) {
									successor[i].roundUpdated = _counter;
									break;
								}
							}

							if (!added) {
								sendMessage(successor[0].Id, message);
							}
						}
					}
					else if (reqId < id()) {
						if (predecessor.size() == 0) {
							LinearChordFinger newNode;
							newNode.Id = reqId;
							newNode.roundUpdated = _counter;
							predecessor.insert(predecessor.begin(), newNode);
						}
						if (predecessor.size() == 1 && reqId != predecessor[0].Id) {
							LinearChordFinger newNode;
							newNode.Id = reqId;
							newNode.roundUpdated = _counter;
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
									newNode.roundUpdated = _counter;
									predecessor.insert(predecessor.begin() + i, newNode);
									added = true;
									position = i;
									break;
								}
								else if (reqId == predecessor[i].Id) {
									predecessor[i].roundUpdated = _counter;
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
								response.reqId = id();
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
								response.reqId = id();
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
		_counter++;
	}

	void LinearChordPeer::endOfRound(const vector<Peer<LinearChordMessage>*>& _peers) {
		const vector<LinearChordPeer*> peers = reinterpret_cast<vector<LinearChordPeer*> const&>(_peers);
		numberOfNodes = peers.size();
		peers[rand() % numberOfNodes]->submitTrans(currentTransaction);
		double satisfied = 0;
		double hops = 0;
		for (int i = 0; i < peers.size(); i++) {
			satisfied += peers[i]->requestsSatisfied;
			hops += peers[i]->totalHops;
		}
		//LogWritter::instance()->data["tests"][LogWritter::instance()->getTest()]["throughput"].push_back(satisfied);
		LogWritter::instance()->data["tests"][LogWritter::instance()->getTest()]["averageHops"].push_back(hops / satisfied);
	}

	void LinearChordPeer::heartBeat() {
		if (alive) {
			LinearChordMessage message;
			message.action = "N";
			message.reqId = id();
			for (int i = 0; i < successor.size(); i++) {
				if (successor[i].roundUpdated + 20 > _counter) {
					sendMessage(successor[i].Id, message);
				}
				else {
					successor.erase(successor.begin() + i);
					i--;
				}
			}
			for (int i = 0; i < predecessor.size(); i++) {
				if (predecessor[i].roundUpdated + 20 > _counter) {
					sendMessage(predecessor[i].Id, message);
				}
				else {
					predecessor.erase(predecessor.begin() + i);
					i--;
				}
			}
		}
	}

	void LinearChordPeer::sendMessage(long peer, LinearChordMessage message) {
		Packet<LinearChordMessage> newMessage(_counter, peer, id());
		message.hops++;
		newMessage.setMessage(message);
		pushToOutSteam(newMessage);
	}

	void LinearChordPeer::submitTrans(int tranID) {
		LinearChordMessage message;
		message.reqId = rand() % numberOfNodes;
		long reqId = message.reqId;
		message.action = "R";
		message.roundSubmitted = _counter;
		if (id() == reqId) {
			requestsSatisfied++;
			totalHops += message.hops;
		}
		else if (reqId > id()) {
			if (successor.size() > 0) {
				if (id() < reqId && (reqId < successor[0].Id || successor[0].Id < id())) {
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
			sendMessage(id(), message);
		}
		currentTransaction++;
	}

	std::ostream& LinearChordPeer::printTo(std::ostream& out)const {
		Peer<LinearChordMessage>::printTo(out);

		out << id() << std::endl;
		out << "counter:" << _counter << std::endl;

		return out;
	}

	std::ostream& operator<< (std::ostream& out, const LinearChordPeer& peer) {
		peer.printTo(out);
		return out;
	}

}