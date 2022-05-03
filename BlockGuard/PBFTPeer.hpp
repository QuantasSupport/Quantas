//
//  PBFTPeer.hpp
//  BlockGuard
//
//  Created by Joseph Oglio on 4/14/22.
//  Copyright © 2022 Kent State University. All rights reserved.
//

#ifndef PBFTPeer_hpp
#define PBFTPeer_hpp

#include <deque>
#include "./Common/Peer.hpp"

namespace blockguard{

    struct PBFTPeerMessage {

        int 				Id = -1; // node who sent the message
        int					trans = -1; // the transaction id
        int                 sequenceNum = -1;
        string              messageType = ""; // phase
        int                 roundSubmitted;
    };

    class PBFTPeer : public Peer<PBFTPeerMessage>{
    protected:
        // counts number of rounds
        int _counter;
        
    public:
        // methods that must be defined when deriving from Peer
        PBFTPeer                             (long);
        PBFTPeer                             (const PBFTPeer &rhs);
        ~PBFTPeer                            ();

        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound(const vector<Peer<PBFTPeerMessage>*>& _peers);


        // its normally a good idea to make some getters and setters for a peer to enable testing 
        //  of the peers state durring an experment and to get metrics. 
        int                  getCounter()const { return _counter; };
        void                 setCounter(int c) { _counter = c; };

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
}
#endif /* PBFTPeer_hpp */
