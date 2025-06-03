/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef PBFTPeerV2_hpp
#define PBFTPeerV2_hpp

#include <deque>
#include <vector>
#include "../Common/ByzantinePeer.hpp"


namespace quantas{

    using std::vector;

    // class PBFT : public Consensus {

    // }

    // class PBFTPeerV2Message : public Message {
    // public:
    //     PBFTPeerV2Message() {}
    //     PBFTPeerV2Message(const PBFTPeerV2Message& rhs) {
    //         Id = rhs.Id;
    //         trans = rhs.trans;
    //         sequenceNum = rhs.sequenceNum;
    //         messageType = rhs.messageType;
    //         roundSubmitted = rhs.roundSubmitted;
    //     }
    //     PBFTPeerV2Message* clone() const override {return new PBFTPeerV2Message(*this);}
    //     int 				Id = -1; // node who sent the message
    //     int     			trans;   // the transaction
    //     int                 sequenceNum = -1;
    //     string              messageType = ""; // phaseTransaction
    //     int                 roundSubmitted = -1;
        
    // };

    class PBFTPeerV2 : public ByzantinePeer {
    public:
        // methods that must be defined when deriving from Peer
        PBFTPeerV2                             (NetworkInterface*);
        PBFTPeerV2                             (const PBFTPeerV2 &rhs);
        ~PBFTPeerV2                            ();

        // initialize the configuration of the system
        void initParameters(const std::vector<Peer*>& peers, json parameters) override;

        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation() override;
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound(vector<Peer*>& _peers) override;

        // string indicating the current status of a node
        string                          status = "pre-prepare";
        // current squence number
        int                             sequenceNum = 0;
        // current view
        int                             view = 0;
        // timer for suggesting a view change
        int                             viewChangeTimer = 0;
        int                             viewChangeTimeOut = INT_MAX;
        interfaceId                     currentLeader = 0;
        
        // vector of vectors of messages that have been received
        vector<vector<json>> receivedMessages;
        // vector of recieved transactions
        vector<json>		    transactions;
        // vector of confirmed transactions
        vector<json>		    confirmedTrans;
        // vector of view change messages
        std::vector<json> viewChangeMsgs;

        // latency of confirmed transactions
        int                             latency = 0;
        // rate at which to submit transactions ie 1 in x chance for all n nodes
        int                             submitRate = 20;
        // the id of the next transaction to submit
        static int                             currentTransaction;


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

        void processViewChangeMessage(json msg);

        void processNewViewMessage(json msg);        
    };
}
#endif /* PBFTPeerV2_hpp */
