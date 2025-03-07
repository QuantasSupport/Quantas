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
} // namespace quantas

#endif // NETWORK_HPP
