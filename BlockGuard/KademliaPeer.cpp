//
//  KademliaPeer.cpp
//  BlockGuard
//
//  Created by Joseph Oglio on 4/14/22.
//  Copyright © 2022 Kent State University. All rights reserved.
//

#include <iostream>
#include "KademliaPeer.hpp"

namespace blockguard {

	int KademliaPeer::currentTransaction = 1;

	KademliaPeer::~KademliaPeer() {

	}

	KademliaPeer::KademliaPeer(const KademliaPeer& rhs) : Peer<KademliaMessage>(rhs) {
		_counter = rhs._counter;
	}

	KademliaPeer::KademliaPeer(long id) : Peer(id) {
		_counter = 0;
	}

	void KademliaPeer::performComputation() {
		if (alive) {

			if (_counter == 0 && fingers.size() == 0) {
				binaryId = getBinaryId(id());
				vector<vector<KademliaFinger>> groupedNeighbors(binaryIdSize);
				for (int i = 0; i < neighbors().size(); i++) {
					long id = neighbors()[i];
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
						int index = rand() % groupedNeighbors[j].size();
						fingers.insert(fingers.begin(), groupedNeighbors[j][index]);
					}
				}
			}

			while (!inStreamEmpty()) {
				Packet<KademliaMessage> packet = popInStream();
				long source = packet.sourceId();
				KademliaMessage message = packet.getMessage();
				if (message.action == "R") {
					if (id() == message.reqId) {
						requestsSatisfied++;
						latency += _counter - message.roundSubmitted;
						totalHops += message.hops;
					}
					else {
						sendMessage(findRoute(message.binId), message);
					}
				}
			}
		}
		_counter++;
	}

	void KademliaPeer::endOfRound(const vector<Peer<KademliaMessage>*>& _peers) {
		const vector<KademliaPeer*> peers = reinterpret_cast<vector<KademliaPeer*> const&>(_peers);
		peers[rand() % neighbors().size() + 1]->submitTrans(currentTransaction);
		if (_counter == 1000) {
			double satisfied = 0;
			double hops = 0;
			for (int i = 0; i < peers.size(); i++) {
				satisfied += peers[i]->requestsSatisfied;
				hops += peers[i]->totalHops;
			}
			//LogWritter::instance()->data["tests"][LogWritter::instance()->getTest()]["throughput"].push_back(satisfied);
			LogWritter::instance()->data["tests"][LogWritter::instance()->getTest()]["averageHops"].push_back(hops / satisfied);
		}
	}

	string KademliaPeer::getBinaryId(long id) {
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

	void KademliaPeer::sendMessage(long peer, KademliaMessage message) {
		Packet<KademliaMessage> newMessage(_counter, peer, id());
		message.hops++;
		newMessage.setMessage(message);
		pushToOutSteam(newMessage);
	}

	void KademliaPeer::submitTrans(int tranID) {
		KademliaMessage message;
		message.reqId = rand() % (neighbors().size() + 1);
		string binId = getBinaryId(message.reqId);
		message.binId = binId;
		message.action = "R";
		message.roundSubmitted = _counter;
		if (id() == message.reqId) {
			requestsSatisfied++;
			totalHops += message.hops;
		}
		else {
			sendMessage(findRoute(message.binId), message);
		}
		currentTransaction++;
	}

	long KademliaPeer::findRoute(string binId) {
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
	}

	std::ostream& KademliaPeer::printTo(std::ostream& out)const {
		Peer<KademliaMessage>::printTo(out);

		out << id() << std::endl;
		out << "counter:" << _counter << std::endl;

		return out;
	}

	std::ostream& operator<< (std::ostream& out, const KademliaPeer& peer) {
		peer.printTo(out);
		return out;
	}

}