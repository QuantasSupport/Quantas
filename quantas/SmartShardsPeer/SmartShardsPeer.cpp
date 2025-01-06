/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include "SmartShardsPeer.hpp"

namespace quantas {

	int SmartShardsPeer::currentTransaction = 1;
	mutex SmartShardsPeer::currentTransaction_mutex;
	int SmartShardsPeer::nextJoiningNode = 0;
	int SmartShardsPeer::numberOfShards = 0;
	int SmartShardsPeer::churnRate = 0;
	int SmartShardsPeer::maxLeaveDelay = 100;
	int SmartShardsPeer::ChurnOption = 0;
	double SmartShardsPeer::creationThreshold = 100.0;
	double SmartShardsPeer::removalThreshold = -1.0;

	template <typename Map>
	bool key_compare(Map const& lhs, Map const& rhs) {
		return lhs.size() == rhs.size()
			&& std::equal(lhs.begin(), lhs.end(), rhs.begin(),
				[](auto a, auto b) { return a.first == b.first; });
	}

	SmartShardsPeer::~SmartShardsPeer() {

	}

	SmartShardsPeer::SmartShardsPeer(const SmartShardsPeer& rhs) : Peer<SmartShardsMessage>(rhs) {

	}

	SmartShardsPeer::SmartShardsPeer(long id) : Peer(id) {

	}

	void SmartShardsPeer::performComputation() {
		if (alive) {
			if (true)
				checkInStrm();

			vector<int> originalShards; // used for when a node leaves/joins a shard it will mess with the iterators
			for (auto ip = shards.begin(); ip != shards.end(); ip++)
				originalShards.push_back(ip->first);

			for (int i = 0; i < originalShards.size(); i++)
				checkContents(originalShards[i]);

			if (leaving) {
				leaveDelay++;
				if (leaveDelay >= maxLeaveDelay) {
					//cout << "Node " << id() << " LEFT WITHOUT PERMISSION -----------------------" << endl;
					//cout << "Shards: ";
					for (auto ip = shards.begin(); ip != shards.end(); ip++) {
						//cout << ip->first << ", ";
					}
					//cout << endl;
					shards.clear();
					members.clear();
					status.clear();
					leaving = false;
					alive = false;
					clearMessages();
				}
			}
			if (joining) {
				joinDelay++;
			}
		}
	}

	void SmartShardsPeer::initParameters(const vector<Peer<SmartShardsMessage>*>& _peers, json parameters) {
		const vector<SmartShardsPeer*> peers = reinterpret_cast<vector<SmartShardsPeer*> const&>(_peers);
		// number of shards = s
		// number of shards a node is in L = 2 currently fixed at 2
		// number of intersections = intersections
		// total number of nodes n = intersections * s * (s - 1) / L
		// nodes in a shard = (s - 1) * intersections
		int s = numberOfShards = parameters["s"];
		int intersections = parameters["intersections"];
		vector<vector<int>> shardGrid(s);
		int nextNode = 0;
		if (false) {
			// Total shard overlap
			for (int i = 0; i < shardGrid.size(); i++) {
				for (int j = 0; shardGrid[i].size() < (s - 1) * intersections; j++) {
					for (int k = 0; k < intersections; k++) {
						shardGrid[i].push_back(nextNode);
						shardGrid[i + j + 1].push_back(nextNode++);
					}
				}
			}
		} else if (true) {
			// skip list \_(ツ)_/¯
			for (int i = 0; i < shardGrid.size(); i++) {
				int j = 1;
				while (i + (pow(2,j)) - 1 < shardGrid.size()) {
					for (int k = 0; k < intersections; k++) {
						shardGrid[i].push_back(nextNode);
						int shard2 = i + pow(2,j) - 1;
						shardGrid[shard2].push_back(nextNode++);
						// std::cout << "s1 " << i + 1 << " s2 " << (shard2 + 1) << std::endl;
					}
					j++;
				}
			}
		}
		
		// intersections * s * (s - 1) / 2; // only for total overlap
		nextJoiningNode = nextNode;
		std::cout << nextNode << endl;
		// std::cout << 5 / 0;

		for (int i = 0; i < shardGrid.size(); i++) {

			for (int j = 0; j < shardGrid[i].size(); j++) {
				peers[shardGrid[i][j]]->shards[i] = false;
				peers[shardGrid[i][j]]->status[i] = "pre-prepare";
				peers[shardGrid[i][j]]->workingTrans[i] = 0;
				peers[shardGrid[i][j]]->alive = true;
			}
			int leader = randMod(shardGrid[i].size());
			peers[shardGrid[i][leader]]->shards[i] = true;
			peers[shardGrid[i][leader]]->submitTrans(i);
		}
		for (int i = 0; i < shardGrid.size(); i++) {
			vector<SmartShardsMember> newMembers;
			for (int k = 0; k < shardGrid[i].size(); k++) {
				SmartShardsMember newMember;
				newMember.Id = shardGrid[i][k];
				if (peers[shardGrid[i][k]]->shards[i] == true) {
					newMember.leader = true;
				}
				for (auto ip = peers[shardGrid[i][k]]->shards.begin(); ip != peers[shardGrid[i][k]]->shards.end(); ip++) {
					newMember.shards.insert(ip->first);
				}
				newMembers.push_back(newMember);
			}
			for (int j = 0; j < shardGrid[i].size(); j++) {
				peers[shardGrid[i][j]]->members[i] = newMembers;
			}
		}

		if (parameters.contains("churnRate")) {
			churnRate = parameters["churnRate"];
		}
		if (parameters.contains("maxLeaveDelay")) {
			maxLeaveDelay = parameters["maxLeaveDelay"];
		}
		if (parameters.contains("ChurnOption")) {
			ChurnOption = parameters["ChurnOption"];
		}
		if (parameters.contains("creationThreshold")) {
        	creationThreshold = parameters["creationThreshold"];
		}
		if (parameters.contains("removalThreshold")) {
			removalThreshold = parameters["removalThreshold"];
		}
	}

