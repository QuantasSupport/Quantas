
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
#include <unordered_set>
#include "../Json.hpp"
#include "../RandomUtil.hpp"
#include "../Packet.hpp"

namespace quantas {

using std::deque;
using nlohmann::json;

// Forward declaration
class NetworkInterface;

enum class DelayStyle { DS_UNIFORM, DS_POISSON, DS_ONE };

class ChannelProperties {
public:    
    DelayStyle delayStyle{DelayStyle::DS_UNIFORM};

    ChannelProperties(const nlohmann::json &params) {
        setParameters(params);
    }

    void setParameters(const nlohmann::json &params) {
        dropProbability = params.value("dropProbability", 0.0);
        reorderProbability = params.value("reorderProbability", 0.0);
        duplicateProbability = params.value("duplicateProbability", 0.0);
        maxMsgsRec = params.value("maxMsgsRec", 1);
        size = params.value("size", INT_MAX);
        avgDelay = params.value("avgDelay", 1);
        minDelay = params.value("minDelay", 1);
        maxDelay = params.value("maxDelay", 1);
        std::string t = params.value("type", "UNIFORM");
        if (t == "UNIFORM")  delayStyle = DelayStyle::DS_UNIFORM;
        else if (t == "POISSON") delayStyle = DelayStyle::DS_POISSON;
        else if (t == "ONE") delayStyle = DelayStyle::DS_ONE;
    }

    bool operator==(const ChannelProperties& other) const {
        return dropProbability == other.dropProbability &&
                reorderProbability == other.reorderProbability &&
                duplicateProbability == other.duplicateProbability &&
                maxMsgsRec == other.maxMsgsRec &&
                size == other.size &&
                avgDelay == other.avgDelay &&
                minDelay == other.minDelay &&
                maxDelay == other.maxDelay &&
                delayStyle == other.delayStyle;
    }

    struct Hash {
        size_t operator()(const ChannelProperties* props) const {
            size_t h = 0;
            h ^= std::hash<double>{}(props->getDropProbability());
            h ^= std::hash<double>{}(props->getReorderProbability());
            h ^= std::hash<double>{}(props->getDuplicateProbability());
            h ^= std::hash<int>{}(props->getMaxMsgsRec());
            h ^= std::hash<int>{}(props->getSize());
            h ^= std::hash<int>{}(props->getAvgDelay());
            h ^= std::hash<int>{}(props->getMinDelay());
            h ^= std::hash<int>{}(props->getMaxDelay());
            h ^= std::hash<int>{}(static_cast<int>(props->getDelayStyle()));
            return h;
        }
    };

    // Getters
    double getDropProbability() const { return dropProbability; }
    double getReorderProbability() const { return reorderProbability; }
    double getDuplicateProbability() const { return duplicateProbability; }
    int getMaxMsgsRec() const { return maxMsgsRec; }
    int getSize() const { return size; }
    int getAvgDelay() const { return avgDelay; }
    int getMinDelay() const { return minDelay; }
    int getMaxDelay() const { return maxDelay; }
    DelayStyle getDelayStyle() const { return delayStyle; }

private: 
    double dropProbability{0.0};
    double reorderProbability{0.0};
    double duplicateProbability{0.0};
    int maxMsgsRec{INT_MAX};
    int size{INT_MAX};
    int avgDelay{1};
    int minDelay{1};
    int maxDelay{1};
};

class ChannelPropertiesFactory {
public:
    static ChannelPropertiesFactory &instance() {
        static ChannelPropertiesFactory instance;
        return instance;
    }

    ChannelProperties *create(const json &params) {
        ChannelProperties *newProps = new ChannelProperties(params);
        for (auto prop : _propertiesCache) {
            if (*prop == *newProps) {
                delete newProps;
                return prop;
            }
        }
        _propertiesCache.insert(newProps);
        return newProps;
    }

    ~ChannelPropertiesFactory() {
        for (auto prop : _propertiesCache) {
            delete prop;
        }
        _propertiesCache.clear();
    }

private:
    ChannelPropertiesFactory() = default;
    ChannelPropertiesFactory(const ChannelPropertiesFactory &) = delete;
    ChannelPropertiesFactory &operator=(const ChannelPropertiesFactory &) = delete;

    std::unordered_set<ChannelProperties*, ChannelProperties::Hash> _propertiesCache;
};

class Channel : public std::enable_shared_from_this<Channel> {
private:
    static ChannelPropertiesFactory _propertiesFactory;

    interfaceId _targetId{ NO_PEER_ID };
    interfaceId _targetInternalId{ NO_PEER_ID };

    interfaceId _sourceId{ NO_PEER_ID };
    interfaceId _sourceInternalId{ NO_PEER_ID };

    ChannelProperties* _properties; // properties of this channel
    int    _throughputLeft{INT_MAX};   // throughputLeft, if you want a limited number of sends to reduce the maximum size of the channel

    // These are the packets that have been "sent" by the source side
    // but not yet delivered to the target side.
    deque<Packet> _packetQueue;

    // Helpers
    bool canSend() const { return (_throughputLeft != 0 && (_properties->getSize() > _packetQueue.size())); }
    int computeRandomDelay() const;
    void consumeThroughput() {
        if (_throughputLeft > 0) {
            _throughputLeft--;
        }
    }

public:
    Channel() = default;

    // Create a channel with references to inbound side and outbound side
    Channel(interfaceId targetId, interfaceId targetInternalId,
            interfaceId sourceId, interfaceId sourceInternalId,
            const json &channelParams);
    ~Channel();

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

    int maxMsgsRec() const {return _properties->getMaxMsgsRec();}

    bool frontHasArrived() const {
        if (_packetQueue.empty()) return false;
        return _packetQueue.front().hasArrived();
    }
};
} // end namespace quantas

#endif
