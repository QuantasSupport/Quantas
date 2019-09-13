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
			for (int i = 0; i < _shards; ++i)
				for (int j = i; j < _peerspershard; ++j) {
					_peers[peer].insert((*_system[i])[j]);
					_peers[peer].insert((*_system[j + 1])[i]);
					++peer;
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

		void run(int rounds) {
			
			for (int i = 0; i < rounds; ++i) {
				if (i % 50 == 0)
					makeByzantine(rand() % _peers.size());
				if (i % 100 == 0)
					for (int j = 0; j < _peers.size(); ++j)
						makeCorrect(j);
						
				for (auto e : _system) {
					for (int j = 0; j < _peerspershard; ++j)
						(*e)[j]->makeRequest();
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

private:
	std::vector < ByzantineNetwork<markPBFT_message, markPBFT_peer>*> _system;
	std::ostream* _out;
	int _peerspershard;
	int _shards;
	std::map<int, std::set<markPBFT_peer*>> _peers;


};

#endif // SMART_SHARD_HPP