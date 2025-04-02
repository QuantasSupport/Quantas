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

#ifndef Consensus_hpp
#define Consensus_hpp

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

//---------------------------------
// Member: A single participant
//---------------------------------
class Member {
public:
    Member() : _id(NO_PEER_ID) {}
    Member(interfaceId id) : _id(id) {}

    interfaceId getId() const { return _id; }

    // Comparison operators so we can insert into sets, etc.
    bool operator==(const Member& rhs) const {
        return _id == rhs._id;
    }

    bool operator<(const Member& rhs) const {
        return _id < rhs._id;
    }

private:
    interfaceId _id;
};

//---------------------------------
// Committee: A group of members
//---------------------------------
class Committee {
public:
    Committee() : _committeeId(-1) {}
    explicit Committee(int committeeId) : _committeeId(committeeId) {}

    int getId() const { return _committeeId; }

    // Return a const reference to avoid copying the entire set
    const std::set<Member>& getMembers() const { return _members; }

    void addMember(const Member& toAdd) {
        _members.insert(toAdd);
    }

    void removeMember(const Member& toRemove) {
        _members.erase(toRemove);
    }

    bool contains(interfaceId peerID) const {
        // Make a temporary Member with that ID
        Member temp(peerID);
        return _members.find(temp) != _members.end();
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
    int _committeeId;
    std::set<Member> _members;
};

//---------------------------------
// Transaction
//---------------------------------
class Transaction {
public:
    Transaction() : _num(-1), _submitter(NO_PEER_ID), _committeeId(-1)
    {_roundSubmitted = RoundManager::currentRound();}

    Transaction(int n, interfaceId id, int com) : _num(n), _submitter(id), _committeeId(com)
    {
        _roundSubmitted = RoundManager::currentRound();
    }

    // Identity of transaction: (submitter, number)
    bool operator==(const Transaction& rhs) const {
        return _submitter == rhs._submitter && _num == rhs._num;
    }

    // Example ordering based on transaction number
    bool operator<(const Transaction& rhs) const {
        return _num < rhs._num;
    }

    // Public members for brevity; in practice you may want getter/setters
    int _num;
    interfaceId _submitter;
    int _committeeId;
    int _roundSubmitted;
};

class Request {
public:
    Transaction _trans;
    int _sequenceNum = -1;
    bool _value = false;
}

// Forward declaration
class Phase;

class ConsensusMessage : public Message {
public:
    ConsensusMessage() {}
    ConsensusMessage(Request req, Phase*& phase, interfaceId from) : _req(req), _phase(phase), _from(from) {};
    ConsensusMessage* clone() const override {return new ConsensusMessage(_req, _phase, _from);}

    Request _req;
    Phase* _phase;
    interfaceId _from;
}

// Context
class Consensus {
public:
    Consensus();
    void changePhase(Phase*& phase){_phase=phase;};

public:
    // vector of vectors of messages that have been received
    vector<vector<ConsensusMessages*>>  _receivedMessages;
    // vector of recieved transactions
    vector<ConsensusMessages*>          _transactions;
    // vector of confirmed transactions
    vector<ConsensusMessages*>		    _confirmedTrans;
    // latency of confirmed transactions
    int                                 _latency = 0;
    // rate at which to submit transactions ie 1 in x chance for all n nodes
    int                                 _submitRate = 20;
    // the id of the next transaction to submit
    int                                 _currentTransaction;
    // current phase
    Phase*                              _phase;
    // committee involved in this consensus
    Committee                           _committee;
}

class Phase {
public:
    virtual string report() =0;
    void changePhase(Consensus*& c, Phase*& s) const {
        c->changePhase(s);
     }
}

class ExamplePhase: public Phase {
public:
    static Phase* instance() {
        static Phase* onlyInstance = new ExamplePhase;
        return onlyInstance;
    }
    string report() override {return "ExamplePhase";}
private:
    ExamplePhase() = default;
    ExamplePhase(const ExamplePhase&) = delete;
    ExamplePhase& operator=(const ExamplePhase&) = delete;
};



}