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
    interfaceId _id = NO_PEER_ID;
};

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
    int _committeeId = -1;
    std::set<Member> _members;
};

//---------------------------------
// ClientRequest
//---------------------------------
class ClientRequest {
public:
    ClientRequest() : _num(-1), _submitter(NO_PEER_ID), _committeeId(-1)
    {
        _roundSubmitted = RoundManager::currentRound();
    }

    ClientRequest(int n, interfaceId id, int com) : _num(n), _submitter(id), _committeeId(com)
    {
        _roundSubmitted = RoundManager::currentRound();
    }

    ClientRequest(const ClientRequest& rhs) {
        _submitter = rhs._submitter;
        _num = rhs._num;
        _committeeId = rhs._committeeId;
    };


    // Identity of ClientRequest: (submitter, number)
    virtual bool operator==(const ClientRequest& rhs) const {
        return _submitter == rhs._submitter && _num == rhs._num && _committeeId == rhs._committeeId;
    }

    virtual bool operator<(const ClientRequest& rhs) const {
        // test is writing it out is faster
        // (_committeeId == rhs.committeeId && submitter == rhs.submitter && _num < rhs._num)
        if (_committeeId == rhs.committeeId) {
            if (submitter == rhs.submitter) {
                return _num < rhs._num;
            } else {
                return submitter < rhs.submitter;
            }
        } else {
            return committeeId < rhs.committeeId;
        }
    }

    int getNum() const { return _num; }
    interfaceId getSubmitter() const { return _submitter; }
    int getCommitteeId() const { return _committeeId; }
    int getRoundSubmitted() const { return _roundSubmitted; }

    void setNum(int num) { _num = num; }
    void setSubmitter(interfaceId submitter) { _submitter = submitter; }
    void setCommitteeId(int committeeId) { _committeeId = committeeId; }
    void setRoundSubmitted(int round) { _roundSubmitted = round; }

private:
    int _num = -1;
    interfaceId _submitter = NO_PEER_ID;
    int _committeeId = -1;
    int _roundSubmitted = 0;
};

class Proposal {
public:
    Proposal() {}
    Proposal(const Proposal& rhs) {
        _req = rhs._req;
        _seqNum = rhs._seqNum;
        _value = rhs._value;
    };

    int getCommitteeId() const { return _req->getCommitteeId(); }

    ClientRequest* getRequest() const { return _req; }
    void setRequest(ClientRequest* req) { _req = req; }

    int getSeqNum() const { return _seqNum; }
    void setSeqNum(int seqNum) { _seqNum = seqNum; }

    bool getValue() const { return _value; }
    void setValue(bool value) { _value = value; }
private:
    ClientRequest* _req;
    int _seqNum = -1;
    bool _value = true;
};

// Forward declaration
class MessageType;

class ConsensusMessage {
public:
    ConsensusMessage() {}
    ConsensusMessage(const ConsensusMessage& rhs) {
        _proposal = rhs._proposal;
        _phase = rhs._phase;
        _from = rhs._from;
    };

    int getCommitteeId() const { return _proposal->getCommitteeId(); }

    Proposal* getProposal() const { return _proposal; }
    void setProposal(Proposal* proposal) { _proposal = proposal; }

    MessageType* getMessageType() const { return _messageType; }
    void setMessageType(MessageType* messageType) { _messageType = messageType; }

    interfaceId getFrom() const { return _from; }
    void setFrom(interfaceId from) { _from = from; }
private:
    Proposal* _proposal;
    MessageType* _messageType;
    interfaceId _from;
}

class Phase;

// Context
class Consensus {
public:
    Consensus();

    void changePhase(Phase*& phase){_phase=phase;};
    void runPhase() {_phase->run_phase(this)};

    ~Consensus() {
        for_each(receivedMessages.begin(), receivedMessages.end(), [] (auto v) {
			for_each(v.begin(), v.end(), [](auto msg) {
				delete msg.second;
			});
		});
		for_each(transactions.begin(), transactions.end(), [](auto msg) {
			delete msg;
		});
		for_each(confirmedTrans.begin(), confirmedTrans.end(), [](auto msg) {
			delete msg;
		});
    }

public:
    // map of a multimap of messages that have been received 
    // keyed by the sequence number and 
    // phase to which they belong for quicker lookups
    map<int, multi_map<MessageType*, ConsensusMessages*>>    _receivedMessages;
    // vector of recieved ClientRequests
    // consider changing container later
    vector<ClientRequest*>                  _unhandledRequests;
    // vector of confirmed ClientRequests
    // consider changing container later
    vector<ClientRequest*>		            _confirmedTrans;
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
}

class Phase {
public:
    virtual string run_phase(Consensus*& c) =0;

    void changePhase(Consensus*& c, Phase*& s) const {
        c->changePhase(s);
    }
}

/*
class ExamplePhase: public Phase {
public:
    static Phase* instance() {
        static Phase* onlyInstance = new ExamplePhase;
        return onlyInstance;
    }
    string report() override {return "ExamplePhase";}
    string run_phase(Consensus*& c) override {
        changephase(c, ExamplePhase2::instance());
    }
private:
    ExamplePhase() = default;
    ExamplePhase(const ExamplePhase&) = delete;
    ExamplePhase& operator=(const ExamplePhase&) = delete;
};
*/


}