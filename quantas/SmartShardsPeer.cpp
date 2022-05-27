/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "SmartShardsPeer.hpp"

namespace quantas {

	int SmartShardsPeer::currentTransaction = 1;

	SmartShardsPeer::~SmartShardsPeer() {

	}

	SmartShardsPeer::SmartShardsPeer(const SmartShardsPeer& rhs) : Peer<SmartShardsMessage>(rhs) {
		
	}

	SmartShardsPeer::SmartShardsPeer(long id) : Peer(id) {

	}

	void SmartShardsPeer::performComputation() {
		if (true)
			checkInStrm();

		for (auto ip = shards.begin(); ip != shards.end(); ip++)
			checkContents(ip->first);

	}

	void SmartShardsPeer::initParameters(const vector<Peer<SmartShardsMessage>*>& _peers, json parameters) {
		const vector<SmartShardsPeer*> peers = reinterpret_cast<vector<SmartShardsPeer*> const&>(_peers);
		// number of shards = s
			// number of shards a node is in k = 2 currently fixed at 2
			// number of intersections = intersections
			// total number of nodes n = intersections * s * (s - 1) / k
			// nodes in a shard = (s - 1) * intersections
			int s = parameters["s"];
			int intersections = parameters["intersections"];
			vector<vector<int>> shardGrid(s); 
			int nextNode = 0;
			for (int i = 0; i < shardGrid.size(); i++) {
				for (int j = 0; shardGrid[i].size() < (s - 1) * intersections; j++) {
					for (int k = 0; k < intersections; k++) {
						shardGrid[i].push_back(nextNode);
						shardGrid[i + j + 1].push_back(nextNode++);
					}
				}
			}
			for (int i = 0; i < shardGrid.size(); i++) {
				for (int j = 0; j < shardGrid[i].size(); j++) {
					peers[shardGrid[i][j]]->shards[i] = false;
					peers[shardGrid[i][j]]->status[i] = "pre-prepare";
					peers[shardGrid[i][j]]->workingTrans[i] = 0;
					for (int k = 0; k < shardGrid[i].size(); k++) {
						if (k != j) {
							peers[shardGrid[i][j]]->members[i].push_back(shardGrid[i][k]);
						}
					}
				}
				int leader = rand() % shardGrid[i].size();
				peers[shardGrid[i][leader]]->shards[i] = true;
				peers[shardGrid[i][leader]]->submitTrans(i);
			}
	}

	void SmartShardsPeer::endOfRound(const vector<Peer<SmartShardsMessage>*>& _peers) {
		const vector<SmartShardsPeer*> peers = reinterpret_cast<vector<SmartShardsPeer*> const&>(_peers);

		double length = 0;
		for (int i = 0; i < peers.size(); i++) {
			length += peers[i]->confirmedTrans.size();
		}
		LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["throughput"].push_back(length);
	}

	void SmartShardsPeer::checkInStrm() {
		while (!inStreamEmpty()) {
			Packet<SmartShardsMessage> newMsg = popInStream();
			
			if (newMsg.getMessage().messageType == "trans") {
				transactions.push_back(newMsg.getMessage());
			}
			else {
				receivedMessages[newMsg.getMessage().trans].push_back(newMsg.getMessage());
			}
		}
	}
	void SmartShardsPeer::checkContents(int shard) {
		if (shards[shard] && status[shard] == "pre-prepare") {
			status[shard] = "prepare";
			SmartShardsMessage message;
			for (int i = 0; i < transactions.size(); i++) {
				if (transactions[i].trans == workingTrans[shard]) {
					message = transactions[i];
					break;
				}
			}
			message.messageType = "pre-prepare";
			message.Id = id();
			message.sequenceNum = sequenceNum;
			sendMessage(shard, message);
			receivedMessages[message.trans].push_back(message);
		} else if (status[shard] == "pre-prepare") {
			for (auto ip = receivedMessages.begin(); ip != receivedMessages.end(); ip++) {
				if (ip->first > workingTrans[shard] && ip->second[0].shard == shard) {
					workingTrans[shard] = ip->first;
					if (receivedMessages.find(workingTrans[shard]) != receivedMessages.end()) {
						for (int i = 0; i < receivedMessages[workingTrans[shard]].size(); i++) {
							SmartShardsMessage message = receivedMessages[workingTrans[shard]][i];
							if (message.messageType == "pre-prepare") {
								status[shard] = "prepare";
								SmartShardsMessage newMsg = message;
								newMsg.messageType = "prepare";
								newMsg.Id = id();
								sendMessage(shard, newMsg);
								receivedMessages[workingTrans[shard]].push_back(newMsg);
								break;
							}
						}
					}
					break;
				}
			}
		}

		if (status[shard] == "prepare") {
			int count = 0;
			for (int i = 0; i < receivedMessages[workingTrans[shard]].size(); i++) {
				SmartShardsMessage message = receivedMessages[workingTrans[shard]][i];
				if (message.messageType == "prepare") {
					count++;
				}
			}
			if (count > (members[shard].size() * 2 / 3)) {
				status[shard] = "commit";
				SmartShardsMessage newMsg = receivedMessages[workingTrans[shard]][0];
				newMsg.messageType = "commit";
				newMsg.Id = id();
				sendMessage(shard, newMsg);
				receivedMessages[workingTrans[shard]].push_back(newMsg);
			}
		}

		if (status[shard] == "commit") {
			int count = 0;
			for (int i = 0; i < receivedMessages[workingTrans[shard]].size(); i++) {
				SmartShardsMessage message = receivedMessages[workingTrans[shard]][i];
				if (message.messageType == "commit") {
					count++;
				}
			}
			if (count > (members[shard].size() * 2 / 3)) {
				status[shard] = "pre-prepare";
				if (shards[shard] == true) {
					confirmedTrans.push_back(receivedMessages[workingTrans[shard]][0]);
					latency += getRound() - receivedMessages[workingTrans[shard]][0].roundSubmitted;
					submitTrans(shard);
				}
				checkContents(shard);
			}
		}
	}

	void SmartShardsPeer::submitTrans(int shard) {
		SmartShardsMessage message;
		message.messageType = "trans";
		message.trans = currentTransaction;
		message.Id = id();
		message.roundSubmitted = getRound();
		message.sequenceNum = sequenceNum;
		message.shard = shard;
		sendMessage(shard, message);
		transactions.push_back(message);
		workingTrans[shard] = currentTransaction;
		currentTransaction++;
	}

	void SmartShardsPeer::sendMessage(int shard, SmartShardsMessage message) {
		for (int i = 0; i < members[shard].size(); i++) {
			Packet<SmartShardsMessage> newMessage(getRound(), members[shard][i], id());
			newMessage.setMessage(message);
			pushToOutSteam(newMessage);
		}
	}

	ostream& SmartShardsPeer::printTo(ostream& out)const {
		Peer<SmartShardsMessage>::printTo(out);

		out << id() << endl;
		out << "counter:" << getRound() << endl;

		return out;
	}

	ostream& operator<< (ostream& out, const SmartShardsPeer& peer) {
		peer.printTo(out);
		return out;
	}
}