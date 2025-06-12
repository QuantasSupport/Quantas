/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef PaxosPeer_hpp
#define PaxosPeer_hpp

#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas{

    struct PaxosPeerMessage {

        int 				Id = -1; // node who sent the message
        //int					trans = -1; // the transaction id
        int                 sequenceNum = -1;
        string              messageType = "";
        string              message = ""; // decree
    };

    class PaxosPeer : public Peer<PaxosPeerMessage>{
    public:
        // methods that must be defined when deriving from Peer
        Paxoseer                             (long);
        PaxosPeer                             (const PaxosPeer &rhs);
        ~PaxosPeer                            ();

        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound(const vector<Peer<PaxosPeerMessage>*>& _peers);

        // number last ballot that peer tried to initiate (-1 if the peer hasn't attempted to initiate a ballot)
        int lastTried = -1;
        // vote of ballot with highest sequence number that peer has cast a vote for. (-1 if peer has never voted)
        int prevVote = -1;
        // largest ballot number for which peer has granted promise for. (-1 if peer hasn't sent a lastVote message to anyone)
        int nextBal = -1;

        // vector of vectors of messages that have been received
        vector<vector<PaxosPeerMessage>> receivedMessages;
        // vector of recieved transactions
        vector<PaxosPeerMessage>		    transactions;
        // vector of confirmed transactions OR ledger in reference to Part Time-Parliament
        vector<PaxosPeerMessage>		    confirmedTrans;
        // latency of confirmed transactions
        int                             latency = 0;
        // rate at which to submit transactions ie 1 in x chance for all n nodes
        int                             submitRate = 20;

        // checkInStrm loops through the in stream adding messsages to receivedMessages or transactions
        void                  checkInStrm();
        // checkContents loops through the receivedMessages attempting to advance the status of consensus
        void                  checkContents();
        // submitTrans creates a transaction and broadcasts it to everyone
        void                  submitTrans(int tranID);
    };

    Simulation<quantas::PaxosPeerMessage, quantas::PaxosPeer>* generateSim();
}
#endif /* PBFTPeer_hpp */
