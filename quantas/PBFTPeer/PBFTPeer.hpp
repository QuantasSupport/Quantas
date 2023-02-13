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
#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas{

    struct PBFTPeerMessage {

        int 				Id = -1; // node who sent the message
        int					trans = -1; // the transaction id
        int                 sequenceNum = -1;
        string              messageType = ""; // phase
        int                 roundSubmitted;
    };

    class PBFTPeer : public Peer<PBFTPeerMessage>{
    public:
        // methods that must be defined when deriving from Peer
        PBFTPeer                             (long);
        PBFTPeer                             (const PBFTPeer &rhs);
        ~PBFTPeer                            ();

        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound(const vector<Peer<PBFTPeerMessage>*>& _peers);

        // addintal method that have defulte implementation from Peer but can be overwritten
        void                 log()const { printTo(*_log); };
        ostream&             printTo(ostream&)const;
        friend ostream& operator<<         (ostream&, const PBFTPeer&);
        

        // string indicating the current status of a node
        string                          status = "pre-prepare";
        // current squence number
        int                             sequenceNum = 0;
        // vector of vectors of messages that have been received
        vector<vector<PBFTPeerMessage>> receivedMessages;
        // vector of recieved transactions
        vector<PBFTPeerMessage>		    transactions;
        // vector of confirmed transactions
        vector<PBFTPeerMessage>		    confirmedTrans;
        // latency of confirmed transactions
        int                             latency = 0;
        // rate at which to submit transactions ie 1 in x chance for all n nodes
        int                             submitRate = 20;
        
        // the id of the next transaction to submit
        static int                      currentTransaction;


        // checkInStrm loops through the in stream adding messsages to receivedMessages or transactions
        void                  checkInStrm();
        // checkContents loops through the receivedMessages attempting to advance the status of consensus
        void                  checkContents();
        // submitTrans creates a transaction and broadcasts it to everyone
        void                  submitTrans(int tranID);
    };

    Simulation<quantas::PBFTPeerMessage, quantas::PBFTPeer>* generateSim();
}
#endif /* PBFTPeer_hpp */
