#ifndef NETWORKINTERFACE_HPP
#define NETWORKINTERFACE_HPP

#include <memory>
#include <map>
#include <set>
#include <deque>
#include <string>
#include <algorithm>
#include <mutex>
#include "Packet.hpp"

namespace quantas {

class NetworkInterface {
protected:
    // "Public" ID + unique internal ID
    interfaceId _publicId{NO_PEER_ID};
    interfaceId _internalId{NO_PEER_ID};

    // set of public ids this peer thinks it is currently directly connected to 
    std::set<interfaceId> _neighbors;

    // Our local arrived messages
    std::deque<Packet> _inStream;

    // mutex lock for adding and removing messages from _inStream
    std::mutex _inStream_mtx;
public:
    inline NetworkInterface() {};
    inline NetworkInterface(interfaceId pubId) : _publicId(pubId) {};
    inline NetworkInterface(interfaceId pubId, interfaceId internalId) : _publicId(pubId), _internalId(internalId) {};
    inline ~NetworkInterface() {};

    // getters
    inline interfaceId publicId()   const { return _publicId; }
    inline interfaceId internalId() const { return _internalId; }
    inline std::set<interfaceId> neighbors() const {return _neighbors; }
    inline void setPublicId(interfaceId pid) { _publicId = pid; }
    inline void addNeighbor(interfaceId nbr) {_neighbors.insert(nbr);};
    inline void removeNeighbor(interfaceId nbr) {_neighbors.erase(nbr);};

    // Send messages to to others using these
    virtual void unicastTo (json msg, const interfaceId& dest) = 0;
    virtual void unicast (json msg);
    virtual void multicast (json msg, const std::set<interfaceId>& targets);
    virtual void broadcast (json msg);
    virtual void broadcastBut (json msg, const interfaceId& id);
    virtual void randomMulticast (json msg);

    // Pop from local arrived inStream
    inline Packet popInStream();
    inline bool inStreamEmpty() {
        std::lock_guard<std::mutex> lock(_inStream_mtx);
        return _inStream.empty(); 
    }

    // moves msgs to the inStream if they've arrived
    virtual void receive() = 0;

    // Clear everything
    virtual void clearAll() {
        _inStream.clear();
        _neighbors.clear();
    };
};

// ----------------------------------
// Implementation
// ----------------------------------

// Unicast to the *first* neighbor (if any exist)
inline void NetworkInterface::unicast(json msg) {
    if (!_neighbors.empty()) {
        auto firstNbr = *_neighbors.begin();
        unicastTo(msg, firstNbr);
    }
}

inline void NetworkInterface::multicast(json msg, const std::set<interfaceId>& targets) {
    for (auto nbr : targets) {
        unicastTo(msg, nbr);
    }
}

inline void NetworkInterface::broadcast(json msg) {
    multicast(msg, _neighbors);
}

inline void NetworkInterface::broadcastBut(json msg, const interfaceId& exceptId) {
    for (auto nbr : _neighbors) {
        if (nbr == exceptId) continue;
        unicastTo(msg, nbr);
    }
}

// Randomly sends to a *random subset* of neighbors
inline void NetworkInterface::randomMulticast(json msg) {

    // pick a random subset size from 0..neighbors.size()
    int count = uniformInt(0, (int)_neighbors.size());

    std::vector<interfaceId> temp(_neighbors.begin(), _neighbors.end());  // Copy set to vector
    std::shuffle(temp.begin(), temp.end(), threadLocalEngine());  // Shuffle vector
    std::set<interfaceId> subset(temp.begin(), temp.begin() + count);  // Take the first 'count' elements

    multicast(msg, subset);
}

inline Packet NetworkInterface::popInStream() {
    std::lock_guard<std::mutex> lock(_inStream_mtx);
    if (_inStream.empty()) {
        return Packet();
    }
    Packet p = std::move(_inStream.front());
    _inStream.pop_front();
    return p;
}
} // end namespace quantas

#endif
