/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <functional>
#include <sstream>
#include "PBFTPeer.hpp"
#include "../Common/Faults/EquivocateFault.hpp"

namespace quantas {

static bool registerPBFT = [](){
	PeerRegistry::registerPeerType("PBFTPeer", 
		[](interfaceId pubId){ return new PBFTPeer(new NetworkInterfaceAbstract(pubId)); });
	return true;
}();

class PBFTConsensus : public Consensus {
public:
    PBFTConsensus(Committee* committee);

    ~PBFTConsensus() {};

    int view = 0;
    int viewChangeTimer = 10;
    int viewChangeDelay = 10;
    int checkpointInterval = 10;
    int lowWaterMark = 0;
    int highWaterMark = -1;
    const int WINDOW = 128; // or a parameter for watermarks
    int lastStableCheckpoint = 0;
    int viewChangeAnchorSeq = 0;  // seq key to anchor VC/NV
    int progressSeq = -1;
    int progressView = -1;
    std::string progressDigest;
    int progressStage = 0;
    int progressCount = 0;
    // seqNum, view
    map<int, map<int, multimap<string, json>>> _receivedMessages;

    void submitRequest(Peer* peer) {
        json msg = {
            {"type", "Request"},
            {"requestId", _currentClientRequestId++},
            {"submitterId", peer->publicId()},
            {"consensusId", getId()},
            {"roundSubmitted", RoundManager::currentRound()},
            {"fault_flip", false}
        };
        
        peer->multicast(msg, getMembers());
        _unhandledRequests.insert({RoundManager::currentRound(),msg});
    }

    void sendCheckpoint(Peer* peer);
    void maybeStableCheckpoint(Peer* peer);
    void requestViewChange(Peer* peer);
    bool installCommittedPrefix(Peer* peer);
    void observeProgress(int candidateView, int candidateSeq, const std::string& digest, int stage, int count);

    int f() const { return (int)(_committee->size()-1)/3; }
    int quorum() const { return 2*f(); }        // # of msgs > 2f to move on

    // Deterministic leader for a given view:
    interfaceId leaderFor(int v) const {
        const auto& S = getMembers();           // std::set<interfaceId>
        if (S.empty()) return NO_PEER_ID;
        size_t idx = (size_t)v % S.size();
        auto it = S.begin();
        std::advance(it, idx);
        return *it;
    }

    // Digest for the client request (or proposal payload)
    std::string digestOf(const json& request) const {
        // Stable enough for the simulator; replace with real hashing if available.
        return std::to_string(std::hash<std::string>{}(request.dump()));
    }

    // Simple state digest: hash concat of confirmed requests’ dumps
    std::string computeStateDigest() const {
        std::ostringstream oss;
        for (const auto& r : _confirmedTrans) oss << r.dump() << '|';
        return std::to_string(std::hash<std::string>{}(oss.str()));
    }

    struct CertKey { int view; int seq; std::string digest; };

    bool hasPrePrepare(int v, int n, const std::string& d) {
        auto &mm = _receivedMessages[n][v];
        auto range = mm.equal_range("pre-prepare");
        for (auto it=range.first; it!=range.second; ++it) {
            const auto& m = it->second;
            if (m.contains("proposal") &&
                m["proposal"].contains("view") && (int)m["proposal"]["view"]==v &&
                m["seqNum"]==n &&
                digestOf(m["proposal"]["Request"])==d &&
                m["from_id"]==leaderFor(v)) {
                return true;
            }
        }
        return false;
    }

    int countPrepares(int v, int n, const std::string& d) {
        auto &mm = _receivedMessages[n][v];
        auto range = mm.equal_range("prepare");
        std::set<interfaceId> senders;
        for (auto it=range.first; it!=range.second; ++it) {
            const auto& m = it->second;
            if (!m.contains("proposal")) continue;
            if (!m["proposal"].contains("Request")) continue;
            if ((int)m["proposal"]["view"]!=v) continue;
            if (m["seqNum"]!=n) continue;
            if (digestOf(m["proposal"]["Request"]) != d) continue;
            senders.insert(m["from_id"].get<interfaceId>());
        }

        // if (who) *who = std::move(senders);
        // add option who = nullptr if I want to know who the senders were
        return (int)senders.size();
    }

