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
#include <set>
#include <mutex>
#include <iostream>
#include <bits/stdc++.h>
#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas {

    using std::map;
    using std::set;
    using std::mutex;
    using std::lock_guard;

    struct SmartShardsMember {
        long Id = -1;
        bool leader = false;
        set<int> shards;
    };

    struct SmartShardsMessage {

        long 				Id = -1; // node who sent the message
        int					trans = -1; // the transaction id
        int                 sequenceNum = -1;
        int                 shard = -1; // shard the transaction is for
        string              messageType = ""; // type of the message being sent; many options
        int                 roundSubmitted;
        vector<std::pair<string, long>> churningNodes; // type of churn and id of nodes churning
        vector<SmartShardsMember>       members; // the ids and other shards of the nodes in the shard (used when a node joins the shard)
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
        // list of the shards the node is in and if the node is the leader of that shard
        map<int, bool>                  shards;
        // ids and shards of the members of the shards this node is in
        map<int, vector<SmartShardsMember>>          members;
        // ids of the nodes requesting to leave/join
        map<int, vector<std::pair<string, long>>>          churnRequests;
        // tracks if node is trying to leave or join
        bool                            leaving = false;
        bool                            joining = false;
        // amount of time passed since sending leave or join request
        int                             leaveDelay = 0;
        int                             joinDelay = 0;
        // tracks if node is awake
        bool                            alive = false;
        // current squence number
        int                             sequenceNum = 0;
        // map of vectors of messages that have been received keyed by transaction id
        map<int, vector<SmartShardsMessage>> receivedMessages;
        // vector of recieved transactions
        vector<SmartShardsMessage>		    transactions;
        // vector of confirmed transactions
        vector<SmartShardsMessage>		    confirmedTrans;
        // Various metrics
        int                             latency = 0;
        int                             messagesSent = 0;
        int                             timeToJoin = 0;
        int                             timeToLeave = 0;
        // transaction currently being processed in a specific shard
        map<int, int>                   workingTrans;
        
        // the id of the next transaction to submit
        static int                      currentTransaction;
        static mutex                    currentTransaction_mutex;
        // percent of network which will request to join/leave each round
        static int                      churnRate;
        // index of the next node to request to join the network
        static int                      nextJoiningNode;
        // amount of time node is willing to wait before leaving
        static int                      maxLeaveDelay;
        static int                      numberOfShards;
         // 0 - joins 2 random, 1 - joins 1 random routed to second, 2 - no churn permitted, 3 - 1 & tries to balance the shardsfor routed joins
        static int                      ChurnOption;

        // checkInStrm loops through the in stream adding messsages to receivedMessages or transactions
        void                  checkInStrm();
        // checkContents loops through the receivedMessages attempting to advance the status of consensus
        void                  checkContents(int shard);
        // submitTrans creates a transaction and broadcasts it to the necessary shard
        void                  submitTrans(int shard);
        // send message to everyone in the shard
        void                  sendMessageShard(int shard, SmartShardsMessage message);
        // send message to specific node by id
        void                  sendMessage(int node, SmartShardsMessage message);
        // updates the status of a member node
        void                  updateMember(int shard, SmartShardsMember member);

    };


    Simulation<quantas::SmartShardsMessage, quantas::SmartShardsPeer>* generateSim();
}
#endif /* SmartShardsPeer_hpp */
