#include "SmartShard.hpp"
#include "params_common.h"

SmartShard::SmartShard(const int& shards, std::ostream& out, int delay, int peerspershard = -1, int reserveSize = 0, int quorumIntersection = 1) {
	_out = &out;
	_shards = shards;
	if (peerspershard == -1)
        _peersPerShard = shards - 1;
	else
        _peersPerShard = peerspershard;

	// Initialize Quorums
	for (int i = 0; i < _shards; ++i) {
		_system.push_back(new ByzantineNetwork<markPBFT_message, markPBFT_peer>());
		_system[i]->setLog(*_out);
		_system[i]->setToRandom();
		_system[i]->setMaxDelay(delay);
		_system[i]->initNetwork(_peersPerShard);
		(*_system[i])[rand() % _peersPerShard]->setPrimary(true);
		for (int j = 0; j < _peersPerShard; ++j) {
            (*_system[i])[j]->setShard(i);
            (*_system[i])[j]->setShardCount(_shards);
        }
	}

	// map PBFT instances (called peers) to virtual peers (actual peers)
    int vPeer = 0;
    for(int quorum = 0; quorum < (_system.size()-1); quorum++){
        int nextQuorum = quorum +1 ;
        int offset = quorum*quorumIntersection;
        for(int thisPBFTInstances = offset; thisPBFTInstances < _system[quorum]->size(); ) {
            for (int otherPBFTInstances = 0; otherPBFTInstances < quorumIntersection; otherPBFTInstances++) {
                markPBFT_peer* peerFromThisQuorum = (*_system[quorum])[thisPBFTInstances];
                markPBFT_peer* peerFromOtherQuorum = (*_system[nextQuorum])[quorum + otherPBFTInstances];
                assert(peerFromThisQuorum != NULL);
                assert(peerFromOtherQuorum != NULL);
                _peers[vPeer].insert(peerFromThisQuorum);
                _peers[vPeer].insert(peerFromOtherQuorum);
                vPeer++;
                thisPBFTInstances++;
            }
            nextQuorum++;
        }
    }
    setupShardNeighborhood();

    if(shards*peerspershard < PEER_COUNT){
        _numberOfPeersInReserve = reserveSize + (PEER_COUNT - (shards*peerspershard)); // add left over peers to reserve
    }else {
        _numberOfPeersInReserve = reserveSize;
    }
}

void SmartShard::printPeers() {
	for (auto e : _peers) {
		std::cerr << "Peer " << e.first << ": ";
		for (auto f : e.second)
			std::cerr << f->id() << " quorum " << f->getShard() << ", ";
		std::cerr << std::endl;
	}
}

void SmartShard::setFaultTolerance(double tolerance) {
	for (auto e : _system)
		for (int i = 0; i < _peersPerShard; ++i)
			(*e)[i]->setFaultTolerance(tolerance);

}