    int countCommits(int v, int n, const std::string& d, std::set<interfaceId>* who=nullptr) {
        auto &mm = _receivedMessages[n][v];
        auto range = mm.equal_range("commit");
        std::set<interfaceId> senders;
        for (auto it=range.first; it!=range.second; ++it) {
            const auto& m = it->second;
            if (!m.contains("proposal")) continue;
            if (!m["proposal"].contains("Request")) continue;
            if ((int)m["proposal"]["view"]!=v) continue;
            if (m["seqNum"]!=n) continue;
            if (digestOf(m["proposal"]["Request"]) != d) continue;
            senders.insert(m["from_id"].get<interfaceId>());
        }
        if (who) *who = std::move(senders);
        return (int)senders.size();
    }

    // Prepared certificate in v,n for digest d
    bool isPrepared(int v, int n, const std::string& d) {
        // PBFT prepared requires the pre-prepare plus 2f matching prepares
        // from distinct backups, not 2f+1 prepare messages.
        return hasPrePrepare(v,n,d) && countPrepares(v,n,d) >= quorum();
    }

    // Commit certificate in v,n for d
    bool isCommitted(int v, int n, const std::string& d) {
        return countCommits(v,n,d) > quorum();
    }

    // Choose next sequence number and cache last proposal digest
    // Sequence numbers start at 1 so the first live request is not confused
    // with the initial stable checkpoint at sequence 0 during view changes.
    int nextSeq() const { return (int)_confirmedTrans.size() + 1; }

    // Stable checkpoint check: 2f+1 matching digests at seq multiple of interval
    bool stableCheckpointReady(int seq, std::string* dig=nullptr) {
        if (seq % checkpointInterval != 0) return false;
        auto &mm = _receivedMessages[seq][view];
        auto range = mm.equal_range("checkpoint");
        std::map<std::string,int> tally;
        for (auto it=range.first; it!=range.second; ++it) {
            auto d = it->second.value("digest","__");
            tally[d]++;
            if (tally[d] > quorum()) { if (dig) *dig = d; return true; }
        }
        return false;
    }
};

bool PBFTConsensus::installCommittedPrefix(Peer* peer) {
    bool changed = false;

    while (true) {
        const int targetSeq = static_cast<int>(_confirmedTrans.size()) + 1;
        int bestView = -1;
        json committedReq;

        auto seqIt = _receivedMessages.find(targetSeq);
        if (seqIt == _receivedMessages.end()) {
            break;
        }

        for (const auto& viewEntry : seqIt->second) {
            const int candidateView = viewEntry.first;
            const auto& mm = viewEntry.second;
            auto ppRange = mm.equal_range("pre-prepare");
            for (auto it = ppRange.first; it != ppRange.second; ++it) {
                const auto& pp = it->second;
                if ((int)pp["view"] != candidateView) continue;
                if (pp["from_id"] != leaderFor(candidateView)) continue;
                if (!pp.contains("proposal") || !pp["proposal"].contains("Request")) continue;
                if (pp["seqNum"] != targetSeq) continue;

                const auto& req = pp["proposal"]["Request"];
                const auto digest = digestOf(req);
                if (!isCommitted(candidateView, targetSeq, digest)) continue;

                if (candidateView > bestView) {
                    bestView = candidateView;
                    committedReq = req;
                }
            }
        }

        if (bestView < 0 || committedReq.empty()) {
            break;
        }

        _confirmedTrans.push_back(committedReq);
        _latency += RoundManager::currentRound() - committedReq["roundSubmitted"].get<int>();
        viewChangeTimer = RoundManager::currentRound() + viewChangeDelay;
        changed = true;

        QUANTAS_LOG_TRACE("PBFT") << peer->publicId()
            << " install committed prefix seq " << targetSeq
            << " from view " << bestView
            << " at round " << RoundManager::currentRound();

        for (auto it = _unhandledRequests.begin(); it != _unhandledRequests.end(); ++it) {
            if (it->second == committedReq) {
                _unhandledRequests.erase(it);
                break;
            }
        }

        sendCheckpoint(peer);
    }

    return changed;
}

void PBFTConsensus::observeProgress(int candidateView, int candidateSeq, const std::string& digest, int stage, int count) {
    if (count <= 0) {
        return;
    }

    const bool changedRequest =
        candidateSeq != progressSeq || candidateView != progressView || digest != progressDigest;
    const bool advancedWithinRequest =
        stage > progressStage || (stage == progressStage && count > progressCount);

    if (changedRequest || advancedWithinRequest) {
        progressSeq = candidateSeq;
        progressView = candidateView;
        progressDigest = digest;
        progressStage = stage;
        progressCount = count;
        viewChangeTimer = RoundManager::currentRound() + viewChangeDelay;
    }
}

// Called periodically (e.g. after commit) to multicast a checkpoint
void PBFTConsensus::sendCheckpoint(Peer* peer) {
    int seqNum = _confirmedTrans.size();
    if (seqNum % checkpointInterval != 0) return;

    // 1) Compute a digest of your application state at this point
    std::string digest = computeStateDigest(); 

    json msg = {
        {"type","Consensus"},{"consensusId",getId()},
        {"MessageType","checkpoint"},
        {"seqNum", seqNum},
        {"digest", digest},
        {"from_id", peer->publicId()},
        {"view", view}
    };
    peer->multicast(msg, getMembers());
    _receivedMessages[seqNum][view].insert({ "checkpoint", msg});
}

// Once enough matching checkpoints arrive, advance the stable checkpoint
void PBFTConsensus::maybeStableCheckpoint(Peer* peer) {
    int n = _confirmedTrans.size();
    if (n % checkpointInterval != 0) return;

    std::string dig;
    if (!stableCheckpointReady(n, &dig)) return;

    // advance stable checkpoint
    lastStableCheckpoint = n;
    lowWaterMark  = lastStableCheckpoint;
    highWaterMark = lowWaterMark + WINDOW; // maintain a window

    // prune logs below lowWaterMark
    for (auto it=_receivedMessages.begin(); it!=_receivedMessages.end();) {
        if (it->first < lowWaterMark) it = _receivedMessages.erase(it);
        else ++it;
    }
}


class PBFTPrePreparePhase : public Phase {
public:
    static Phase* instance() {
        static PBFTPrePreparePhase instance;
        return &instance;
    }

