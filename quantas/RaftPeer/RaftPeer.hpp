/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef RaftPeer_hpp
#define RaftPeer_hpp

#include <deque>
#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas{

    struct RaftPeerMessage {

        int 				Id = -1; // node who sent the message
        int					trans = -1; // the transaction id also used to indicate who a vote is for
        int                 termNum = -1;
        string              messageType = ""; // vote, elect, request, respondRequest
        int                 roundSubmitted;
    };

    class RaftPeer : public Peer<RaftPeerMessage>{
    public:
        // methods that must be defined when deriving from Peer
        RaftPeer                             (long);
        RaftPeer                             (const RaftPeer &rhs);
        ~RaftPeer                            ();

        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound(const vector<Peer<RaftPeerMessage>*>& _peers);

        // addintal method that have defulte implementation from Peer but can be overwritten
        void                 log()const { printTo(*_log); };
        ostream&             printTo(ostream&)const;
        friend ostream& operator<<         (ostream&, const RaftPeer&);
        
        // id of the node voted as the next leader
        int                             candidate = -1;
        // the id of the next transaction to submit
        static int                      currentTransaction;
        // number of requests satisfied
        int                             requestsSatisfied = 0;
        // latency of satisfied requests
        int                             latency = 0;
        // number of rounds to add to timeouts
        const static int                timeOutSpacing = 100;
        // max number of random rounds to add to timeouts 0 - (timeOutRandom - 1)
        const static int                timeOutRandom = 5;
        // round to advance the term
        int                             timeOutRound = 100;
        
        // the term for this leader
        int                             term = 0;
        // current leader
        int                             leaderId = 0;
        // vector containing ids of those who voted for me to be the next leader
        vector<int>		                votes;
        // map containing ids of those who recieved a transaction
        std::map<int, vector<int>>      replys;
        // resets timer
        void                            resetTimer();
        // sends a direct message
        void                            sendMessage(long peer, RaftPeerMessage message);


        // checkInStrm loops through the in stream responding appropriatly to each recieved message
        void                  checkInStrm();
        // submitTrans creates a transaction and broadcasts it to everyone
        void                  submitTrans(int tranID);
    };

    Simulation<quantas::RaftPeerMessage, quantas::RaftPeer>* generateSim();
}
#endif /* RaftPeer_hpp */
