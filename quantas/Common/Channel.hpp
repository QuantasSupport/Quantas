
/**
 * This channel stores a queue of packets that are being delivered
 * to the inbound interface. The outbound interface calls pushPacket(...)
 * via its pointer.
 */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP



#include <memory>
#include <deque>
#include <random>
#include <algorithm>
#include <stdexcept>
#include <climits>
#include "Json.hpp"
#include "RandomUtil.hpp"
#include "Packet.hpp"

namespace quantas {

using std::deque;
using nlohmann::json;

// Forward declaration
class NetworkInterface;


class Channel : public std::enable_shared_from_this<Channel> {
private:
    interfaceId _targetId{ NO_PEER_ID };
    interfaceId _targetInternalId{ NO_PEER_ID };

    interfaceId _sourceId{ NO_PEER_ID };
    interfaceId _sourceInternalId{ NO_PEER_ID };

    // =============== Reliability Parameters ===============
    double _dropProbability{0.0};      // chance of dropping each incoming packet
    double _reorderProbability{0.0};   // chance of shuffling the queue
    double _duplicateProbability{0.0}; // chance of duplicating an incoming packet
    int    _maxMsgsRec{INT_MAX};       // max messages we deliver from the channel each round
    int    _size{INT_MAX};             // fixed size of the channel (different than throughputLeft)
    int    _throughputLeft{INT_MAX};   // throughputLeft, if you want a limited number of sends to reduce the maximum size of the channel

    // Delay distribution
    int    _avgDelay{1};
    int    _minDelay{1};
    int    _maxDelay{1};
    enum class DelayStyle { DS_UNIFORM, DS_POISSON, DS_ONE };
    DelayStyle _delayStyle{DelayStyle::DS_UNIFORM};

    // These are the packets that have been "sent" by the source side
    // but not yet delivered to the target side.
    deque<Packet> _packetQueue;

    // Helpers
    bool canSend() const { return (_throughputLeft != 0 && _size > _packetQueue.size()); }
    int computeRandomDelay() const;
    void consumeThroughput() {
        if (_throughputLeft > 0) {
            _throughputLeft--;
        }
    }

public:
    Channel() = default;
    ~Channel() = default; // automatically freed when shared_ptr (inbound side) is destroyed

    // Create a channel with references to inbound side and outbound side
    Channel(interfaceId targetId, interfaceId targetInternalId,
            interfaceId sourceId, interfaceId sourceInternalId,
            const json &channelParams);

    interfaceId targetId() {return _targetId;}
    interfaceId targetInternalId() {return _targetInternalId;}
    interfaceId sourceId() {return _sourceId;}
    interfaceId sourceInternalId() {return _sourceInternalId;}

    void setParameters(const nlohmann::json &params);

    // Called by the source to push a new packet into the queue
    void pushPacket(Packet pkt);

    // Called by the target before removing packets from the queue
    void shuffleChannel();

    // Called by the target to remove packets from the queue
    Packet popPacket();

    // Helpers
    bool empty() const {return _packetQueue.empty();}

    int maxMsgsRec() const {return _maxMsgsRec;}

    bool frontHasArrived() const {
        if (_packetQueue.empty()) return false;
        return _packetQueue.front().hasArrived();
    }
};
} // end namespace quantas

#endif
