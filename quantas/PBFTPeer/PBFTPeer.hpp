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


namespace quantas{

    using std::vector;

    // class PBFT : public Consensus {

    // }

    class PBFTPeerMessage : public Message {
    public:
        PBFTPeerMessage() {}
		PBFTPeerMessage(int id, int tran, int sqnNum, string msgType, int rndSubmit) : Id(id), trans(tran), sequenceNum(sqnNum), messageType(msgType), roundSubmitted(rndSubmit) {};
        int 				Id = -1; // node who sent the message
        Transaction			trans;   // the transaction
        int                 sequenceNum = -1;
        string              messageType = ""; // phaseTransaction
        PBFTPeerMessage* clone() const override {return new PBFTPeerMessage(Id, trans, sequenceNum, messageType, roundSubmitted);}
    };

    class PBFTPeer : public Peer{
    public:
        // methods that must be defined when deriving from Peer
        PBFTPeer                             (interfaceId);
        PBFTPeer                             (const PBFTPeer &rhs);
        ~PBFTPeer                            ();

        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound(const vector<Peer*>& _peers);

        // string indicating the current status of a node
        string                          status = "pre-prepare";
        // current squence number
        int                             sequenceNum = 0;
        // current view
        int                             view = 0;
        // timer for suggesting a view change
        int                             viewChangeTimer = 0;
        int                             viewChangeTimeOut = INT_MAX;
        interfaceId                     currentLeader = NO_PEER_ID;
        
        // vector of vectors of messages that have been received
        vector<vector<PBFTPeerMessage*>> receivedMessages;
        // vector of recieved transactions
        vector<PBFTPeerMessage*>		    transactions;
        // vector of confirmed transactions
        vector<PBFTPeerMessage*>		    confirmedTrans;
        // vector of view change messages
        std::vector<PBFTPeerMessage*> viewChangeMsgs;

        // latency of confirmed transactions
        int                             latency = 0;
        // rate at which to submit transactions ie 1 in x chance for all n nodes
        int                             submitRate = 20;
        // the id of the next transaction to submit
        int                             currentTransaction;


        // checkInStrm loops through the in stream adding messsages to receivedMessages or transactions
        void                  checkInStrm();
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

        // view change functions
        void initiateViewChange();

        void processViewChangeMessage(const PBFTPeerMessage* &msg);

        void processNewViewMessage(const PBFTPeerMessage* &msg);        
    };
}
#endif /* PBFTPeer_hpp */