	void SmartShardsPeer::endOfRound(const vector<Peer<SmartShardsMessage>*>& _peers) {
		const vector<SmartShardsPeer*> peers = reinterpret_cast<vector<SmartShardsPeer*> const&>(_peers);

		//for (int j = 0; j < nextJoiningNode; j++) {
		//	for (auto ip = peers[j]->shards.begin(); ip != peers[j]->shards.end(); ip++) {
		//		if (ip->second) {
		//			//cout << "size of shard " << ip->first << " " << peers[j]->members[ip->first].size() << endl;
		//		}
		//	}
		//}
		if (churnRate != 0) {
			// Joins
			// i < churnRate && LogWriter::instance()->getRound() < 50;
			for (int i = 0; i < churnRate; i++) {
				SmartShardsPeer* nextNode = peers[nextJoiningNode++];
				set<int> churnApprovals;
				int numberOfJoinRequests = 2;
				if (ChurnOption == 1 || ChurnOption == 3) {
					numberOfJoinRequests = 1; // second join will be routed
				}
				while (churnApprovals.size() < numberOfJoinRequests && churnApprovals.size() < numberOfShards) {
					churnApprovals.insert(randMod(numberOfShards));
					nextNode->joining = true;
					nextNode->alive = true;
				}

				for (auto ip = churnApprovals.begin(); ip != churnApprovals.end(); ip++) {
					for (int j = 0; j < peers.size(); j++) {
						if (peers[j]->shards.find(*ip) != peers[j]->shards.end()) {
							if (peers[j]->shards[*ip]) { // send request directly to leader
								nextNode->addNeighbor(_peers[j]->id()); // add node as a connection
								SmartShardsMessage message;
								message.messageType = "joinRequest";
								message.Id = nextNode->id();
								message.shard = *ip;
								sendMessage(peers[j]->id(), message);
								break;
							}
						}
					}
				}
			}

			// Leaves
			vector<int> options(nextJoiningNode);
			for (int i = 0; i < nextJoiningNode; i++) options[i] = i;
			shuffle(options.begin(), options.end(), default_random_engine{});
			
			// i < churnRate && LogWriter::instance()->getRound() > 50;
			for (int i = 0; i < churnRate; i++) {
				if (options.size() == 0) {
					// ran out of nodes able to churn
					break;
				}
				SmartShardsPeer* leavingNode = peers[options.back()];
				options.pop_back();
				// nodes already churning can't leave
				if (leavingNode->joining || leavingNode->leaving || !leavingNode->alive) { // has to wait until previous action is finished and must be alive
					i--;
					continue;
				}
				// leaders can't leave
				bool leader = false;
				for (auto ip = leavingNode->shards.begin(); ip != leavingNode->shards.end(); ip++) {
					if (ip->second) {
						i--;
						leader = true;
						break;
					}
				}

				if (leader) continue;

				leavingNode->leaving = true;
				for (int j = 0; j < peers.size() && LogWriter::instance()->getRound() < 50; j++) {
					for (auto ip = leavingNode->shards.begin(); ip != leavingNode->shards.end(); ip++) {
						if (peers[j]->shards.find(ip->first) != peers[j]->shards.end()) {
							if (peers[j]->shards[ip->first]) { // send request directly to leaders
								SmartShardsMessage message;
								message.messageType = "leaveRequest";
								message.Id = leavingNode->id();
								message.shard = ip->first;
								sendMessage(peers[j]->id(), message);
								break;
							}
						}
					}
				}
			}
		}


		// Check rough network intersections for shard creation or removal
		// total number of nodes n = intersections * s * (s - 1) / L
		double networkSize = std::count_if(peers.begin(), peers.end(),
										[](const auto& peer) { return peer->alive && !peer->leaving; });

		double intersections = networkSize * 2.0 / numberOfShards / (numberOfShards - 1.0);
		if (intersections > creationThreshold) {
			createShard(++numberOfShards - 1, peers);
		} else if (intersections < removalThreshold && numberOfShards > 1) {
			removeShard(--numberOfShards, peers);
		}

		double length = 0;
		double count = 0;
		for (int i = 0; i < peers.size(); i++) {
			count += peers[i]->confirmedTrans.size(); 
			for (int j = 0; j < peers[i]->confirmedTrans.size(); j++) {
				if (peers[i]->confirmedTrans[j].crossShardTransaction) {
					length += 0.5;
				} else {
					length += 1.0;
				}
			}
		}
		// LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()][LogWriter::instance()->getRound()]["Throughput"].push_back(length);
		// if (length > 0) {
		// 	LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["Latency"].push_back(totalLatency / length);
		// }
		if (lastRound() && true) {
			// doubles for division
			double length = 0;
			double totalMessages = 0;
			double totalLatency = 0;
			double joinTime = 0;
			double nodesJoined = 0;
			double leaveTime = 0;
			double nodesLeftSuccessfully = 0;
			double count = 0;
			for (int i = 0; i < peers.size(); i++) {
				count += peers[i]->confirmedTrans.size(); 
				for (int j = 0; j < peers[i]->confirmedTrans.size(); j++) {
					if (peers[i]->confirmedTrans[j].crossShardTransaction) {
						length += 0.5;
					} else {
						length += 1.0;
					}
				}
				totalMessages += peers[i]->messagesSent;
				totalLatency += peers[i]->latency;
				joinTime += peers[i]->timeToJoin;
				if (peers[i]->timeToJoin != 0) {
					nodesJoined++;
				}
				leaveTime += peers[i]->timeToLeave;
				if (peers[i]->timeToLeave != 0) {
					nodesLeftSuccessfully++;
				}
			}
			
			LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["NumberOfMessages"].push_back(totalMessages);
			LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["Throughput"].push_back(length);
			if (nodesJoined != 0) {
				LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["joinWaiting"].push_back(joinTime / nodesJoined);
			}
			if (nodesLeftSuccessfully != 0) {
				LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["leaveWaiting"].push_back(leaveTime / nodesLeftSuccessfully);
			}
			if (length > 0) {
				LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["Latency"].push_back(totalLatency / length);
			}
		}
	}