    void runPhase(Consensus* con, Peer* peer) override;
};

class PBFTPreparePhase : public Phase {
public:
    static Phase* instance() {
        static PBFTPreparePhase instance;
        return &instance;
    }

    void runPhase(Consensus* con, Peer* peer) override;
};

class PBFTCommitPhase : public Phase {
public:
    static Phase* instance() {
        static PBFTCommitPhase instance;
        return &instance;
    }

    void runPhase(Consensus* con, Peer* peer) override;
};

class PBFTViewChangePhase : public Phase {
public:
    static Phase* instance() {
        static PBFTViewChangePhase instance;
        return &instance;
    }

    void runPhase(Consensus* con, Peer* peer) override;
};

class PBFTNewViewPhase : public Phase {
public:
    static Phase* instance() {
        static PBFTNewViewPhase _inst;
        return &_inst;
    }

    void runPhase(Consensus* con, Peer* peer) override;
};

void PBFTPeer::performComputation() {

    while (!inStreamEmpty()) {
        Packet packet = popInStream();
        json msg = packet.getMessage();
        
        if (!msg.contains("type")) {
            QUANTAS_LOG_WARN("PBFT") << "Message requires a type";
            continue;
        }
        if (!msg.contains("consensusId")) {
            QUANTAS_LOG_WARN("PBFT") << "Message requires a consensusId";
            continue;
        }
        if (msg["type"] == "Request") {
            int targetId = msg["consensusId"];
            auto it = consensuses.find(targetId);
            if (it != consensuses.end()) {
                Consensus* target = it->second;
                target->_unhandledRequests.insert({RoundManager::currentRound(), msg});
            }
        } else if (msg["type"] == "Consensus") {
            int targetId = msg["consensusId"];
            auto it = consensuses.find(targetId);
            if (it != consensuses.end()) {
                if (!msg.contains("seqNum")) {
                    QUANTAS_LOG_WARN("PBFT") << "Message requires a seqNum";
                    continue;
                }
                int seq = msg["seqNum"];
                if (!msg.contains("view")) {
                    QUANTAS_LOG_WARN("PBFT") << "Message requires a view";
                    continue;
                }
                int view = msg["view"];
                if (!msg.contains("MessageType")) {
                    QUANTAS_LOG_WARN("PBFT") << "Message requires a MessageType";
                    continue;
                }
                string type = msg["MessageType"];
                Consensus* base = it->second;
                auto* target = dynamic_cast<PBFTConsensus*>(base);
                if (!target) { QUANTAS_LOG_WARN("PBFT") << "Message lost"; continue; }
                target->_receivedMessages[seq][view].insert({type, msg});

                QUANTAS_LOG_TRACE("PBFT") << publicId() << " receive " << type << " in round " << RoundManager::currentRound();
            } else {
                QUANTAS_LOG_WARN("PBFT") << "Message lost";
            }
        } else {
            QUANTAS_LOG_WARN("PBFT") << "Other error";
        }
    }

    for (auto consensus : consensuses) {
        if (auto* pbft = dynamic_cast<PBFTConsensus*>(consensus.second)) {
            if (pbft->installCommittedPrefix(this)) {
                pbft->_phase = PBFTPrePreparePhase::instance();
            }
        }
        consensus.second->runPhase(this);
    }
};


PBFTConsensus::PBFTConsensus(Committee* committee) {
    _committee = committee;
    _phase = PBFTPrePreparePhase::instance();
    lowWaterMark = lastStableCheckpoint;
    highWaterMark = lowWaterMark + WINDOW;
}

void PBFTConsensus::requestViewChange(Peer* peer) {
    int oldView = view;

    viewChangeAnchorSeq = lastStableCheckpoint;

    view++;
    progressSeq = -1;
    progressView = -1;
    progressDigest.clear();
    progressStage = 0;
    progressCount = 0;

    std::vector<json> preparedProof;
    for (int s=lastStableCheckpoint+1; s<nextSeq(); ++s) {
        const json* bestPrepared = nullptr;
        int bestPreparedView = -1;

        auto seqIt = _receivedMessages.find(s);
        if (seqIt == _receivedMessages.end()) {
            continue;
        }

        for (const auto& viewEntry : seqIt->second) {
            const int candidateView = viewEntry.first;
            if (candidateView > oldView) continue;

            const auto& mm = viewEntry.second;
            auto ppr = mm.equal_range("pre-prepare");
            for (auto it = ppr.first; it != ppr.second; ++it) {
                const auto& pp = it->second;
                if ((int)pp["view"] != candidateView) continue;
                if (!pp.contains("proposal") || !pp["proposal"].contains("Request")) continue;

                auto d = digestOf(pp["proposal"]["Request"]);
                if (!isPrepared(candidateView, s, d)) continue;

                if (candidateView > bestPreparedView) {
                    bestPrepared = &pp;
                    bestPreparedView = candidateView;
                }
            }
        }

        if (bestPrepared) {
            preparedProof.push_back(*bestPrepared);
        }
    }

    json vc = {
        {"type","Consensus"},
        {"consensusId",getId()},
        {"MessageType","viewChange"},
        {"seqNum", viewChangeAnchorSeq},   // <<<<<< required
        {"newView", view},
        {"view", oldView},
        {"lastStable", lastStableCheckpoint},
        {"confirmedPrefix", _confirmedTrans},
        {"preparedProof", preparedProof},
        {"from_id", peer->publicId()}
    };
    peer->multicast(vc, getMembers());
    _receivedMessages[viewChangeAnchorSeq][oldView].insert({"viewChange", vc});
    
    // Update view change timer since we have requested to move to the next view
    viewChangeTimer = RoundManager::currentRound() + viewChangeDelay;

    QUANTAS_LOG_TRACE("PBFT") << peer->publicId() << " request view change in round " << RoundManager::currentRound();
    QUANTAS_LOG_TRACE("PBFT") << "seqNum: " << viewChangeAnchorSeq << " newView: " << view;

    Phase* nextPhase = PBFTViewChangePhase::instance();
    changePhase(nextPhase);
}

void PBFTPrePreparePhase::runPhase(Consensus* con, Peer* peer) {
    PBFTConsensus* c = dynamic_cast<PBFTConsensus*>(con);
    if (!c) return;

    c->maybeStableCheckpoint(peer);

    if (c->viewChangeTimer < RoundManager::currentRound()) {
        QUANTAS_LOG_TRACE("PBFT") << peer->publicId()
            << " timeout in pre-prepare at round " << RoundManager::currentRound()
            << " view " << c->view
            << " nextSeq " << c->nextSeq()
            << " confirmed " << c->_confirmedTrans.size();
        c->requestViewChange(peer);
        return;
    }

    int n = c->nextSeq();
    // Leader creates pre-prepare
    if (peer->publicId() == c->leaderFor(c->view)) {
        if (c->_unhandledRequests.empty()) {
            c->submitRequest(peer);
        }

        auto tx = c->_unhandledRequests.begin()->second;
        int n = c->nextSeq();

         json msg = {
            {"type","Consensus"},
            {"consensusId",c->getId()},
            {"MessageType","pre-prepare"},
            {"seqNum", n},
            {"view", c->view},
            {"proposal", {
                {"Request", tx},
                {"view", c->view}
            }},
            {"from_id", peer->publicId()}
        };
        
        peer->multicast(msg, c->getMembers());
        c->_receivedMessages[n][c->view].insert({"pre-prepare", msg});

        changePhase(c, PBFTPreparePhase::instance());
        // no need to run phase as it was just started
    } else {
        if (c->_receivedMessages.find(n) == c->_receivedMessages.end()) {
            return;
        }
        int n = c->nextSeq();
        auto &mm = c->_receivedMessages[n][c->view];
        auto range = mm.equal_range("pre-prepare");
        json pp;

        for (auto it=range.first; it!=range.second; ++it) {
            const auto& m = it->second;
            if ((int)m["view"]==c->view &&
                m["from_id"]==c->leaderFor(c->view) &&
                m["seqNum"]==n &&
                m.contains("proposal") && m["proposal"].contains("Request")) {
                pp = m; break;
            }
        }
        if (pp.empty()) return;

        c->observeProgress(c->view, n, c->digestOf(pp["proposal"]["Request"]), 1, 1);

        // Send PREPARE matching pp
        json prep = pp; 
        prep["MessageType"]="prepare";
        prep["from_id"]=peer->publicId();
        peer->multicast(prep, c->getMembers());
        c->_receivedMessages[n][c->view].insert({"prepare", prep});

        changePhase(c, PBFTPreparePhase::instance());
        c->runPhase(peer);
    }
}

void PBFTPreparePhase::runPhase(Consensus* con, Peer* peer) {
    auto* c = dynamic_cast<PBFTConsensus*>(con);
    if (!c) return;
    c->maybeStableCheckpoint(peer);

    if (c->viewChangeTimer < RoundManager::currentRound()) {
        QUANTAS_LOG_TRACE("PBFT") << peer->publicId()
            << " timeout in prepare at round " << RoundManager::currentRound()
            << " view " << c->view
            << " nextSeq " << c->nextSeq()
            << " confirmed " << c->_confirmedTrans.size();
        c->requestViewChange(peer);
        return;
    }

    int n = c->nextSeq();
    // Find the pre-prepare digest we’re tracking
    auto &mm = c->_receivedMessages[n][c->view];
    auto ppRange = mm.equal_range("pre-prepare");
    for (auto it=ppRange.first; it!=ppRange.second; ++it) {
        const auto& pp = it->second;
        if ((int)pp["view"]!=c->view) continue;
        // if (pp["from_id"]!=c->leaderFor(c->view)) continue;
        if (!pp.contains("proposal") || !pp["proposal"].contains("Request")) continue;
        auto d = c->digestOf(pp["proposal"]["Request"]);
        c->observeProgress(c->view, n, d, 2, c->countPrepares(c->view, n, d));
        // digest might not match for leader if they are in the wrong group
        if (!c->isPrepared(c->view, n, d)) continue;

        // Prepared → send COMMIT
        json commit = pp;
        commit["MessageType"]="commit";
        commit["from_id"]=peer->publicId();
        peer->multicast(commit, c->getMembers());
        c->_receivedMessages[n][c->view].insert({"commit", commit});

        changePhase(c, PBFTCommitPhase::instance());
        c->runPhase(peer);
        return;
    }
}

void PBFTCommitPhase::runPhase(Consensus* con, Peer* peer) {
    auto* c = dynamic_cast<PBFTConsensus*>(con);
    if (!c) return;
    c->maybeStableCheckpoint(peer);

    if (c->viewChangeTimer < RoundManager::currentRound()) {
        QUANTAS_LOG_TRACE("PBFT") << peer->publicId()
            << " timeout in commit at round " << RoundManager::currentRound()
            << " view " << c->view
            << " nextSeq " << c->nextSeq()
            << " confirmed " << c->_confirmedTrans.size();
        c->requestViewChange(peer);
        return;
    }

    int n = c->nextSeq();
    auto &mm = c->_receivedMessages[n][c->view];
    auto ppRange = mm.equal_range("pre-prepare");
    for (auto it=ppRange.first; it!=ppRange.second; ++it) {
        const auto& pp = it->second;
        if ((int)pp["view"]!=c->view) continue;
        if (pp["from_id"]!=c->leaderFor(c->view)) continue;
        if (!pp.contains("proposal") || !pp["proposal"].contains("Request")) continue;
        auto req = pp["proposal"]["Request"];
        auto d = c->digestOf(req);
        c->observeProgress(c->view, n, d, 3, c->countCommits(c->view, n, d));
        if (!c->isCommitted(c->view, n, d)) continue;

        // Decide
        c->_confirmedTrans.push_back(req);
        c->_latency += RoundManager::currentRound() - req["roundSubmitted"].get<int>();
        c->viewChangeTimer = RoundManager::currentRound() + c->viewChangeDelay;

        QUANTAS_LOG_TRACE("PBFT") << peer->publicId()
            << " commit seq " << n
            << " in view " << c->view
            << " at round " << RoundManager::currentRound()
            << " confirmed " << c->_confirmedTrans.size();

        // Checkpointing at interval
        c->sendCheckpoint(peer);

        // Remove from unhandled (exact match)
        for (auto it2=c->_unhandledRequests.begin(); it2!=c->_unhandledRequests.end(); ++it2) {
            if (it2->second == req) { c->_unhandledRequests.erase(it2); break; }
        }

        // Move back to pre-prepare for next request
        changePhase(c, PBFTPrePreparePhase::instance());
        c->runPhase(peer);
        return;
    }
}

void PBFTViewChangePhase::runPhase(Consensus* con, Peer* peer) {
    auto* c = dynamic_cast<PBFTConsensus*>(con); if (!c) return;
    c->maybeStableCheckpoint(peer);

    if (RoundManager::currentRound() >= c->viewChangeTimer) {
        QUANTAS_LOG_TRACE("PBFT") << peer->publicId()
            << " timeout while waiting for new view at round " << RoundManager::currentRound()
            << " current view " << c->view
            << " anchor " << c->viewChangeAnchorSeq;
        c->requestViewChange(peer);
        return;
    }

    // Count unique senders of viewChange(oldView → c->view)
    std::set<interfaceId> vcSenders;
    auto &mm = c->_receivedMessages[c->viewChangeAnchorSeq][c->view-1];
    auto r = mm.equal_range("viewChange");
    for (auto it=r.first; it!=r.second; ++it) {
        const auto& m = it->second;
        if ((int)m["newView"]==c->view) vcSenders.insert(m["from_id"].get<interfaceId>());
    }
    if ((int)vcSenders.size() <= c->quorum()) return;

    if (peer->publicId() == c->leaderFor(c->view)) {
        std::vector<json> prepared;
        std::vector<json> confirmedPrefix;
        auto &mmOld = c->_receivedMessages[c->viewChangeAnchorSeq][c->view-1];
        auto r = mmOld.equal_range("viewChange");
        for (auto it=r.first; it!=r.second; ++it) {
            const auto& m = it->second;
            if ((int)m["newView"] != c->view) continue;
            if (!m.contains("confirmedPrefix") || !m["confirmedPrefix"].is_array()) continue;
            auto candidate = m["confirmedPrefix"].get<std::vector<json>>();
            if (candidate.size() > confirmedPrefix.size()) {
                confirmedPrefix = std::move(candidate);
            }
        }
        for (int n = c->lastStableCheckpoint + 1; n <= c->highWaterMark; ++n) {
            const json* best = nullptr; int bestV = -1;
            for (auto it=r.first; it!=r.second; ++it) {
                const auto& m = it->second;
                if ((int)m["newView"] != c->view) continue;
                for (const auto& pp : m["preparedProof"]) {
                    if (pp["seqNum"] == n) {
                        int pv = pp["proposal"]["view"];
                        if (pv > bestV) { best = &pp; bestV = pv; }
                    }
                }
            }
            if (best) prepared.push_back(*best);
        }

        json nv = {
            {"type","Consensus"},
            {"consensusId", c->getId()},
            {"MessageType","newView"},
            {"seqNum", c->viewChangeAnchorSeq},        // <<<<<< required
            {"view", c->view},                         // new view
            {"lastStable", c->lastStableCheckpoint},
            {"confirmedPrefix", confirmedPrefix},
            {"preparedProof", prepared},
            {"from_id", peer->publicId()}
        };

        peer->multicast(nv, c->getMembers());
        c->_receivedMessages[c->viewChangeAnchorSeq][c->view].insert({"newView", nv});
    }

    Phase* np = PBFTNewViewPhase::instance();
    c->changePhase(np);
    QUANTAS_LOG_TRACE("PBFT") << peer->publicId() << " move to new view in round  " << RoundManager::currentRound();
    QUANTAS_LOG_TRACE("PBFT") << "seqNum: " << c->viewChangeAnchorSeq << " newView: " << c->view;
}

void PBFTNewViewPhase::runPhase(Consensus* con, Peer* peer) {
    auto* c = dynamic_cast<PBFTConsensus*>(con); if (!c) return;
    c->maybeStableCheckpoint(peer);

    if (RoundManager::currentRound() >= c->viewChangeTimer) {
        QUANTAS_LOG_TRACE("PBFT") << peer->publicId()
            << " timeout in new-view at round " << RoundManager::currentRound()
            << " view " << c->view
            << " anchor " << c->viewChangeAnchorSeq;
        c->requestViewChange(peer);
        return;
    }

    int f = c->f();
    // Must see the NEW-VIEW from the new leader
    json nv;
    {
        auto &mm = c->_receivedMessages[c->viewChangeAnchorSeq][c->view];
        auto r = mm.equal_range("newView");
        for (auto it=r.first; it!=r.second; ++it) {
            const auto& m = it->second;
            if (m["from_id"]==c->leaderFor(c->view) && (int)m["view"]==c->view) { nv=m; break; }
        }
        if (nv.empty()) return;
    }
    // And 2f+1 matching VCs for this new view (collected in previous phase)
    {
        std::set<interfaceId> vcSenders;
        auto &mm2 = c->_receivedMessages[c->viewChangeAnchorSeq][c->view-1];
        auto r2 = mm2.equal_range("viewChange");
        for (auto it=r2.first; it!=r2.second; ++it) {
            const auto& m = it->second;
            if ((int)m["newView"]==c->view) vcSenders.insert(m["from_id"].get<interfaceId>());
        }
        if ((int)vcSenders.size() <= 2*f) return;
    }

    // Install watermarks for new view
    c->lowWaterMark  = c->lastStableCheckpoint;
    c->highWaterMark = c->lowWaterMark + c->WINDOW;

    if (nv.contains("confirmedPrefix") && nv["confirmedPrefix"].is_array()) {
        const auto prefix = nv["confirmedPrefix"].get<std::vector<json>>();
        while (c->_confirmedTrans.size() < prefix.size()) {
            const auto& req = prefix[c->_confirmedTrans.size()];
            c->_confirmedTrans.push_back(req);
            c->_latency += RoundManager::currentRound() - req["roundSubmitted"].get<int>();
            QUANTAS_LOG_TRACE("PBFT") << peer->publicId()
                << " state-transfer install seq " << c->_confirmedTrans.size()
                << " in new view " << c->view
                << " at round " << RoundManager::currentRound();
            for (auto it = c->_unhandledRequests.begin(); it != c->_unhandledRequests.end(); ++it) {
                if (it->second == req) {
                    c->_unhandledRequests.erase(it);
                    break;
                }
            }
        }
    }

    std::vector<json> carried = nv["preparedProof"].get<std::vector<json>>();
    std::sort(carried.begin(), carried.end(), [](const json& lhs, const json& rhs) {
        return lhs["seqNum"].get<int>() < rhs["seqNum"].get<int>();
    });

    for (const auto& pp : carried) {
        const int seq = pp["seqNum"].get<int>();
        const auto& req = pp["proposal"]["Request"];

        if (seq == static_cast<int>(c->_confirmedTrans.size()) + 1) {
            c->_confirmedTrans.push_back(req);
            c->_latency += RoundManager::currentRound() - req["roundSubmitted"].get<int>();
        }
    }

    // Re-issue PRE-PREPAREs only for requests not already installed locally.
    for (const auto& pp : carried) {
        const int seq = pp["seqNum"].get<int>();
        if (seq <= static_cast<int>(c->_confirmedTrans.size())) {
            continue;
        }

        json p = {
            {"type","Consensus"},
            {"consensusId", c->getId()},
            {"MessageType","pre-prepare"},
            {"seqNum", seq},
            {"view", c->view},                     // new view
            {"proposal", {
                {"Request", pp["proposal"]["Request"]},  // same payload/digest
                {"view", c->view}
            }},
            {"from_id", peer->publicId()}          // new leader
        };
        peer->multicast(p, c->getMembers());
        c->_receivedMessages[seq][c->view].insert({"pre-prepare", p});
    }
    // Update view change timer since we have made it to the next view
    c->viewChangeTimer = RoundManager::currentRound() + c->viewChangeDelay;

    QUANTAS_LOG_TRACE("PBFT") << peer->publicId() << " move to pre-prepare in round  " << RoundManager::currentRound();
    QUANTAS_LOG_TRACE("PBFT") << "seqNum: " << c->viewChangeAnchorSeq << " newView: " << c->view;
    changePhase(c, PBFTPrePreparePhase::instance());
    c->runPhase(peer);
}

PBFTPeer::~PBFTPeer() {
    for (auto consensus : consensuses) {
        
    }
}

PBFTPeer::PBFTPeer(const PBFTPeer& rhs) : ConsensusPeer(rhs) {
	
}

PBFTPeer::PBFTPeer(NetworkInterface* networkInterface) : ConsensusPeer(networkInterface) {

}

void PBFTPeer::initParameters(const std::vector<Peer*>& _peers, json parameters) {
	const vector<PBFTPeer*> peers = reinterpret_cast<vector<PBFTPeer*> const&>(_peers);

	const int committeeId = 0;
    const int byzantine_count = parameters.value("byzantine_count", 0);
    const int view_change_delay = parameters.value("view_change_delay", 10);
    const int checkpoint_interval = parameters.value("checkpoint_interval", 10);

    Committee* committeePtr = new Committee(committeeId);
    std::set<interfaceId> members;
    for (auto p : peers) {
        members.insert(p->publicId());
        committeePtr->addMember(p->publicId());
    }

    std::set<std::string> types = {"pre-prepare", "prepare", "commit", "checkpoint"};
    std::set<interfaceId> A, B;
    bool flip = true;
    for (auto id : members) { (flip ? A : B).insert(id); flip = !flip; }

    for (int i = 0; i < byzantine_count; ++i) {
        peers[i]->faultManager.addFault(new EquivocateFault(A,B,types));
    }

    // Assign a PBFTConsensus instance using this committee to each peer
    for (auto p : peers) {
        PBFTConsensus* pbft = new PBFTConsensus(new Committee(*committeePtr));
        pbft->viewChangeDelay = view_change_delay;
        pbft->viewChangeTimer = view_change_delay;
        pbft->checkpointInterval = checkpoint_interval;
        p->consensuses[committeeId] = pbft;
	}
    delete committeePtr;
}

void PBFTPeer::endOfRound(vector<Peer*>& _peers) {
	const vector<PBFTPeer*> peers = reinterpret_cast<vector<PBFTPeer*> const&>(_peers);
    double length = 0;
    double latency = 0;
    double faultyConfirmed = 0;
    int count = 0;
    for (auto& p : peers) {
        for (auto& consensus : p->consensuses) {
            length += consensus.second->_confirmedTrans.size();
            latency += consensus.second->_latency;
            for (auto& trans : consensus.second->_confirmedTrans) {
                if (trans.contains("fault_flip") && trans["fault_flip"] == true) {
                    ++faultyConfirmed;
                }
            }
        }
    }

    if (length > 0) {
        OutputWriter::pushValue("latency", latency / length);
        OutputWriter::pushValue("faultyConfirmed", faultyConfirmed / length);
    } else {
        OutputWriter::pushValue("latency", 0.0);
        OutputWriter::pushValue("faultyConfirmed", 0.0);
    }
	OutputWriter::pushValue("throughput", length / peers.size());
    
}

}
