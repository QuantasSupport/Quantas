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

void Channel::setParameters(const nlohmann::json &params) {
    _dropProbability = params.value("dropProbability", 0.0);
    _reorderProbability = params.value("reorderProbability", 0.0);
    _duplicateProbability = params.value("duplicateProbability", 0.0);
    _maxMsgsRec = params.value("dropProbability", INT_MAX);
    _throughputLeft = _maxMsgsRec*(RoundManager::instance()->lastRound()+1-RoundManager::instance()->currentRound());
    _size = params.value("size", INT_MAX);
    _avgDelay = params.value("avgDelay", 1);
    _minDelay = params.value("minDelay", 1);
    _maxDelay = params.value("maxDelay", 1);
    std::string t = params.value("type", "UNIFORM");
    if (t == "UNIFORM")  _delayStyle = DelayStyle::DS_UNIFORM;
    else if (t == "POISSON") _delayStyle = DelayStyle::DS_POISSON;
    else if (t == "ONE") _delayStyle = DelayStyle::DS_ONE;
}

int Channel::computeRandomDelay() const {
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

void Channel::pushPacket(Packet pkt) {
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

void Channel::shuffleChannel() {
    // reorder
    if (_packetQueue.size() > 1 && trueWithProbability(_reorderProbability)) {
        std::shuffle(_packetQueue.begin(), _packetQueue.end(), threadLocalEngine());
    }
}

Packet Channel::popPacket() {
    Packet p = std::move(_packetQueue.front());
    _packetQueue.pop_front();
    return p;
}

} // end namespace quantas