	void SmartShardsPeer::createShard(int shardId, const std::vector<SmartShardsPeer*>& peers) {
		// Logic for creating a new shard
		std::vector<SmartShardsPeer*> newShardMembers;
		std::vector<int> shardsToLeave;
		std::vector<int> membersSoFar(shardId, 0);
		// Select members for the new shard.... TODO
		// Members need to request to leave one of the 
		// shards they are currently in. Select correct 
		// non leader peers from the set to be the ones
		// moved such that there is still overlap between shards
		// Currently a hopeful approach with randomness
		int membersPerShard = std::round(creationThreshold / 2.0);
		if (numberOfShards == 2) {
			membersPerShard = 4;
		}
		for (const auto& peer : peers) {
			if (peer->shards.size() > 1) {
				for (auto currentShard : peer->shards) {
					if (peer->alive && !peer->leaving && !currentShard.second && membersSoFar[currentShard.first] < membersPerShard) {
						membersSoFar[currentShard.first]++; // planning to add member from this shard
						newShardMembers.push_back(peer);
						shardsToLeave.push_back(currentShard.first);
						break;
					}
				}
			}
		}
		// Can't create shard as there aren't enough members
		if (newShardMembers.size() < 4) {
			numberOfShards--;
			return;
		}

		LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()][LogWriter::instance()->getRound()]["CreatedShard"].push_back(LogWriter::instance()->getRound());

