#include "Channel.hpp"

 
namespace quantas {

Channel::Channel(interfaceId targetId, interfaceId targetInternalId,
                        interfaceId sourceId, interfaceId sourceInternalId,
                        const nlohmann::json &channelParams)
: _targetId(targetId),
  _targetInternalId(targetInternalId),
  _sourceId(sourceId),
  _sourceInternalId(sourceInternalId)
{
    setParameters(channelParams);
}

Channel::~Channel() {
    while (!_packetQueue.empty()) {
        _packetQueue.pop_front();
    }
};

void Channel::setParameters(const nlohmann::json &params) {
    _properties = ChannelPropertiesFactory::instance().create(params);
    _throughputLeft = _properties->getMaxMsgsRec()*(RoundManager::lastRound()-RoundManager::currentRound());
}

int Channel::computeRandomDelay() const {
    int delay = 1;
    switch (_properties->getDelayStyle()) {
    case DelayStyle::DS_UNIFORM:
        delay = uniformInt(_properties->getMinDelay(), _properties->getMaxDelay());
        break;
    case DelayStyle::DS_POISSON:
        delay = poissonInt(_properties->getAvgDelay());
        delay = std::clamp(delay, _properties->getMinDelay(), _properties->getMaxDelay());
        break;
    case DelayStyle::DS_ONE:
        delay = 1;
        break;
    }
    return delay;
}

void Channel::pushPacket(Packet pkt) {
    // possible drop
    if (trueWithProbability(_properties->getDropProbability())) {
        return;
    }

    bool duplicate = false;

    do {
        duplicate = false;
        if (!canSend()) {
            return;
        }

        consumeThroughput();
        int d = computeRandomDelay();
        pkt.setDelay(d, d);
        _packetQueue.push_back(pkt);

        duplicate = trueWithProbability(_properties->getDuplicateProbability());
        if (duplicate) pkt.setMessage(pkt.getMessage());

    } while (duplicate);
}

void Channel::shuffleChannel() {
    // reorder
    if (_packetQueue.size() > 1 && trueWithProbability(_properties->getReorderProbability())) {
        std::shuffle(_packetQueue.begin(), _packetQueue.end(), threadLocalEngine());
    }
}

Packet Channel::popPacket() {
    Packet p = std::move(_packetQueue.front());
    _packetQueue.pop_front();
    return p;
}

} // end namespace quantas