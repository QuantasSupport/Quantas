/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

//Note: This implementation of Paxos represents classic Multi-Paxos (meaning no leader with multiple rounds of consensus)

#ifndef PaxosPeer_hpp
#define PaxosPeer_hpp

#include <set>
#include <map>
#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas{

    // ballot numbers are stored as std::pairs. first is ballotIndex, second is peer id. 
    // If p.first == q.first, id is used to decide the greater ballot number.
    // comparison operators for std::pair already compare first element and then second element
    // so no additional logic is needed

    struct PaxosPeerMessage {
        int                 Id = -1;
        std::pair<int,int>  lastVoted = {-1,-1}; // used for lastVote message. -1 if irrelavent
        std::pair<int,int>  ballotNum = {-1,-1}; // note: no need for separate id variable. stored in second
        string              messageType = "";
        string              decree = ""; // decree
        int                 slotNumber      = -1; // slot number
    };


    // ledger contains peer data considered crash safe (stored in stable memory)
    struct Ledger {
        // number last ballot that peer tried to initiate (-1 if the peer hasn't attempted to initiate a ballot)
        std::pair<int,int> lastTried = {-1,-1};

        // number of ballot for which peer last voted for (-1 if peer has never voted)
        std::pair<int, int> prevBal = {-1,-1};

        // decree of ballot for which peer last voted for ("" if peer has never voted) 
        string ledgerDecree = "";

        // largest ballot number for which peer has granted promise for.
        //(first is -1 if peer hasn't received any ballots yet)
        std::pair<int,int> nextBal = {-1,-1};

        // successful ballot decree
        string outcome = "";

        // slot number (essential for multi-paxos). indicates what slot peer is trying to achieve consensus for
        int currentSlot = 0;
    };

    
    // paper contains peer data not considered crash safe (not stored in stable memory)
    struct Paper {
        enum Status {
            IDLE,
            TRYING,
            POLLING
        };

        Status status = IDLE;
        
        // set of votes received in lastVote messages
        vector<PaxosPeerMessage> prevVotes;

        // set of peers forming the quorum of current ballot (set of peers that will decide whether to vote on ballot)
        std::set<int> quorum;

        // set of quorum peers that have sent peer Voted message for current ballot
        std::set<int> voters;

        // if peer is polling then decree of current ballot (otherwise -1)
        int paperDecree = -1;

        // STILL FIGURING THIS OUT
        int timer = 0;

    };

    class PaxosPeer : public Peer<PaxosPeerMessage>{
    public:
        // methods that must be defined when deriving from Peer
        PaxosPeer                             (long);
        PaxosPeer                             (const PaxosPeer &rhs);
        ~PaxosPeer                            ();

        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound(const vector<Peer<PaxosPeerMessage>*>& _peers);

        // returns a NextBallot PaxosPeer message with unique sequence number
        PaxosPeerMessage nextBallot();
        // returns a LastVote PaxosPeer message
        PaxosPeerMessage lastMessage();
        // returns a BeginBallot PaxosPeer message
        PaxosPeerMessage beginBallot();
        // returns a Voted PaxosPeer message
        PaxosPeerMessage voted();
        // returns a Success PaxosPeer message
        PaxosPeerMessage success();

        // stores all data that is expected to remain if peer crashes
        Ledger ledgerData;

        // stores all data that may be corrupted if peer crashes
        Paper paperData;

        // function that clears ledger and paper when slot changes
        void clearState();

        // used for creating ballot/sequence numbers
        int ballotIndex = 0;

        // vector of vectors of messages that have been received
        vector<vector<PaxosPeerMessage>> receivedMessages;
        // map of confirmed transactions, where key is slot number
        // and value is the message that reached consensus for that slot 
        std::map<int,string>		    confirmedTrans;

        // latency of successful ballots
        int                             latency = 0;
        /// throughput of successful ballots
        int                             throughput = 0;

        // round number of when the last ballot was submitted. for tracking latency.
        // -1 if peer hasn't submitted a ballot yet.
        int                             roundSent = -1;
        // rate at which to submit transactions ie 1 in x chance for all n nodes
        int                             submitRate = 20;

        // AAA
        void                  checkInStrm();
        // Used for submitting a new ballot to vote on
        void                  submitBallot();
        
        // direct messages between peers
        void sendMessage(long, PaxosPeerMessage);

    };

    Simulation<quantas::PaxosPeerMessage, quantas::PaxosPeer>* generateSim();
}
#endif /* PaxosPeer_hpp */