		int i = 0;
		for (const auto& member : newShardMembers) {
			// Leave shard to join the new one
			// TODO check to see if I need a different kind of leaving check for moving shards
			SmartShardsMessage message;
			message.messageType = "leaveRequest";
			message.Id = member->id();
			message.shard = shardsToLeave[i];
			// send request directly to leaders
			for (int j = 0; j < peers.size(); j++) {
				if (peers[j]->shards.find(shardsToLeave[i]) != peers[j]->shards.end()) {
						if (peers[j]->shards[shardsToLeave[i]]) {
							sendMessage(peers[j]->id(), message);
							break;
						}
				}
			}
			i++;
		}

		// nodes don't need to request to join the new shard
		// so they just join it
		std::vector<SmartShardsMember> newMembers;
		int leader = randMod(newShardMembers.size());
		i = 0;
		for (const auto& member : newShardMembers) {
			member->shards[shardId] = false;
			member->status[shardId] = "pre-prepare";
			member->workingTrans[shardId] = 0;

			SmartShardsMember newMember;
			newMember.Id = member->id();
			if (i == leader) {
				newMember.leader = true;
				member->shards[shardId] = true;
			}
			for (const auto& shard : member->shards) {
				newMember.shards.insert(shard.first);
			}
			newMembers.push_back(newMember);
			i++;
		}

		for (const auto& member : newShardMembers) {
			member->members[shardId] = newMembers;
		}

