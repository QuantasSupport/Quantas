#include "SmartShard.hpp"

SmartShard::SmartShard(const int& shards, std::ostream& out, int delay, int peerspershard = -1, int reserveSize = 0, int quorumIntersection = 1) {
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
		(*_system[i])[rand() % _peerspershard * quorumIntersection]->setPrimary(true);
		for (int j = 0; j < _peerspershard * quorumIntersection; ++j)
			(*_system[i])[j]->setShard(i);

	}


	int peer = 0; // Assign to virtual peer
	int peerIndex = 0; // peer index for shards

	for (int shard = 0; shard < _system.size(); ++shard) {
		for (int offset = 0; offset < quorumIntersection; ++offset) {
			for (int vPeer = shard; vPeer < _peerspershard; ++vPeer) {
				_peers[peer].insert((*_system[shard])[vPeer + _peerspershard * offset]);
				_peers[peer].insert((*_system[vPeer + 1])[shard + offset * _peerspershard]);
				++peer;
			}
		}
	}
	_peerspershard *= quorumIntersection;
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
		for (int i = 0; i < _peerspershard; ++i)
			(*e)[i]->setFaultTolerance(tolerance);

}