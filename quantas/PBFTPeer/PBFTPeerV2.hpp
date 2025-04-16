/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef PBFTPeer_hpp
#define PBFTPeer_hpp

#include <deque>
#include <vector>
#include "../Common/Peer.hpp"
#include "../Common/Consensus.hpp"


namespace quantas{

using std::vector;

class PrePrepare : public Phase {
public:
    static Phase* instance() {
        static Phase* onlyInstance = new ExamplePhase;
        return onlyInstance;
    }
    string report() override {return "prePrepare";}
private:
    PrePrepare() = default;
    PrePrepare(const PrePrepare&) = delete;
    PrePrepare& operator=(const PrePrepare&) = delete;
}

class Prepare : public Phase {
public:
    static Phase* instance() {
        static Phase* onlyInstance = new ExamplePhase;
        return onlyInstance;
    }
    string report() override {return "prepare";}
private:
    Prepare() = default;
    Prepare(const Prepare&) = delete;
    Prepare& operator=(const Prepare&) = delete;
}

class Commit : public Phase {
public:
    static Phase* instance() {
        static Phase* onlyInstance = new ExamplePhase;
        return onlyInstance;
    }
    string report() override {return "commit";}
private:
    Commit() = default;
    Commit(const Commit&) = delete;
    Commit& operator=(const Commit&) = delete;
}

class viewChange : public Phase {
public:
    static Phase* instance() {
        static Phase* onlyInstance = new ExamplePhase;
        return onlyInstance;
    }
    string report() override {return "viewChange";}
private:
    viewChange() = default;
    viewChange(const viewChange&) = delete;
    viewChange& operator=(const viewChange&) = delete;
}

class PBFTConsensus : public Consensus {
    PBFTConsensus() : Consensus() {};

    // checkContents loops through the receivedMessages attempting to advance the status of consensus
    void                  checkContents();

    // submitTrans creates a transaction and broadcasts it to everyone
    void                  submitTrans();

    void updateLeader(const interfaceId& newLeader) {
        currentLeader = newLeader;
    }

    interfaceId getLeader() const {
        return currentLeader;
    }

    bool isLeader(interfaceId id) const {
        return id == currentLeader;
    }

    // current view
    int                             view = 0;
    // timer for suggesting a view change
    int                             viewChangeTimer = 0;
    int                             viewChangeTimeOut = INT_MAX;
    interfaceId                     currentLeader = 0;
}

class PBFTPeer : public Peer{
public:
    // methods that must be defined when deriving from Peer
    PBFTPeer                             (NetworkInterface*);
    PBFTPeer                             (const PBFTPeer &rhs);
    ~PBFTPeer                            ();

    // perform one step of the Algorithm with the messages in inStream
    void                 performComputation() override;
    // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
    void                 endOfRound(vector<Peer*>& _peers) override;
    // checkInStrm loops through the in stream adding messsages to receivedMessages or transactions
    void                 checkInStrm();

    PBFTConsensus consensusObj;

};
}
#endif /* PBFTPeer_hpp */
