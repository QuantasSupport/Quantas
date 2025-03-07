/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QUANTAS is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <vector>
#include <string>
#include <random>
#include <limits>
#include <iostream>
#include <algorithm>
#include <memory>
#include <deque>
#include <climits>   // for INT_MAX
#include "Peer.hpp"  // Your custom Peer class (inherits NetworkInterface)
#include "Json.hpp"

namespace quantas {

using nlohmann::json;

class Network {
private:
    std::vector<Peer*>  _peers;

    json _distribution;
    Network& operator=(const Network &rhs) = delete;
    Network(const Network &rhs) = delete;

public:
    Network();
    ~Network();
    
    void setDistribution (json distribution) {_distribution = distribution;}
    // -------------- TOPOLOGY INIT --------------
    // This can create the peers, set up neighbors, etc.
    void initNetwork(json topology);

    // -------------- Topology Helpers --------------
    // Each function sets up "neighbors" among subsets of _peers
    void fullyConnect(int numberOfPeers);
    void star(int numberOfPeers);
    void grid(int height, int width);
    void torus(int height, int width);
    void chain(int numberOfPeers);
    void ring(int numberOfPeers);
    void unidirectionalRing(int numberOfPeers);
    void userList(json topology);
    void createInitialChannels();

    // -------------- Specialized Initilization ------------
    void initParameters(json parameters) {
        _peers[0]->initParameters(_peers, parameters);
    }

    // -------------- Simulation loop --------------
    // call each peer's receive, performComputation.
    void receive(int begin, int end);    
    void performComputation(int begin, int end);

    void endOfRound() {_peers[0]->endOfRound(_peers); }

    // -------------- Access by index --------------
    // (Might be optional if you rarely do random access.)
    Peer*       operator[](int i)       { return _peers[i]; }
    const Peer* operator[](int i) const { return _peers[i]; }
};

// -------------------------------------------------
// Implementation
// -------------------------------------------------

inline Network::Network()
: _peers()
{
}

inline Network::~Network() {
    // delete all owned peers
    for (auto *p : _peers) {
        delete p;
    }
    _peers.clear();
}

// create peers based on "topology" JSON
// at this stage all public and internal 
// ids are the same and unique across peers
inline void Network::initNetwork(json topology) {
    // Clear existing
    for (auto *p : _peers) {
        delete p;
    }
    _peers.clear();

    NetworkInterface::resetCounter();

    int initialPeers = topology.value("initialPeers", 0);
    std::string peerType = topology.value("initialPeerType", "");
    // build peers
    for (int i = 0; i < initialPeers; i++) {
        auto *peer = PeerRegistry::makePeer(peerType, i);
        _peers.push_back(peer);
    }

    if (topology.value("identifiers", "") == "random") {
        static std::mt19937 rng(std::random_device{}());
        std::shuffle(_peers.begin(), _peers.end(), rng);
    }

    // pick the topology
    std::string t = topology.value("type", "");

    if (t == "complete") {
        fullyConnect(initialPeers);
    } else if (t == "star") {
        star(initialPeers);
    } else if (t == "grid") {
        int h = topology.value("height", 1);
        int w = topology.value("width", 1);
        grid(h, w);
    } else if (t == "torus") {
        int h = topology.value("height", 1);
        int w = topology.value("width", 1);
        torus(h, w);
    } else if (t == "chain") {
        chain(initialPeers);
    } else if (t == "ring") {
        ring(initialPeers);
    } else if (t == "unidirectionalRing") {
        unidirectionalRing(initialPeers);
    } else if (t == "userList") {
        userList(topology);
    } else {
        std::cerr << "Error: missing or unknown topology 'type' in JSON.\n";
    }

    createInitialChannels();
}

inline void Network::createInitialChannels() {
// For each peer in the network create their channels from their neighbors
for (auto* peer : _peers) {
    auto neighbors    = peer->neighbors();
    for (auto nbr : neighbors) {
            auto channelPtr = std::make_shared<Channel>(
                /* target IDs: */ 
                _peers[nbr]->publicId(), 
                _peers[nbr]->internalId(),
                /* outbound (the remote) IDs: */
                peer->publicId(),
                peer->internalId(),
                _distribution
            );
            _peers[nbr]->addInboundChannel(peer->publicId(), channelPtr);
            peer->addOutboundChannel(_peers[nbr]->publicId(), channelPtr);
        }
    }
}

// ------------- Topology Builders -------------

inline void Network::fullyConnect(int numberOfPeers) {
    // for each pair i<j, call addNeighbor
    for (int i = 0; i < numberOfPeers; i++) {
        for (int j = i + 1; j < numberOfPeers; j++) {
            if (i != j) {
                _peers[i]->addNeighbor(_peers[j]->internalId());
                _peers[j]->addNeighbor(_peers[i]->internalId());
            }
        }
    }
}

inline void Network::star(int numberOfPeers) {
    // connect all to peer[0]
    for (int i = 1; i < numberOfPeers; i++) {
        _peers[0]->addNeighbor(_peers[i]->internalId());
        _peers[i]->addNeighbor(_peers[0]->internalId());
    }
}

inline void Network::grid(int height, int width) {
    // interpret peers as a 2D grid
    // link up/down/left/right
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int idx = i * width + j;
            if (j + 1 < width) {
                // link horizontally
                _peers[idx]->addNeighbor(_peers[idx+1]->internalId());
                _peers[idx+1]->addNeighbor(_peers[idx]->internalId());
            }
            if (i + 1 < height) {
                // link vertically
                _peers[idx]->addNeighbor(_peers[idx+width]->internalId());
                _peers[idx+width]->addNeighbor(_peers[idx]->internalId());
            }
        }
    }
}

