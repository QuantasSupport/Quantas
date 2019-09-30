//
// Smart Shard class, wraps markPbft into virtual peers that are part of multiple consensus instances
// Mark Gardner
// 9/6/2019
//
#ifndef SMART_SHARD_HPP
#define SMART_SHARD_HPP

#include <vector>
#include <set>
#include "MarkPBFT_peer.hpp"
#include "Common/ByzantineNetwork.hpp"

class SmartShard {
public:
	~SmartShard() {
		for (auto e : _system)
			delete e;
	}
		SmartShard(const int& shards, std::ostream& out, int delay, int peerspershard = -1) {
			_out = &out;
			_shards = shards;
			if (peerspershard == -1)
				_peerspershard = shards - 1;
			else
				_peerspershard = peerspershard;

			// Initalize Quorums
			for (int i = 0; i < _shards; ++i) {
				_system.push_back(new ByzantineNetwork<markPBFT_message, markPBFT_peer>());
				_system[i]->setLog(*_out);
				_system[i]->setToRandom();
				_system[i]->setMaxDelay(delay);
				_system[i]->initNetwork(_peerspershard);
				(*_system[i])[rand() % _peerspershard]->setPrimary(true);
				for (int j = 0; j < _peerspershard; ++j)
					(*_system[i])[j]->setShard(i);

			}

			// Assign to virtual peer
			int peer = 0;
			// goes thought each shard and links each peer to another shard one at a time.
			for (int i = 0; i < _shards; ++i) // number of shards that we have
				for (int j = i; j < _peerspershard; ++j) { // for each peer in a shard
				    // shard matches peer index in other shards i.e. the first shard intersects every other shard on their first peer
				    // i: origin shard index and peer index in all other shards, origin is the shard that is said to have the peer
				    // j: j is the shard index that we iterate over
					_peers[peer].insert((*_system[i])[j]); // takes the jth peer of the ith quorum and insert
					_peers[peer].insert((*_system[j + 1])[i]); // takes the j+1 quorum (next one) and gets the ith peer
					++peer; // moves to next peer in the current shard
				}
		}


		void printPeers() {
			for (auto e : _peers) {
				std::cerr << "Peer " << e.first << ": ";
				for (auto f : e.second)
					std::cerr << f->id() << " quorum " << f->getShard() <<  ", ";
				std::cerr << std::endl;
			}
		}
		
		void setFaultTolerance(double tolerance) {
			for (auto e : _system) 
				for (int i = 0; i < _peerspershard; ++i)
					(*e)[i]->setFaultTolerance(tolerance);

		}

		void setRequestsPerRound(int requestPerRound) {
			for (auto e : _system)
				for (int i = 0; i < _peerspershard; ++i)
					(*e)[i]->setRequestPerRound(requestPerRound);
		}

		void setRoundsToRequest(int roundstoRequest) {
			for (auto e : _system)
				for (int i = 0; i < _peerspershard; ++i)
					(*e)[i]->setRoundsToRequest(roundstoRequest);
		}

		void setMaxWait() {
			for (auto e : _system)
				for (int i = 0; i < _peerspershard; ++i) {
					(*e)[i]->setMaxWait();
				}
		}

		std::set<markPBFT_peer*>& operator[] (int i) { 
			if (_peers.find(i) == _peers.end())
				std::cerr << "peer " << i << " does not exist";
			return _peers[i]; }

		void run(int rounds, int churn = 0, int churnRate = 100) {
			
			for (int i = 0; i < rounds; ++i) {
				
				if (i % 50 == 0 && i%churnRate != 0) {
					for (int j = 0; j < churn; ++j) {
						int choice = rand() % _peers.size();
						while (isByzantine(choice) == true) {
							choice = rand() % _peers.size();
						}
						makeByzantine(choice);
					}
				}

				if (i % churnRate == 0)
					for (int j = 0; j < _peers.size(); ++j)
						makeCorrect(j);
						
				for (auto e : _system) {
					for (int j = 0; j < _peerspershard; ++j)
						(*e)[j]->makeRequest();
				}
				for(auto e:_system){
					for (int j = 0; j < _peerspershard; ++j)
						(*e)[j]->transmit();
					e->log();
				}
			}

		}

		int getConfirmationCount() {
			int total = 0;
			for (auto e : _system)
				for (int i = 0; i < _peerspershard; ++i)
					total += (*e)[i]->getLedger().size();
			return total;
		}

		int peerCount() { return _peers.size(); }

		void makeByzantine(int peer) {
			if (_peers.find(peer) == _peers.end())
				std::cerr << "peer does not exist making byzantine";
			for (auto e : _peers[peer])
				e->makeByzantine();
		}

		void makeCorrect(int peer) {
			if (_peers.find(peer) == _peers.end())
				std::cerr << "peer does not exist making correct";
			for (auto e : _peers[peer])
				e->makeCorrect();
		}

		bool isByzantine(int peer) {
			auto it = _peers.find(peer);
			if (it == _peers.end())
				std::cerr << "peer does not exist finding byzantine";
			return ((*_peers[peer].begin())->isByzantine());
		}

		int shardCount() {
			return _system.size();
		}

		void makeRequest(int forQuorum = -1, int toQuorum = -1, int toPeer = -1) {
			if (forQuorum == -1) {
				forQuorum = rand() % _shards;
			}

			if (toQuorum == -1) {
				toQuorum = rand() % _shards;
			}

			if (toPeer == -1) {
				toPeer == rand() % _peerspershard;
			}

			markPBFT_message requestMSG;
			requestMSG.creator_id = "MAGIC";
			requestMSG.type = "REQUEST";
			requestMSG.requestGoal = forQuorum;

			(*_system[toQuorum])[toPeer]->makeRequest(requestMSG);

		}

private:
	std::vector < ByzantineNetwork<markPBFT_message, markPBFT_peer>*> _system; // list of shards in the system
	std::ostream* _out;
	int _peerspershard;
	int _shards;
	std::map<int, std::set<markPBFT_peer*>> _peers; // peer id, to list of real peers that it is in the quorums (vir peers)


};

#endif // SMART_SHARD_HPP