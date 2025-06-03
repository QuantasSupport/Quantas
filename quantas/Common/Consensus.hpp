/*
Copyright 2023

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version. QUANTAS is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CONSENSUS_HPP
#define CONSENSUS_HPP

#include <algorithm>
#include <atomic>
#include <iostream>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Packet.hpp"
#include "RoundManager.hpp"

namespace quantas {

using std::map;
using std::vector;
using std::multimap;
using nlohmann::json;

//---------------------------------
// Committee: A group of members
//---------------------------------
class Committee {
public:
    Committee() : _committeeId(-1) {}
    Committee(int committeeId) : _committeeId(committeeId) {}

    int getId() const { return _committeeId; }
    void setId(int committeeId) {_committeeId = committeeId;}

    // Return a const reference to avoid copying the entire set
    const std::set<interfaceId>& getMembers() const { return _members; }

    void addMember(const interfaceId& toAdd) {
        _members.insert(toAdd);
    }

    void removeMember(const interfaceId& toRemove) {
        _members.erase(toRemove);
    }

    bool contains(interfaceId peerID) const {
        return _members.find(peerID) != _members.end();
    }

    int size() const {
        return static_cast<int>(_members.size());
    }

    // Comparisons for sorting committees, if needed
    bool operator==(const Committee& rhs) const {
        return _committeeId == rhs._committeeId;
    }

    bool operator<(const Committee& rhs) const {
        return _committeeId < rhs._committeeId;
    }

private:
    int _committeeId = -1;
    std::set<interfaceId> _members;
};


// Request {
//     {"requestId", -1},
//     {"submitterId", NO_PEER_ID},
//     {"consensusId", -1},
//     {"roundSubmitted", 0}
// };

// Proposal {
//     {"Request", {}},
//     {"seqNum", -1},
//     {"value", true}
// };

// ConsensusMessage {
//     {"proposal", {}},
//     {"messageType", ""},
//     {"from_id", NO_PEER_ID}
// }

class Consensus;

class Phase {
public:
    virtual void runPhase(Consensus* consensus, Peer* peer) =0;

    void changePhase(Consensus* c, Phase* s) const;
};

// Context
class Consensus {
public:
    Consensus() {};

    void changePhase(Phase*& phase) {
        std::cout << "Phase changed" << std::endl;
        _phase=phase;
    };
    void runPhase(Peer* peer) {_phase->runPhase(this, peer);};
    virtual interfaceId getLeader() {return NO_PEER_ID;};

    int getId() const {return _committee->getId();};
    // Return a const reference to avoid copying the entire set
    const std::set<interfaceId>& getMembers() const { return _committee->getMembers(); }

    ~Consensus() {

    }

public:
    // map of a multimap of messages that have been received 
    // keyed by the sequence number and 
    // phase to which they belong for quicker lookups
    map<int, multimap<string, json>>    _receivedMessages;
    // vector of recieved ClientRequests
    // consider changing container later maybe a list
    vector<json>                  _unhandledRequests;
    // vector of confirmed ClientRequests
    // consider changing container later
    vector<json>		            _confirmedTrans;
    // running sum of latencies of confirmed ClientRequests
    int                                     _latency = 0;
    // rate at which to submit ClientRequests i.e. 1 in x chance for all n nodes
    int                                     _submitRate = 20;
    // the id of the next ClientRequest to submit for this submitter
    int                                     _currentClientRequestId = 0;
    // current phase
    Phase*                                  _phase = nullptr;
    // committee involved in this consensus group
    Committee*                              _committee;
    // tells if the peer has confirmed a zero valued request
    bool                                    _confirmedZero = false;
};

void Phase::changePhase(Consensus* c, Phase* s) const {
    c->changePhase(s);
}

/*
class ExamplePhase: public Phase {
public:
    static Phase* instance() {
        static Phase* onlyInstance = new ExamplePhase;
        return onlyInstance;
    }
    string runPhase(Consensus*& c) override {
        changephase(c, ExamplePhase2::instance());
    }
private:
    ExamplePhase() = default;
    ExamplePhase(const ExamplePhase&) = delete;
    ExamplePhase& operator=(const ExamplePhase&) = delete;
};
*/


}

#endif /* CONSENSUS_HPP */