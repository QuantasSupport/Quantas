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
#include "Abstract/NetworkInterfaceAbstract.hpp"
#include "Concrete/NetworkInterfaceConcrete.hpp"
#include "RoundManager.hpp"
#include "LogWriter.hpp"

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

    static Peer* makePeer(const std::string &type, interfaceId pubId = 0) {
        PeerRegistry* inst = instance();
        if (inst->registry.bucket_count() == 0)
            throw  std::runtime_error("unordered_map has zero buckets in makePeer() in file Peer.hpp");
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

    ~PeerRegistry(){}

private:
    // copying and creation prohibited 
    PeerRegistry() {}
    PeerRegistry(const PeerRegistry&) = delete;
    PeerRegistry& operator=(const PeerRegistry&) = delete;

    std::unordered_map<std::string, std::function<Peer*(interfaceId)>> registry;
};

// The base Peer class
class Peer {
public:
inline Peer() {}
    inline Peer(NetworkInterface* networkInterface) : _networkInterface(networkInterface) {}
    inline Peer(const Peer &rhs) {};
    inline virtual ~Peer() {};

    virtual void clearInterface() {
        if (_networkInterface != nullptr) {
            _networkInterface->clearAll();
            delete _networkInterface;
            _networkInterface = nullptr;
        }
    }

    // Called once after constructing all peers
    // (subclass can parse 'parameters' as needed)
    virtual void initParameters(const std::vector<Peer*>& peers,
                                json parameters) {}

    // try to run performComputation though it may not
    virtual void tryPerformComputation() {
        if (!isCrashed()) {
            performComputation();
        }
    };

    // Main algorithmic step that each Peer runs each round
    virtual void performComputation() = 0;

    // Called after performComputation in each round (subclass can override to collect metrics, etc.)
    virtual void endOfRound(std::vector<Peer*>& peers) {}
    
    bool isCrashed() {return (_crashRecoveryRound > RoundManager::currentRound());}
    void setCrashRecoveryRound(size_t crashRecoveryRound) {_crashRecoveryRound = crashRecoveryRound;}


    ////////////////// Network Interface direct access ////////////////////
    // getters
    NetworkInterface* getNetworkInterface() const { return _networkInterface; };
    interfaceId publicId()   const { return _networkInterface->publicId(); };
    interfaceId internalId() const { return _networkInterface->internalId(); };
    std::set<interfaceId> neighbors() const { return _networkInterface->neighbors(); };
    void setPublicId(interfaceId pid) { _networkInterface->setPublicId(pid); };
    void addNeighbor(interfaceId nbr) { _networkInterface->addNeighbor(nbr); };
    void removeNeighbor(interfaceId nbr) { _networkInterface->removeNeighbor(nbr); };

    // Send messages to to others using these
    virtual void unicastTo (json msg, const interfaceId& dest) { _networkInterface->unicastTo(msg, dest); };
    virtual void unicast (json msg) { _networkInterface->unicast(msg); };
    virtual void multicast (json msg, const std::set<interfaceId>& targets) { _networkInterface->multicast(msg, targets); };
    virtual void broadcast (json msg) { _networkInterface->broadcast(msg); };
    virtual void broadcastBut (json msg, const interfaceId& id) { _networkInterface->broadcastBut(msg, id); };
    virtual void randomMulticast (json msg) { _networkInterface->randomMulticast(msg); };

    // Pop from local arrived inStream
    Packet popInStream() { return _networkInterface->popInStream(); };
    bool inStreamEmpty() const { return _networkInterface->inStreamEmpty(); }

    // moves msgs to the inStream if they've arrived
    void receive() { _networkInterface->receive(); };

    // Clear everything
    void clearAll() { _networkInterface->clearAll(); };

protected:
    size_t _crashRecoveryRound = 0;
    NetworkInterface* _networkInterface = nullptr;
};

} // namespace quantas

#endif // PEER_HPP
