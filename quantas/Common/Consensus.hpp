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
#include <map>
#include <set>
#include <vector>

#include "Packet.hpp"
#include "RoundManager.hpp"
#include "Committee.hpp"

namespace quantas {

using std::vector;
using std::multimap;
using nlohmann::json;

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
        _phase=phase;
    };
    void runPhase(Peer* peer) {_phase->runPhase(this, peer);};

    int getId() const {return _committee->getId();};
    // Return a const reference to avoid copying the entire set
    const std::set<interfaceId>& getMembers() const { return _committee->getMembers(); }

    virtual ~Consensus() {
        delete _committee;
    }

public:
    // multimap of received ClientRequests
    // key is the round the request was received
    multimap<int, json>                     _unhandledRequests;
    // vector of confirmed ClientRequests
    // consider changing container later
    vector<json>		                    _confirmedTrans;
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