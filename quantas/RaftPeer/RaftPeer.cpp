/*
Copyright 2024

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the
GNU General Public License as published by the Free Software Foundation, either version 3
of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS.
If not, see <https://www.gnu.org/licenses/>.
*/

#include "RaftPeer.hpp"

#include <algorithm>
#include <deque>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../Common/Committee.hpp"
#include "../Common/LogWriter.hpp"
#include "../Common/Packet.hpp"
#include "../Common/RandomUtil.hpp"
#include "../Common/RoundManager.hpp"

namespace quantas {

using std::string;

namespace {

using TxKey = std::pair<interfaceId, int>;

TxKey extractKey(const json& request) {
    interfaceId submitter = request.value("submitterId", NO_PEER_ID);
    int clientSeq = request.value("clientSeq", -1);
    return {submitter, clientSeq};
}

bool keyValid(const TxKey& key) {
    return key.first != NO_PEER_ID && key.second >= 0;
}

json buildRequestMessage(const json& request, interfaceId sender, int term, int consensusId) {
    json msg = {
        {"type", "Consensus"},
        {"consensusId", consensusId},
        {"MessageType", "request"},
        {"request", request},
        {"from_id", sender},
        {"termNum", term}
    };
    return msg;
}

json buildCommitMessage(const json& request,
                        interfaceId sender,
                        int term,
                        int consensusId,
                        int committedRound,
                        int submittedRound) {
    json msg = {
        {"type", "Consensus"},
        {"consensusId", consensusId},
        {"MessageType", "commit"},
        {"request", request},
        {"from_id", sender},
        {"termNum", term},
        {"roundCommitted", committedRound},
        {"roundSubmitted", submittedRound}
    };
    return msg;
}

class RaftConsensus : public Consensus {
public:
    explicit RaftConsensus(Committee* committee);

    void onConsensusMessage(RaftPeer* peer, const json& msg);
    void onClientRequest(RaftPeer* peer, json request);
    void tick(RaftPeer* peer);

    void setTimeoutSpacing(int spacing) { _timeOutSpacing = spacing; }
    void setTimeoutRandom(int random) { _timeOutRandom = random; }
    void setSubmitRate(int submitRate) { _submitRate = submitRate; }
    int leaderChanges() const { return _leaderChanges; }

private:
    void handleRequest(RaftPeer* peer, const json& msg);
    void handleRespond(RaftPeer* peer, const json& msg);
    void handleVote(RaftPeer* peer, const json& msg);
    void handleElect(RaftPeer* peer, const json& msg);
    void handleCommit(RaftPeer* peer, const json& msg);

    void maybeGenerateClientRequest(RaftPeer* peer);
    void tryForwardDeferredRequests(RaftPeer* peer);
    void tryStartReplication(RaftPeer* peer);
    void maybeStartElection(RaftPeer* peer);
    void resetTimer();
    void resetReplicationState(bool requeueInFlight);

    void confirmRequest(RaftPeer* peer, const json& request, int submittedRound);
    void applyCommit(const json& request, int submittedRound, int committedRound, bool skipMetrics);

    void enqueueDeferred(json request);
    void dropDeferredRequest(const TxKey& key);
    void dropPendingRequest(const TxKey& key);

    size_t quorumThreshold() const;
    bool hasQuorum(size_t count) const;

private:
    interfaceId _candidate = NO_PEER_ID;
    interfaceId _leaderId = 0;
    int _term = 0;
    std::vector<interfaceId> _votes;
    std::map<TxKey, std::vector<interfaceId>> _replies;
    std::map<TxKey, int> _submittedRound;
    std::set<TxKey> _committedRequests;
    std::set<TxKey> _knownRequests;

    std::deque<json> _pendingClientRequests;
    std::deque<json> _deferredClientRequests;

    bool _hasInFlight = false;
    TxKey _inFlightKey{NO_PEER_ID, -1};
    json _inFlightRequest = json::object();

    int _leaderChanges = 0;

    int _timeOutRound = 0;
    int _timeOutSpacing = 100;
    int _timeOutRandom = 5;
    bool _initialSubmissionAttempted = false;

