#ifndef NETWORK_INTERFACE_ABSTRACT_HPP
#define NETWORK_INTERFACE_ABSTRACT_HPP

#include <memory>
#include <map>
#include <set>
#include <deque>
#include <string>
#include <algorithm>
#include "Channel.hpp"
#include "../Packet.hpp"
#include "../NetworkInterface.hpp"

namespace quantas {

class NetworkInterfaceAbstract : public NetworkInterface {
private:
    static inline interfaceId s_internalCounter = NO_PEER_ID;

    // Inbound channels
    // key = source peer's public ID
    std::multimap<interfaceId, std::shared_ptr<Channel>> _inBoundChannels;

    // Outbound channels
    // key = target peer's public ID
    std::multimap<interfaceId, std::shared_ptr<Channel>> _outBoundChannels;
public:

    inline NetworkInterfaceAbstract() {
        _internalId = ++s_internalCounter;
    };
    inline NetworkInterfaceAbstract(interfaceId pubId) : NetworkInterface(pubId) {
        _internalId = ++s_internalCounter;
    };
    inline NetworkInterfaceAbstract(interfaceId pubId, interfaceId internalId) : NetworkInterface(pubId, internalId) {};
    inline ~NetworkInterfaceAbstract() {};

    static inline void resetCounter() {s_internalCounter = NO_PEER_ID;}

    // setters
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

    // Send messages to to others using this
    inline void unicastTo (json msg, const interfaceId& dest) override;
    
    // moves msgs from the channel to the inStream if they've arrived
    inline void receive() override;

    inline void clearAll() override {
        _inStream.clear();
        _inBoundChannels.clear();  
        _outBoundChannels.clear();
        _neighbors.clear();
    }
};

void NetworkInterfaceAbstract::unicastTo(json msg, const interfaceId& nbr) {
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

inline void NetworkInterfaceAbstract::receive() {
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

}

#endif /* NETWORK_INTERFACE_ABSTRACT_HPP */