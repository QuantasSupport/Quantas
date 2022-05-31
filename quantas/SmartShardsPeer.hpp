/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SmartShardsPeer_hpp
#define SmartShardsPeer_hpp

#include <deque>
#include <map>
#include "./Common/Peer.hpp"

namespace quantas{

    using std::map;

    struct SmartShardsMessage {

        int 				Id = -1; // node who sent the message
        int					trans = -1; // the transaction id
        int                 sequenceNum = -1;
        int                 shard = -1; // shard the transaction is for
        string              messageType = ""; // phase
        int                 roundSubmitted;
    };

    class SmartShardsPeer : public Peer<SmartShardsMessage>{
    public:
        // methods that must be defined when deriving from Peer
        SmartShardsPeer                             (long);
        SmartShardsPeer                             (const SmartShardsPeer &rhs);
        ~SmartShardsPeer                            ();

        // initialize the configuration of the system
        void                 initParameters(const vector<Peer<SmartShardsMessage>*>& _peers, json parameters);
        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound(const vector<Peer<SmartShardsMessage>*>& _peers);

        // addintal method that have defulte implementation from Peer but can be overwritten
        void                 log()const { printTo(*_log); };
        ostream&             printTo(ostream&)const;
        friend ostream& operator<<         (ostream&, const SmartShardsPeer&);

        // string indicating the current status of a node in each shard
        map<int, string>                status;
        // list of the shards the node is in and the status if the node is the leader of that shard
        map<int, bool>                  shards;
        // ids of the members of the shards this node is in
        map<int, vector<long>>          members;
        // current squence number
        int                             sequenceNum = 0;
        // map of vectors of messages that have been received keyed by transaction id
        map<int, vector<SmartShardsMessage>> receivedMessages;
        // vector of recieved transactions
        vector<SmartShardsMessage>		    transactions;
        // vector of confirmed transactions
        vector<SmartShardsMessage>		    confirmedTrans;
        // latency of confirmed transactions
        int                             latency = 0;
        int                             messagesSent = 0;
        // transaction currently being processed in a specific shard
        map<int, int>                   workingTrans;
        
        // the id of the next transaction to submit
        static int                      currentTransaction;


        // checkInStrm loops through the in stream adding messsages to receivedMessages or transactions
        void                  checkInStrm();
        // checkContents loops through the receivedMessages attempting to advance the status of consensus
        void                  checkContents(int shard);
        // submitTrans creates a transaction and broadcasts it to the necessary shard
        void                  submitTrans(int shard);
        // send message to everyone in the shard
        void                  sendMessage(int shard, SmartShardsMessage message);
    };
}
#endif /* SmartShardsPeer_hpp */