    int _submitRate = 20;
    int _nextClientRequestId = 0;
};

RaftConsensus::RaftConsensus(Committee* committee) {
    if (committee == nullptr) {
        throw std::invalid_argument("RaftConsensus requires a valid committee");
    }
    _committee = committee;
    resetTimer();
}

void RaftConsensus::onConsensusMessage(RaftPeer* peer, const json& msg) {
    const string messageType = msg.value("MessageType", string());
    if (messageType == "request") {
        handleRequest(peer, msg);
    } else if (messageType == "respondRequest") {
        handleRespond(peer, msg);
    } else if (messageType == "vote") {
        handleVote(peer, msg);
    } else if (messageType == "elect") {
        handleElect(peer, msg);
    } else if (messageType == "commit") {
        handleCommit(peer, msg);
    }
}

void RaftConsensus::onClientRequest(RaftPeer* peer, json request) {
    if (!request.contains("roundSubmitted")) {
        request["roundSubmitted"] = RoundManager::currentRound();
    }
    if (!request.contains("submitterId")) {
        request["submitterId"] = peer->publicId();
    }
    if (!request.contains("clientSeq")) {
        request["clientSeq"] = _nextClientRequestId++;
    }

    enqueueDeferred(std::move(request));
    tryForwardDeferredRequests(peer);
    if (_leaderId == peer->publicId()) {
        tryStartReplication(peer);
    }
}

void RaftConsensus::tick(RaftPeer* peer) {
    maybeGenerateClientRequest(peer);
    maybeStartElection(peer);
    tryForwardDeferredRequests(peer);
    // tryStartReplication(peer);
}

void RaftConsensus::handleRequest(RaftPeer* peer, const json& msg) {
    const int termNum = msg.value("termNum", -1);
    const interfaceId sender = msg.value("from_id", NO_PEER_ID);
    if (termNum < 0 || sender == NO_PEER_ID) {
        return;
    }

    if (_term <= termNum) {
        _term = termNum;
        const bool leaderChanged = (_leaderId != sender);
        _leaderId = sender;
        if (leaderChanged) {
            resetReplicationState(false);
        }
        _votes.clear();
        _candidate = NO_PEER_ID;
        resetTimer();
    }

    const json request = msg.value("request", json::object());
    TxKey key = extractKey(request);
    if (!keyValid(key)) {
        return;
    }
    enqueueDeferred(request);

    json response = {
        {"type", "Consensus"},
        {"consensusId", getId()},
        {"MessageType", "respondRequest"},
        {"request", request},
        {"from_id", peer->publicId()},
        {"termNum", _term},
        {"roundSubmitted", request.value("roundSubmitted", RoundManager::currentRound())}
    };

    peer->unicastTo(response, sender);
}

void RaftConsensus::handleRespond(RaftPeer* peer, const json& msg) {
    if (_leaderId != peer->publicId()) {
        return;
    }

    const json request = msg.value("request", json::object());
    TxKey key = extractKey(request);
    if (!_hasInFlight || key != _inFlightKey) {
        return;
    }

    const interfaceId sender = msg.value("from_id", NO_PEER_ID);
    if (sender == NO_PEER_ID) {
        return;
    }

    auto& responders = _replies[key];
    if (std::find(responders.begin(), responders.end(), sender) != responders.end()) {
        return;
    }
    responders.push_back(sender);

    if (hasQuorum(responders.size())) {
        const int submittedRound = _submittedRound.count(key)
            ? _submittedRound[key]
            : request.value("roundSubmitted", RoundManager::currentRound());
        confirmRequest(peer, request, submittedRound);
    }
}

void RaftConsensus::handleVote(RaftPeer* peer, const json& msg) {
    if (_leaderId == peer->publicId()) {
        return;
    }

    const interfaceId voteFor = msg.value("trans", NO_PEER_ID);
    const interfaceId sender = msg.value("from_id", NO_PEER_ID);
    if (voteFor != peer->publicId() || sender == NO_PEER_ID) {
        return;
    }

    if (std::find(_votes.begin(), _votes.end(), sender) != _votes.end()) {
        return;
    }

    _votes.push_back(sender);
    if (hasQuorum(_votes.size())) {
        resetTimer();
        _leaderId = peer->publicId();
        _candidate = NO_PEER_ID;
        _votes.clear();
        resetReplicationState(true);
        ++_leaderChanges;
        tryForwardDeferredRequests(peer);
        tryStartReplication(peer);
    }
}

void RaftConsensus::handleElect(RaftPeer* peer, const json& msg) {
    const int termNum = msg.value("termNum", -1);
    const interfaceId sender = msg.value("from_id", NO_PEER_ID);
    if (termNum <= _term || sender == NO_PEER_ID) {
        return;
    }

    _term = termNum;
    _leaderId = NO_PEER_ID;
    _votes.clear();
    _candidate = sender;
    resetReplicationState(true);
    resetTimer();

    json vote = {
        {"type", "Consensus"},
        {"consensusId", getId()},
        {"MessageType", "vote"},
        {"trans", sender},
        {"from_id", peer->publicId()},
        {"termNum", _term},
        {"roundSubmitted", msg.value("roundSubmitted", RoundManager::currentRound())}
    };

    peer->unicastTo(vote, sender);
}

void RaftConsensus::handleCommit(RaftPeer* peer, const json& msg) {
    if (msg.value("from_id", NO_PEER_ID) == peer->publicId()) {
        return;
    }
    const json request = msg.value("request", json::object());
    const int submittedRound = msg.value("roundSubmitted", RoundManager::currentRound());
    const int committedRound = msg.value("roundCommitted", RoundManager::currentRound());
    const int termNum = msg.value("termNum", _term);
    if (termNum > _term) {
        _term = termNum;
    }
    applyCommit(request, submittedRound, committedRound, false);
}

void RaftConsensus::maybeGenerateClientRequest(RaftPeer* peer) {
    if (_submitRate <= 0) {
        return;
    }
    if (randMod(_submitRate) != 0) {
        return;
    }
    json request = {
        {"submitterId", peer->publicId()},
        {"clientSeq", _nextClientRequestId++},
        {"roundSubmitted", RoundManager::currentRound()}
    };
    onClientRequest(peer, std::move(request));
}

void RaftConsensus::tryForwardDeferredRequests(RaftPeer* peer) {
    if (_deferredClientRequests.empty()) {
        return;
    }

    const interfaceId leader = _leaderId;
    const bool isLeader = (leader == peer->publicId());
    const size_t size = _deferredClientRequests.size();
    for (size_t i = 0; i < size; ++i) {
        json request = std::move(_deferredClientRequests.front());
        _deferredClientRequests.pop_front();

        TxKey key = extractKey(request);
        if (!keyValid(key) || _committedRequests.count(key)) {
            continue;
        }

        if (isLeader) {
            request.erase("forwardedTo");
            _pendingClientRequests.push_back(std::move(request));
        } else if (leader != NO_PEER_ID) {
            interfaceId forwardedTo = request.value("forwardedTo", NO_PEER_ID);
            if (forwardedTo != leader) {
                json outbound = request;
                outbound["type"] = "Request";
                outbound["consensusId"] = getId();
                peer->unicastTo(outbound, leader);
                request["forwardedTo"] = leader;
            }
            _deferredClientRequests.push_back(std::move(request));
        } else {
            _deferredClientRequests.push_back(std::move(request));
        }
    }
}

void RaftConsensus::tryStartReplication(RaftPeer* peer) {
    if (_leaderId != peer->publicId()) {
        return;
    }
    if (_hasInFlight) {
        return;
    }
    if (_pendingClientRequests.empty()) {
        json request = {
            {"submitterId", peer->publicId()},
            {"clientSeq", _nextClientRequestId++},
            {"roundSubmitted", RoundManager::currentRound()}
        };
        onClientRequest(peer, std::move(request));
    }

    while (!_pendingClientRequests.empty()) {
        json request = std::move(_pendingClientRequests.front());
        _pendingClientRequests.pop_front();

        TxKey key = extractKey(request);
        if (!keyValid(key) || _committedRequests.count(key)) {
            continue;
        }

        int submitRound = RoundManager::currentRound();
        request["roundSubmitted"] = submitRound;
        _hasInFlight = true;
        _inFlightKey = key;
        _inFlightRequest = request;
        _replies[key];
        _submittedRound[key] = submitRound;

        const json consensusMsg = buildRequestMessage(request, peer->publicId(), _term, getId());
        peer->multicast(consensusMsg, getMembers());
        resetTimer();
        break;
    }
}

void RaftConsensus::maybeStartElection(RaftPeer* peer) {
    if (_timeOutRound > RoundManager::currentRound()) {
        return;
    }

    resetReplicationState(true);
    _candidate = peer->publicId();
    _leaderId = NO_PEER_ID;
    ++_term;
    _votes.clear();
    resetTimer();

    json elect = {
        {"type", "Consensus"},
        {"consensusId", getId()},
        {"MessageType", "elect"},
        {"from_id", peer->publicId()},
        {"termNum", _term},
        {"roundSubmitted", RoundManager::currentRound()}
    };

    peer->multicast(elect, getMembers());
}

void RaftConsensus::resetTimer() {
    const int jitter = (_timeOutRandom > 0) ? randMod(_timeOutRandom) : 0;
    _timeOutRound = RoundManager::currentRound() + _timeOutSpacing + jitter;
}

void RaftConsensus::resetReplicationState(bool requeueInFlight) {
    if (_hasInFlight) {
        TxKey key = _inFlightKey;
        if (requeueInFlight && keyValid(key) && !_committedRequests.count(key)) {
            _pendingClientRequests.push_front(_inFlightRequest);
        }
        if (keyValid(key)) {
            _replies.erase(key);
            _submittedRound.erase(key);
        }
    }
    _hasInFlight = false;
    _inFlightKey = {NO_PEER_ID, -1};
    _inFlightRequest = json::object();
}

void RaftConsensus::confirmRequest(RaftPeer* peer, const json& request, int submittedRound) {
    const int committedRound = RoundManager::currentRound();

    json confirmation = request;
    confirmation["roundSubmitted"] = submittedRound;
    confirmation["roundCommitted"] = committedRound;
    confirmation["term"] = _term;
    _confirmedTrans.push_back(confirmation);
    _latency += committedRound - submittedRound;

    applyCommit(request, submittedRound, committedRound, true);

    const json commitMsg = buildCommitMessage(request,
                                              peer->publicId(),
                                              _term,
                                              getId(),
                                              committedRound,
                                              submittedRound);
    peer->multicast(commitMsg, getMembers());

    _hasInFlight = false;
    _inFlightKey = {NO_PEER_ID, -1};
    _inFlightRequest = json::object();
    tryStartReplication(peer);
}

void RaftConsensus::applyCommit(const json& request,
                                int submittedRound,
                                int committedRound,
                                bool skipMetrics) {
    TxKey key = extractKey(request);
    if (!keyValid(key)) {
        return;
    }

    _committedRequests.insert(key);
    _submittedRound.erase(key);
    _replies.erase(key);
    dropDeferredRequest(key);
    dropPendingRequest(key);

    if (!skipMetrics) {
        json confirmation = request;
        confirmation["roundSubmitted"] = submittedRound;
        confirmation["roundCommitted"] = committedRound;
        confirmation["term"] = _term;
        _confirmedTrans.push_back(std::move(confirmation));
        _latency += committedRound - submittedRound;
    }
}

void RaftConsensus::enqueueDeferred(json request) {
    TxKey key = extractKey(request);
    if (!keyValid(key)) {
        return;
    }
    if (!_knownRequests.insert(key).second) {
        return;
    }
    _deferredClientRequests.push_back(std::move(request));
}

void RaftConsensus::dropDeferredRequest(const TxKey& key) {
    if (_deferredClientRequests.empty()) {
        return;
    }
    std::deque<json> tmp;
    while (!_deferredClientRequests.empty()) {
        json request = std::move(_deferredClientRequests.front());
        _deferredClientRequests.pop_front();
        if (extractKey(request) == key) {
            continue;
        }
        tmp.push_back(std::move(request));
    }
    _deferredClientRequests = std::move(tmp);
}

void RaftConsensus::dropPendingRequest(const TxKey& key) {
    if (_pendingClientRequests.empty()) {
        return;
    }
    std::deque<json> tmp;
    while (!_pendingClientRequests.empty()) {
        json request = std::move(_pendingClientRequests.front());
        _pendingClientRequests.pop_front();
        if (extractKey(request) == key) {
            continue;
        }
        tmp.push_back(std::move(request));
    }
    _pendingClientRequests = std::move(tmp);
}

size_t RaftConsensus::quorumThreshold() const {
    const auto& members = getMembers();
    if (members.empty()) {
        return 0;
    }
    return members.size() / 2;
}

bool RaftConsensus::hasQuorum(size_t count) const {
    return count > quorumThreshold();
}

} // namespace

static bool registerRaft = []() {
    PeerRegistry::registerPeerType(
        "RaftPeer",
        [](interfaceId pubId) { return new RaftPeer(new NetworkInterfaceAbstract(pubId)); }
    );
    return true;
}();

RaftPeer::~RaftPeer() = default;

RaftPeer::RaftPeer(const RaftPeer& rhs) : ConsensusPeer(rhs) {}

RaftPeer::RaftPeer(NetworkInterface* networkInterface) : ConsensusPeer(networkInterface) {}

void RaftPeer::maybeCrash() {
    if (_crashOdds <= 0.0 || _crashRecoveryDelay == 0) {
        return;
    }

    if (trueWithProbability(_crashOdds)) {
        setCrashRecoveryRound(RoundManager::currentRound() + _crashRecoveryDelay);
    }
}

void RaftPeer::performComputation() {
    maybeCrash();
    if (isCrashed()) {
        return;
    }

    while (!inStreamEmpty()) {
        Packet packet = popInStream();
        json msg = packet.getMessage();

        if (!msg.contains("type")) {
            continue;
        }

        const string msgType = msg["type"];
        const int consensusId = msg.value("consensusId", 0);

        if (msgType == "Request") {
            auto it = consensuses.find(consensusId);
            if (it != consensuses.end()) {
                if (auto* raft = dynamic_cast<RaftConsensus*>(it->second)) {
                    raft->onClientRequest(this, msg);
                } else {
                    it->second->_unhandledRequests.insert({RoundManager::currentRound(), msg});
                }
            }
        } else if (msgType == "Consensus") {
            auto it = consensuses.find(consensusId);
            if (it == consensuses.end()) {
                continue;
            }
            if (auto* raft = dynamic_cast<RaftConsensus*>(it->second)) {
                raft->onConsensusMessage(this, msg);
            }
        }
    }

    for (auto& entry : consensuses) {
        if (auto* raft = dynamic_cast<RaftConsensus*>(entry.second)) {
            raft->tick(this);
        }
    }
}

void RaftPeer::initParameters(const std::vector<Peer*>& peers, json parameters) {
    const vector<RaftPeer*> raftPeers = reinterpret_cast<vector<RaftPeer*> const&>(peers);

    const int committeeId = parameters.value("committee_id", 0);
    const int crashCount = parameters.value("crash_count", 0);
    const double requestedCrashOdds = parameters.value("crash_odds", 0.0);
    const int submitRate = parameters.value("submit_rate", 20);
    const int timeoutSpacing = parameters.value("timeout_spacing", 100);
    const int timeoutJitter = parameters.value("timeout_jitter", 5);

    size_t crashRecoveryRound = std::numeric_limits<size_t>::max();
    if (parameters.contains("crash_recovery_round")) {
        crashRecoveryRound = parameters["crash_recovery_round"].get<size_t>();
    }

    size_t crashRecoveryDelay = 0;
    if (parameters.contains("crash_recovery_delay")) {
        crashRecoveryDelay = parameters["crash_recovery_delay"].get<size_t>();
    }

    Committee baseCommittee(committeeId);
    for (auto* peer : raftPeers) {
        baseCommittee.addMember(peer->publicId());
        peer->_crashRecoveryDelay = crashRecoveryDelay;
        peer->_crashOdds = std::clamp(requestedCrashOdds, 0.0, 1.0);
    }

    for (auto* peer : raftPeers) {
        auto* committeeCopy = new Committee(baseCommittee);
        auto* consensus = new RaftConsensus(committeeCopy);
        consensus->setSubmitRate(submitRate);
        consensus->setTimeoutSpacing(timeoutSpacing);
        consensus->setTimeoutRandom(timeoutJitter);
        peer->consensuses[committeeId] = consensus;
    }

    if (crashCount > 0) {
        std::vector<RaftPeer*> sorted = raftPeers;
        std::sort(sorted.begin(), sorted.end(), [](RaftPeer* lhs, RaftPeer* rhs) {
            return lhs->publicId() < rhs->publicId();
        });

        const size_t limit = std::min(sorted.size(), static_cast<size_t>(crashCount));
        for (size_t i = 0; i < limit; ++i) {
            sorted[i]->setCrashRecoveryRound(crashRecoveryRound);
        }
    }
}

void RaftPeer::endOfRound(std::vector<Peer*>& peers) {
    double totalConfirmed = 0.0;
    double totalLatency = 0.0;
    int totalLeaderChanges = 0;

    for (Peer* base : peers) {
        if (auto* raftPeer = dynamic_cast<RaftPeer*>(base)) {
            for (auto& entry : raftPeer->consensuses) {
                if (auto* consensus = dynamic_cast<RaftConsensus*>(entry.second)) {
                    totalConfirmed += static_cast<double>(consensus->_confirmedTrans.size());
                    totalLatency += static_cast<double>(consensus->_latency);
                    totalLeaderChanges += consensus->leaderChanges();
                }
            }
        }
    }

    if (totalConfirmed > 0.0) {
        LogWriter::pushValue("latency", totalLatency / totalConfirmed);
    } else {
        LogWriter::pushValue("latency", 0.0);
    }

    if (!peers.empty()) {
        LogWriter::pushValue("throughput", totalConfirmed / peers.size());
    } else {
        LogWriter::pushValue("throughput", 0.0);
    }

    LogWriter::pushValue("leaderChanges", totalLeaderChanges);
}

} // namespace quantas