inline void Network::torus(int height, int width) {
    // similar to grid but wrap edges
    // ...
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int idx = i * width + j;
            // neighbor to right (wrapping)
            int rightIdx = i * width + ((j + 1) % width);
            _peers[idx]->addNeighbor(_peers[rightIdx]->internalId());
            _peers[rightIdx]->addNeighbor(_peers[idx]->internalId());

            // neighbor down (wrapping)
            int downIdx = ((i + 1) % height) * width + j;
            _peers[idx]->addNeighbor(_peers[downIdx]->internalId());
            _peers[downIdx]->addNeighbor(_peers[idx]->internalId());
        }
    }
}

inline void Network::chain(int numberOfPeers) {
    // link each i with i+1
    for (int i = 0; i < numberOfPeers - 1; i++) {
        _peers[i]->addNeighbor(_peers[i+1]->internalId());
        _peers[i+1]->addNeighbor(_peers[i]->internalId());
    }
}

inline void Network::ring(int numberOfPeers) {
    chain(numberOfPeers);
    // also link last back to first
    if (numberOfPeers > 1) {
        _peers[numberOfPeers - 1]->addNeighbor(_peers[0]->internalId());
        _peers[0]->addNeighbor(_peers[numberOfPeers - 1]->internalId());
    }
}

inline void Network::unidirectionalRing(int numberOfPeers) {
    // link i->(i+1)
    for (int i = 0; i < numberOfPeers - 1; i++) {
        _peers[i]->addNeighbor(_peers[i+1]->internalId());
    }
    // last -> first
    if (numberOfPeers > 1) {
        _peers[numberOfPeers - 1]->addNeighbor(_peers[0]->internalId());
    }
}

inline void Network::userList(json topology) {
    // "list": { "0": [1,2], "1":[0], ... }
    // for each peer i, read the adjacency
    int initialPeers = topology.value("initialPeers", 0);
    if (!topology.contains("list")) return;
    json lst = topology["list"];
    for (int i = 0; i < initialPeers; i++) {
        std::string key = std::to_string(i);
        if (lst.contains(key)) {
            for (auto &dest : lst[key]) {
                _peers[i]->addNeighbor(dest);
            }
        }
    }
}

inline void Network::receive(int begin, int end) {
    // call each peer's receive
    for (int i = begin; i < end && i < (int)_peers.size(); i++) {
        _peers[i]->receive();
    }
}

inline void Network::performComputation(int begin, int end) {
    // call each peer's performComputation
    for (int i = begin; i < end && i < (int)_peers.size(); i++) {
        _peers[i]->performComputation();
    }
}

} // namespace quantas

#endif // NETWORK_HPP
