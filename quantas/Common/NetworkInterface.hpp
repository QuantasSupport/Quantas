#ifndef NETWORKINTERFACE_HPP
#define NETWORKINTERFACE_HPP

#include <memory>
#include <map>
#include <set>
#include <deque>
#include <string>
#include <algorithm>
#include "Channel.hpp"
#include "Packet.hpp"

namespace quantas {

class NetworkInterface {
private:
    static inline interfaceId s_internalCounter = NO_PEER_ID;

    // "Public" ID + unique internal ID
    interfaceId _publicId{NO_PEER_ID};
    interfaceId _internalId{NO_PEER_ID};

    // set of public ids this peer thinks it is currently directly connected to 
    std::set<interfaceId> _neighbors;

    // Inbound channels
    //   key = source peer's public ID
    std::multimap<interfaceId, std::shared_ptr<Channel>> _inBoundChannels;

    // Outbound channels
    //   key = target peer's public ID
    std::multimap<interfaceId, std::shared_ptr<Channel>> _outBoundChannels;

    // Our local arrived messages
    std::deque<Packet> _inStream;

protected:
    // Send messages to to others using these
    inline void unicastTo (Message* msg, const interfaceId& dest);
    inline void unicast (Message* msg);
    inline void multicast (Message* msg, const std::set<interfaceId>& targets);
    inline void broadcast (Message* msg);
    inline void broadcastBut (Message* msg, const interfaceId& id);
    inline void randomMulticast (Message* msg);

    // Pop from local arrived inStream
    inline Packet popInStream();
    inline bool inStreamEmpty() const { return _inStream.empty(); }

public:
    inline NetworkInterface();
    inline NetworkInterface(interfaceId pubId);
    inline ~NetworkInterface() = default;
    static inline void resetCounter() {s_internalCounter = NO_PEER_ID;}

    inline interfaceId publicId()   const { return _publicId; }
    inline interfaceId internalId() const { return _internalId; }
    inline std::set<interfaceId> neighbors() {return _neighbors;}

    inline void setPublicId(interfaceId pid) { _publicId = pid; }

    inline void addNeighbor(interfaceId nbr) {_neighbors.insert(nbr);};
    inline void removeNeighbor(interfaceId nbr) {_neighbors.erase(nbr);};
    inline void addInboundChannel(interfaceId srcPubId, std::shared_ptr<Channel> ch) {_inBoundChannels.emplace(srcPubId, ch);}
    inline void addOutboundChannel(interfaceId tPubId, std::shared_ptr<Channel> ch) {_outBoundChannels.emplace(tPubId, ch);}
    inline void removeOutboundChannelByPublic(interfaceId remotePubId) {
        auto range = _outBoundChannels.equal_range(remotePubId);
        _outBoundChannels.erase(range.first, range.second);
    }
    inline void removeOutboundChannelByInternal(interfaceId targetInternalId) {
        auto range = _outBoundChannels.equal_range(targetInternalId);
        for (auto channel = _outBoundChannels.begin(); channel != _outBoundChannels.end(); ++channel) {
            if (channel->second->targetInternalId() == targetInternalId) {
                _outBoundChannels.erase(channel);
            }
        }
    }

    // moves msgs from the channel to the inStream if they've arrived
    inline void receive();

    // Clear everything
    inline void clearAll();
};

// ----------------------------------
// Implementation
// ----------------------------------

inline NetworkInterface::NetworkInterface()
{
    _internalId = ++s_internalCounter;
}

inline NetworkInterface::NetworkInterface(long pubId)
: _publicId(pubId)
{
    _internalId = ++s_internalCounter;
}

inline void NetworkInterface::unicastTo(Message* msg, const interfaceId& nbr) {
    if (_neighbors.find(nbr) == _neighbors.end()) return;
    // find the channels with that key if they are our neighbor
    auto range = _outBoundChannels.equal_range(nbr);
    for (auto it = range.first; it != range.second; ++it) {
        Packet p;
        p.setSource(publicId());
        p.setTarget(nbr);
        p.setMessage(msg);
        it->second->pushPacket(p);
    }
}

// Unicast to the *first* neighbor (if any exist)
inline void NetworkInterface::unicast(Message* msg) {
    if (!_neighbors.empty()) {
        auto firstNbr = *_neighbors.begin();
        unicastTo(msg, firstNbr);
    }
}

inline void NetworkInterface::multicast(Message* msg, const std::set<interfaceId>& targets) {
    for (auto nbr : targets) {
        unicastTo(msg, nbr);
        msg = msg->clone();
    }
    delete msg;
}

inline void NetworkInterface::broadcast(Message* msg) {
    multicast(msg, _neighbors);
}

inline void NetworkInterface::broadcastBut(Message* msg, const interfaceId& exceptId) {
    for (auto nbr : _neighbors) {
        if (nbr == exceptId) continue;
        unicastTo(msg, nbr);
        msg = msg->clone();
    }
    delete msg;
}

// Randomly sends to a *random subset* of neighbors
inline void NetworkInterface::randomMulticast(Message* msg) {

    // pick a random subset size from 0..neighbors.size()
    int count = uniformInt(0, (int)_neighbors.size());

    std::vector<interfaceId> temp(_neighbors.begin(), _neighbors.end());  // Copy set to vector
    std::shuffle(temp.begin(), temp.end(), threadLocalEngine());  // Shuffle vector
    std::set<interfaceId> subset(temp.begin(), temp.begin() + count);  // Take the first 'count' elements

    multicast(msg, subset);
}

inline void NetworkInterface::receive() {
    for (auto it = _inBoundChannels.begin(); it != _inBoundChannels.end(); ++it) {
        auto &chPtr = it->second;

        // reorder if needed
        chPtr->shuffleChannel();

        // pop up to chPtr->maxMsgsRec() messages that have arrived
        int recCount = 0;
        while (!chPtr->empty() 
               && chPtr->frontHasArrived() 
               && recCount < chPtr->maxMsgsRec()) 
        {
            Packet arrivedPkt = chPtr->popPacket();
            _inStream.push_back(std::move(arrivedPkt));
            ++recCount;
        }
    }
}

inline Packet NetworkInterface::popInStream() {
    if (_inStream.empty()) {
        return Packet();
    }
    Packet p = std::move(_inStream.front());
    _inStream.pop_front();
    return p;
}

inline void NetworkInterface::clearAll() {
    _inStream.clear();
    _inBoundChannels.clear();  
    _outBoundChannels.clear();
    _neighbors.clear();
}

} // end namespace quantas

#endif
