//
//  RaftPeer.hpp
//  BlockGuard
//
//  Created by Joseph Oglio on 4/14/22.
//  Copyright © 2022 Kent State University. All rights reserved.
//

#ifndef RaftPeer_hpp
#define RaftPeer_hpp

#include <deque>
#include "./Common/Peer.hpp"

namespace blockguard{

    struct RaftPeerMessage {

        int 				Id = -1; // node who sent the message
        int					trans = -1; // the transaction id also used to indicate who a vote is for
        int                 termNum = -1;
        string              messageType = ""; // vote, elect, request, respondRequest
        int                 roundSubmitted;
    };

    class RaftPeer : public Peer<RaftPeerMessage>{
    protected:
        // counts number of rounds
        int _counter;
        
    public:
        // methods that must be defined when deriving from Peer
        RaftPeer                             (long);
        RaftPeer                             (const RaftPeer &rhs);
        ~RaftPeer                            ();

        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound(const vector<Peer<RaftPeerMessage>*>& _peers);


        // its normally a good idea to make some getters and setters for a peer to enable testing 
        //  of the peers state durring an experment and to get metrics. 
        int                  getCounter()const { return _counter; };
        void                 setCounter(int c) { _counter = c; };

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
}
#endif /* RaftPeer_hpp */