		newShardMembers[leader]->submitTrans(shardId);
	}

	void SmartShardsPeer::removeShard(int shardId, const std::vector<SmartShardsPeer*>& peers) {
		// Logic for removing an existing shard
		std::vector<SmartShardsPeer*> shardMembers;
		for (const auto& peer : peers) {
			if (peer->shards.count(shardId)) {
				if (peer->shards[shardId]) {
					// have leader do stuff with pending transactions Todo
				}
				shardMembers.push_back(peer);
				peer->shards.erase(shardId);
				peer->members.erase(shardId);
				peer->status.erase(shardId);
				peer->workingTrans.erase(shardId);
			}
		}

		for (const auto& member : shardMembers) {
			if (member->alive && !member->leaving) {
				int shard = randMod(numberOfShards);
				while (shard == shardId) {
					shard = randMod(numberOfShards);
				}
				SmartShardsMessage message;
				message.messageType = "joinRequest";
				message.Id = member->id();
				message.shard = shard;
				sendMessage(member->id(), message);
			}
		}
	}

	void SmartShardsPeer::checkInStrm() {
		while (!inStreamEmpty()) {
			SmartShardsMessage newMsg = popInStream().getMessage();

			if (newMsg.messageType == "trans") {
				transactions.push_back(newMsg);
			}
			else if (newMsg.messageType == "joinApproved") {

				status[newMsg.shard] = "pre-prepare";
				shards[newMsg.shard] = false;
				members[newMsg.shard] = newMsg.members;
				SmartShardsMember self;
				self.Id = id();
				for (auto ip2 = shards.begin(); ip2 != shards.end(); ip2++) {
					self.shards.insert(ip2->first);
				}
				for (int i = 0; i < members[newMsg.shard].size(); i++) {
					addNeighbor(members[newMsg.shard][i].Id); // add node as a connection
				}
				//cout << "Node " << id() << " successfully joined shard " << newMsg.shard << endl;
				if (shards.size() > 1) {
					//cout << "Node " << id() << " successfully joined both shards" << endl;
					joining = false;
					timeToJoin += joinDelay;
					joinDelay = 0;
					SmartShardsMessage message;
					message.messageType = "updateMember";
					message.Id = id();
					message.members.push_back(self);
					for (auto ip = shards.begin(); ip != shards.end(); ip++) {
						message.shard = ip->first;
						sendMessageShard(ip->first, message);
						updateMember(ip->first, self);
					}
				}
				workingTrans[newMsg.shard] = newMsg.trans;
			}
			else if (newMsg.messageType == "leaveRequest") {
				churnRequests[newMsg.shard].push_back(std::make_pair("leave", newMsg.Id));
			}
			else if (newMsg.messageType == "joinRequest") {
				int shard = newMsg.shard;
				if (shards[newMsg.shard]) {
					churnRequests[shard].push_back(std::make_pair("join", newMsg.Id));
					// need to find other shards for node
					if (ChurnOption == 1) {
						bool foundShards = false;
						for (int j = 0; j < churnRequests[shard].size(); j++) {
							if (churnRequests[shard][j].first == "leave") {
								for (int k = 0; k < members[shard].size(); k++) {
									if (members[shard][k].Id == churnRequests[shard][j].second) {

										for (auto ip = members[shard][k].shards.begin(); ip != members[shard][k].shards.end(); ip++) {
											if (*ip != shard) {
												SmartShardsMessage joinRequestMessage;
												joinRequestMessage.messageType = "joinRequest2";
												joinRequestMessage.Id = newMsg.Id;
												joinRequestMessage.shard = *ip;
												//cout << "Node " << id() << " creating joinrequest2 fill hole to shard " << *ip << " for node " << newMsg.Id << " send to " << members[shard][j].Id << endl;
												sendMessage(churnRequests[shard][j].second, joinRequestMessage);
												foundShards = true;
												break;
											}
										}
									}
									if (foundShards) {
										break;
									}
								}
							}
							if (foundShards) {
								break;
							}
						}



						// have node join other random shard
						if (!foundShards) {
							int otherShard;
							do {
								otherShard = randMod(numberOfShards);
							} while (otherShard == shard);
							// if the leader is in the other shard
							if (shards.find(otherShard) != shards.end()) {
								if (shards[otherShard]) {
									churnRequests[otherShard].push_back(std::make_pair("join2", newMsg.Id));
								}
								else {
									for (int j = 0; j < members[otherShard].size(); j++) {
										if (members[otherShard][j].leader == true) {
											SmartShardsMessage joinRequestMessage;
											//cout << "Node " << id() << " creating joinrequest2 in both to shard " << otherShard << " for node " << newMsg.Id << " send to " << members[otherShard][j].Id << endl;
											joinRequestMessage.messageType = "joinRequest2";
											joinRequestMessage.Id = newMsg.Id;
											joinRequestMessage.shard = otherShard;
											sendMessage(members[otherShard][j].Id, joinRequestMessage);
											break;
										}
									}
								}
							}
							else {
								// route through shared nodes
								for (int j = 0; j < members[shard].size(); j++) {
									if (members[shard][j].shards.find(otherShard) != members[shard][j].shards.end()) {
										SmartShardsMessage joinRequestMessage;
										joinRequestMessage.messageType = "joinRequest2";
										joinRequestMessage.Id = newMsg.Id;
										joinRequestMessage.shard = otherShard;
										sendMessage(members[shard][j].Id, joinRequestMessage);
										//cout << "Node " << id() << " creating joinrequest2 to shard " << otherShard << " for node " << newMsg.Id << " send to " << members[shard][j].Id << endl;
										break;
									}
								}
							}
						}
					}
					else if (ChurnOption == 3) {
						map<int, int> potentialShardSizes;
						for (int j = 0; j < members[shard].size(); j++) {
							for (auto ip = members[shard][j].shards.begin(); ip != members[shard][j].shards.end(); ip++) {
								if (potentialShardSizes.find(*ip) == potentialShardSizes.end()) {
									potentialShardSizes[*ip] = 1;
								}
								else {
									potentialShardSizes[*ip]++;
								}
							}
						}
						for (int j = 0; j < churnRequests[shard].size(); j++) {
							if (churnRequests[shard][j].first == "leave") {
								for (int k = 0; k < members[shard].size(); k++) {
									for (int k = 0; k < members[shard].size(); k++) {
										if (members[shard][k].Id == churnRequests[shard][j].second) {
											for (auto ip = members[shard][k].shards.begin(); ip != members[shard][k].shards.end(); ip++) {
												if (*ip != shard) {
													potentialShardSizes[*ip]--;
												}
											}
										}
									}
								}
							}
						}
						int minMembers = INT_MAX;
						auto minPointer = potentialShardSizes.end();
						for (auto ip = potentialShardSizes.begin(); ip != potentialShardSizes.end(); ip++) {
							if (ip->second < minMembers) {
								minPointer = ip;
								minMembers = ip->second;
							}
						}
						SmartShardsMessage joinRequestMessage;
						joinRequestMessage.messageType = "joinRequest2";
						joinRequestMessage.Id = newMsg.Id;
						joinRequestMessage.shard = minPointer->first;
						minPointer->second++;
						int nodeIndex = -1;
						for (int j = 0; j < members[shard].size(); j++) {
							for (auto ip = members[shard][j].shards.begin(); ip != members[shard][j].shards.end(); ip++) {
								if (members[shard][j].shards.find(minPointer->first) != members[shard][j].shards.end()) {
									nodeIndex = j;
								}
							}
						}
						//cout << "Node " << id() << " creating joinrequest2 fill hole to shard " << minPointer->first << " for node " << newMsg.Id << " send to " << members[shard][nodeIndex].Id << endl;
						sendMessage(members[shard][nodeIndex].Id, joinRequestMessage);
					}
				}
				else {
					for (int i = 0; i < members[newMsg.shard].size(); i++) {
						if (members[newMsg.shard][i].leader == true) {
							//cout << "Node " << id() << " routing joinrequest to shard " << newMsg.shard << " for node " << newMsg.Id << " send to " << members[newMsg.shard][i].Id << endl;
							sendMessage(members[newMsg.shard][i].Id, newMsg);
						}
					}
				}
			}
			else if (newMsg.messageType == "joinRequest2") {
				if (shards.find(newMsg.shard) == shards.end()) {
					//cout << "BAD ROUTE |||||||||||||||||||||||" << endl;
					//cout << "Node " << id() << " got joinrequest2 to shard " << newMsg.shard << endl;
				}
				if (shards[newMsg.shard]) {
					//cout << "Node " << id() << " received routed joinrequest2 for " << newMsg.shard << " for node " << newMsg.Id << endl;
					churnRequests[newMsg.shard].push_back(std::make_pair("join2", newMsg.Id));
				}
				else {
					bool foundRoute = false;
					for (int i = 0; i < members[newMsg.shard].size(); i++) {
						if (members[newMsg.shard][i].leader == true) {
							//cout << "Node " << id() << " routing joinrequest2 to shard " << newMsg.shard << " for node " << newMsg.Id << " send to " << members[newMsg.shard][i].Id << endl;
							sendMessage(members[newMsg.shard][i].Id, newMsg);
							foundRoute = true;
						}
					}
					if (!foundRoute) {
						//cout << "BAD ROUTE |||||||||||||||||||||||" << endl;
						//cout << "Node " << id() << " got joinrequest2 to shard " << newMsg.shard << endl;
					}

				}
			}
			else if (newMsg.messageType == "updateMember") {
				updateMember(newMsg.shard, newMsg.members[0]);
			}
			else {
				receivedMessages[newMsg.trans].push_back(newMsg);
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
			if (ChurnOption != 2) {
				// handl all churn requests if churn is permitted
				// handle leaves first to figure out where to put joins nodes
				for (int i = 0; i < churnRequests[shard].size(); i++) {
					if (churnRequests[shard][i].first == "leave") {
						int leavingIndex = -1;
						for (int j = 0; j < members[shard].size(); j++) {
							if (members[shard][j].Id == churnRequests[shard][i].second) {
								leavingIndex = j;
								break;
							}
						}
						if (leavingIndex == -1) {
							break;
						}
						for (int j = 0; j < members[shard].size(); j++) {
							if (j == leavingIndex) continue;
							if (members[shard][leavingIndex].shards == members[shard][j].shards) {
								// found overlaping node so node can leave
								message.churningNodes.push_back(churnRequests[shard][i]);
								churnRequests[shard].erase(churnRequests[shard].begin() + i);
								i--;
								break;
							}
						}
					}
				}

				for (int i = 0; i < churnRequests[shard].size(); i++) {
					if (churnRequests[shard][i].first == "join") {

						message.churningNodes.push_back(churnRequests[shard][i]);
						churnRequests[shard].erase(churnRequests[shard].begin() + i);
						i--;

					}
					else if (churnRequests[shard][i].first == "join2") {
						message.churningNodes.push_back(churnRequests[shard][i]);
						churnRequests[shard].erase(churnRequests[shard].begin() + i);
						i--;
					}
				}
			}
			sendMessageShard(shard, message);
			receivedMessages[message.trans].push_back(message);
		}
		else if (status[shard] == "pre-prepare") {
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
								sendMessageShard(shard, newMsg);
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
				sendMessageShard(shard, newMsg);
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

				SmartShardsMessage CommitMessage = receivedMessages[workingTrans[shard]][0];

				for (int i = 0; i < CommitMessage.churningNodes.size(); i++) {
					if (CommitMessage.churningNodes[i].first == "leave") {
						if (CommitMessage.churningNodes[i].second == id()) {
							shards.erase(shard); // Removes node from shard
							members[CommitMessage.shard].clear();
							status.erase(shard);
							//cout << "Node " << id() << " successfully left shard " << shard << endl;
							if (shards.size() == 0) {
								leaving = false;
								timeToLeave += leaveDelay;
								leaveDelay = 0;
								alive = false;
								clearMessages();
								//cout << "Node " << id() << " successfully left" << endl;
							}
							return;
						}
						for (int j = 0; j < members[CommitMessage.shard].size(); j++) {
							if (members[CommitMessage.shard][j].Id == CommitMessage.churningNodes[i].second) {
								members[CommitMessage.shard].erase(members[CommitMessage.shard].begin() + j);
								break;
							}
						}

					}
					else if (CommitMessage.churningNodes[i].first == "join" || CommitMessage.churningNodes[i].first == "join2") {
						SmartShardsMember newMember;
						newMember.Id = CommitMessage.churningNodes[i].second;
						newMember.shards.insert(shard);
						members[CommitMessage.shard].push_back(newMember);
						addNeighbor(CommitMessage.churningNodes[i].second); // add node as a connection
					}
				}
				if (shards[shard] == true) {
					//cout << "Commits for shard " << shard << endl;
					for (int j = 0; j < receivedMessages[workingTrans[shard]].size(); j++) {
						SmartShardsMessage message = receivedMessages[workingTrans[shard]][j];
						if (message.messageType == "commit") {
							//cout << "Commit from " << message.Id << endl;
						}
					}
					CommitMessage.crossShardTransaction = randMod(numberOfShards) != 0;
					confirmedTrans.push_back(CommitMessage);
					latency += (getRound() - CommitMessage.roundSubmitted);
					for (int i = 0; i < CommitMessage.churningNodes.size(); i++) {
						if (CommitMessage.churningNodes[i].first == "join" || CommitMessage.churningNodes[i].first == "join2") {
							CommitMessage.messageType = "joinApproved";
							CommitMessage.members = members[shard];
							sendMessage(CommitMessage.churningNodes[i].second, CommitMessage);
						}
					}
					submitTrans(shard);
				}
				checkContents(shard);
			}
		}
	}

	void SmartShardsPeer::submitTrans(int shard) {
		const lock_guard<mutex> lock(currentTransaction_mutex);
		SmartShardsMessage message;
		message.messageType = "trans";
		message.trans = currentTransaction;
		message.Id = id();
		message.roundSubmitted = getRound();
		message.sequenceNum = sequenceNum;
		message.shard = shard;
		sendMessageShard(shard, message);
		transactions.push_back(message);
		workingTrans[shard] = currentTransaction;
		currentTransaction++;
	}

	void SmartShardsPeer::sendMessageShard(int shard, SmartShardsMessage message) {
		for (int i = 0; i < members[shard].size(); i++) {
			if (members[shard][i].Id != id())
				sendMessage(members[shard][i].Id, message);
		}
	}

	void SmartShardsPeer::sendMessage(int node, SmartShardsMessage message) {
		Packet<SmartShardsMessage> newMessage(getRound(), node, id());
		newMessage.setMessage(message);
		pushToOutSteam(newMessage);
		messagesSent++;
	}

	void SmartShardsPeer::updateMember(int shard, SmartShardsMember member) {
		bool found = false;
		for (int i = 0; i < members[shard].size(); i++) {
			if (members[shard][i].Id == member.Id) {
				members[shard][i] = member;
				found = true;
				break;
			}
		}
		if (!found) {
			members[shard].push_back(member);
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
	
	Simulation<quantas::SmartShardsMessage, quantas::SmartShardsPeer>* generateSim() {
        
        Simulation<quantas::SmartShardsMessage, quantas::SmartShardsPeer>* sim = new Simulation<quantas::SmartShardsMessage, quantas::SmartShardsPeer>;
        return sim;
    }
}

