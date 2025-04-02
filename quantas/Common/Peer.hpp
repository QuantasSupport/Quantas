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

class PeerRegistry {
public:
    static PeerRegistry* instance(){
        static PeerRegistry s;
        return &s;
     }

    static Peer* makePeer(const std::string &type, interfaceId pubId) {
        PeerRegistry* inst = instance();
        auto it = inst->registry.find(type);
        if (it == inst->registry.end()) {
            throw std::runtime_error("Unknown peer type: " + type);
        }
        return it->second(pubId);
    }

    static bool registerPeerType(const std::string &name, std::function<Peer*(interfaceId)> factory) {
        PeerRegistry* inst = instance();
        bool result = inst->registry.insert(std::make_pair(name, factory)).second;
        if (!result) {
            std::cout << "Peer of type:" << name <<" already registered." << std::endl;
        }
        return result;
    }

    ~PeerRegistry(){

    }
private:
    // copying and creation prohibited 
    PeerRegistry() {}
    PeerRegistry(const PeerRegistry&) = delete;
    PeerRegistry& operator=(const PeerRegistry&) = delete;

    std::unordered_map<std::string, std::function<Peer*(interfaceId)>> registry;
};

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
    virtual void endOfRound(std::vector<Peer*>& peers) {}

private:

};

} // namespace quantas

#endif // PEER_HPP
