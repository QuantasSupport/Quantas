
/**
 * A Channel object that "belongs" to the INBOUND side (the receiver).
 * The inbound side stores a std::shared_ptr<Channel>,
 * while the outbound side keeps only a std::weak_ptr<Channel>.
 *
 * This channel stores a queue of packets that are being delivered
 * to the inbound interface. The outbound interface calls pushPacket(...)
 * via its weak pointer (if still valid). On each simulation step/round,
 * the inbound interface can call processChannel() to reorder/drop/duplicate,
 * and eventually deliver up to 'maxMsgsRec' messages to itself.
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

// ---------------- Implementation ----------------

inline Channel::Channel(interfaceId targetId, interfaceId targetInternalId,
                        interfaceId sourceId, interfaceId sourceInternalId,
                        const nlohmann::json &channelParams)
: _targetId(targetId),
  _targetInternalId(targetInternalId),
  _sourceId(sourceId),
  _sourceInternalId(sourceInternalId)
{
    setParameters(channelParams);
}

inline void Channel::setParameters(const nlohmann::json &params) {
    if (params.contains("dropProbability")) {
        _dropProbability = params["dropProbability"];
    }
    if (params.contains("reorderProbability")) {
        _reorderProbability = params["reorderProbability"];
    }
    if (params.contains("duplicateProbability")) {
        _duplicateProbability = params["duplicateProbability"];
    }
    if (params.contains("maxMsgsRec")) {
        _maxMsgsRec = params["maxMsgsRec"];
        _throughputLeft = _maxMsgsRec*(RoundManager::instance()->lastRound()+1-RoundManager::instance()->currentRound());
    }
    if (params.contains("size")) {
        _size = params["size"];
    }
    if (params.contains("avgDelay")) {
        _avgDelay = params["avgDelay"];
    }
    if (params.contains("minDelay")) {
        _minDelay = params["minDelay"];
    }
    if (params.contains("maxDelay")) {
        _maxDelay = params["maxDelay"];
    }
    if (params.contains("type")) {
        std::string t = params["type"];
        if (t == "UNIFORM")  _delayStyle = DelayStyle::DS_UNIFORM;
        else if (t == "POISSON") _delayStyle = DelayStyle::DS_POISSON;
        else if (t == "ONE") _delayStyle = DelayStyle::DS_ONE;
    }
}

inline int Channel::computeRandomDelay() const {
    int delay = 1;
    switch(_delayStyle) {
    case DelayStyle::DS_UNIFORM:
        delay = uniformInt(_minDelay, _maxDelay);
        break;
    case DelayStyle::DS_POISSON:
        delay = poissonInt(_avgDelay);
        // clamp
        if (delay > _maxDelay) delay = _maxDelay;
        if (delay < _minDelay) delay = _minDelay;
        break;
    case DelayStyle::DS_ONE:
        delay = 1;
        break;
    }
    return delay;
}

inline void Channel::pushPacket(Packet pkt) {
    // possible drop
    if (trueWithProbability(_dropProbability)) {
        // drop => do nothing
        return;
    }

    do {
        if (!canSend()) {
            // No throughput left
            return;
        }

        consumeThroughput();

        // set random delay
        int d = computeRandomDelay();
        pkt.setDelay(d, d);

        // push into queue
        _packetQueue.push_back(pkt);
    
    } while (trueWithProbability(_duplicateProbability)); // possible duplication
}

inline void Channel::shuffleChannel() {
    // reorder
    if (_packetQueue.size() > 1 && trueWithProbability(_reorderProbability)) {
        std::shuffle(_packetQueue.begin(), _packetQueue.end(), threadLocalEngine());
    }
}

inline Packet Channel::popPacket() {
    Packet p = std::move(_packetQueue.front());
    _packetQueue.pop_front();
    return p;
}

} // end namespace quantas

#endif
