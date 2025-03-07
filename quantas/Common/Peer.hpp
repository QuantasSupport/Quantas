/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QUANTAS is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef PEER_HPP
#define PEER_HPP

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <memory>
#include "NetworkInterface.hpp"

namespace quantas {

using nlohmann::json;

// Forward declaration, so we can pass Peer* in method signatures below
class Peer;

struct PeerRegistry {
    inline static std::unordered_map<std::string, std::function<Peer*(interfaceId)>> registry;

    inline static Peer* makePeer(const std::string &type, interfaceId pubId) {
        auto it = PeerRegistry::registry.find(type);
        if (it == PeerRegistry::registry.end()) {
            throw std::runtime_error("Unknown peer type: " + type);
        }
        return it->second(pubId);
    }
};

inline void registerPeerType(const std::string &name, std::function<Peer*(interfaceId)> factory) {
    PeerRegistry::registry[name] = factory;
}

// The base Peer class
class Peer : public NetworkInterface {
public:

    inline Peer() {};

    inline Peer(interfaceId pubId) : NetworkInterface(pubId) {}
    inline Peer(const Peer &rhs) {};
    inline virtual ~Peer() {};

    // Called once after constructing all peers
    // (subclass can parse 'parameters' as needed)
    virtual void initParameters(const std::vector<Peer*>& peers,
                                json parameters) {}

    // Main algorithmic step that each Peer runs each round
    virtual void performComputation() = 0;

    // Called after performComputation in each round (subclass can override to collect metrics, etc.)
    virtual void endOfRound(const std::vector<Peer*>& peers) {}

private:

};

} // namespace quantas

#endif // PEER_HPP
