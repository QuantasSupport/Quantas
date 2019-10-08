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
		SmartShard(const int& shards, std::ostream& out, int delay, int peerspershard = -1, int reserveSize = 0, int quorumIntersection = 1) {
            _numberOfPeersInReserve = reserveSize;
            _out = &out;
            _shards = shards;
            if (peerspershard == -1)
                _peerspershard = shards - 1;
            else
                _peerspershard = peerspershard;

            // Initialize Quorums
            for (int i = 0; i < _shards; ++i) {
                _system.push_back(new ByzantineNetwork<markPBFT_message, markPBFT_peer>());
                _system[i]->setLog(*_out);
                _system[i]->setToRandom();
                _system[i]->setMaxDelay(delay);
                _system[i]->initNetwork(_peerspershard * quorumIntersection);
                (*_system[i])[rand() % _peerspershard* quorumIntersection]->setPrimary(true);
                for (int j = 0; j < _peerspershard * quorumIntersection; ++j)
                    (*_system[i])[j]->setShard(i);

            }


            int peer = 0; // Assign to virtual peer
            int peerIndex = 0; // peer index for shards

			for (int shard = 0; shard < _system.size(); ++shard) {
				for (int offset = 0; offset < quorumIntersection; ++offset) {
					for (int vPeer = shard; vPeer < _peerspershard; ++vPeer) {
						_peers[peer].insert((*_system[shard])[vPeer+_peerspershard*offset]);
						_peers[peer].insert((*_system[vPeer + 1])[shard + offset * _peerspershard]);
						++peer;
					}
				}
			}
			_peerspershard *= quorumIntersection;
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
			return _peers[i];
	    }

	    // rounds: number of rounds in computation
	    // churn: number of peers that can churn per churnRate
	    // churnRate: number of rounds between peers leaving/coming into network
		void run(int rounds, int churn = 0, int churnRate = 100) {
			
			for (int i = 0; i < rounds; ++i) {
			    if(i % churnRate == 0){
                    for(int i = 0; i < churn; i++){
                        churnCycle();
                    }
			    }

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

		void churnCycle(){
	        assert(_numberOfPeersInReserve > -1);

            bool addingPeer = rand()%2;
            if(addingPeer){
                if(_numberOfPeersInReserve == 0){

                    for(int peer = 0; peer < _peers.size(); peer++){
                        if(isByzantine(peer)){
                            makeCorrect(peer);
							std::cerr << "dropped peer exists, adding to dropped\n";
							std::cerr << "reserves: " << _numberOfPeersInReserve << std::endl;
							std::cerr << "num of dropped peers: ";
							int droppedCount = 0;
							for (int peer = 0; peer < _peers.size(); ++peer)
								if (isByzantine(peer))
									++droppedCount;
							std::cerr << droppedCount << std::endl << std::endl;
                            return;
                        }
                    }
                }
				_numberOfPeersInReserve++;
				std::cerr << "no dropped peers, adding to reserve\n";
				std::cerr << "reserves: " << _numberOfPeersInReserve << std::endl;
				std::cerr << "num of dropped peers: ";
				int droppedCount = 0;
				for (int peer = 0; peer < _peers.size(); ++peer)
					if (isByzantine(peer))
						++droppedCount;
				std::cerr << droppedCount << std::endl<<std::endl;

            }else{
                if(_numberOfPeersInReserve == 0){
                    if(getByzantine() == size()){
                        return;
                    }
                    int peerToDrop = rand()%_peers.size();
					while (isByzantine(peerToDrop)) {
						peerToDrop = rand() % _peers.size();
					}
                    makeByzantine(peerToDrop);
					std::cerr << "no reserve peers, dropping peer\n";

					std::cerr << "reserves: " << _numberOfPeersInReserve << std::endl;
					std::cerr << "num of dropped peers: ";
					int droppedCount = 0;
					for (int peer = 0; peer < _peers.size(); ++peer)
						if (isByzantine(peer))
							++droppedCount;
					std::cerr << droppedCount << std::endl << std::endl;

                }else{
                    _numberOfPeersInReserve--;
					std::cerr << "peers in reverse, removing from reserve\n";
					std::cerr << "reserves: " << _numberOfPeersInReserve << std::endl;
					std::cerr << "num of dropped peers: ";
					int droppedCount = 0;
					for (int peer = 0; peer < _peers.size(); ++peer)
						if (isByzantine(peer))
							++droppedCount;
					std::cerr << droppedCount << std::endl <<std::endl;
                }
            }
	    }

	    int size(){
	        int total = 0;
	        for(auto s : _system){
	            total += s->size();
	        }
	        return total;
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

		int getByzantine(){
	        int total = 0;
	        for(auto s : _system){
	            total += s->getByzantine().size();
	        }
	        return total;
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

        std::vector < ByzantineNetwork<markPBFT_message, markPBFT_peer>*> getQuorums(){
	        return _system;
	    }

	    int getShardSize(){
	        return _peerspershard;
	    }

private:
	std::vector < ByzantineNetwork<markPBFT_message, markPBFT_peer>*> _system; // list of shards in the system
	std::ostream* _out;
	int _peerspershard;
	int _shards;
	int _numberOfPeersInReserve;
	std::map<int, std::set<markPBFT_peer*>> _peers; // peer id, to list of real peers that it is in the quorums (vir peers)


};

#endif // SMART_SHARD_HPP