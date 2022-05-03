//
//  EthereumPeer.hpp
//  BlockGuard
//
//  Created by Joseph Oglio on 4/14/22.
//  Copyright © 2022 Kent State University. All rights reserved.
//

#ifndef EthereumPeer_hpp
#define EthereumPeer_hpp

#include <deque>
#include "./Common/Peer.hpp"

namespace blockguard{

    using std::string; 
    using std::ostream;
    using std::vector;

  
    struct EtherTrans {
        int id              = -1; // the transaction id
        int roundSubmitted  = -1; // the round the transaction was submitted
    };

    struct EtherBlock {
        int 				minerId     = -1; // node who mined the transaction
        EtherTrans			trans;            // transaction
        // Both are needed to uniquely identify a block
        vector<int>         tipMiners   = vector<int>(); // the ids of the nodes who mined the previous blocks
        vector<int>         tipLengths  = vector<int>(); // the lengths of the tip blocks

        int                 length      = 0;  // the length of the blockchain
    };

    struct EthereumPeerMessage {

        EtherBlock			block; // the block being sent
        bool				mined = false; // decides if it's a mined block or submitted transaction
    };

    class EthereumPeer : public Peer<EthereumPeerMessage>{
    protected:
        // counts number of rounds
        int _counter;
        
    public:
        // methods that must be defined when deriving from Peer
        EthereumPeer                             (long);
        EthereumPeer                             (const EthereumPeer&rhs);
        ~EthereumPeer();

        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound(const vector<Peer<EthereumPeerMessage>*>& _peers);


        // its normally a good idea to make some getters and setters for a peer to enable testing 
        //  of the peers state durring an experment and to get metrics. 
        int                  getCounter()const { return _counter; };
        void                 setCounter(int c) { _counter = c; };

        // addintal method that have defulte implementation from Peer but can be overwritten
        void                 log()const { printTo(*_log); };
        ostream&             printTo(ostream&)const;
        friend ostream& operator<<         (ostream&, const EthereumPeer&);
        


        // vector of vectors of blocks that have been mined
        vector<vector<EtherBlock>> blockChain{ { vector<EtherBlock> { EtherBlock() } } };
        // vector of blocks which haven't yet been linked to their previous block
        vector<EtherBlock>         unlinkedBlocks;
        // vector of recieved transactions
        vector<EtherBlock>		   transactions;
        // rate at which to submit transactions ie 1 in x chance for all n nodes
        int                   submitRate = 10;
        // rate at which to mine blocks ie 1 in x chance for all n nodes
        int                   mineRate = 40;
        // the id of the next transaction to submit
        static int            currentTransaction;


        // checkInStrm loops through the in stream adding blocks to unlinked or transactions
        void                  checkInStrm();
        // linkBlocks attempts to add unlinkedBlocks to the blockChain
        void                  linkBlocks();
        // guardSubmit checks if the node should submit a transaction
        bool                  guardSubmitTrans();
        // submitTrans creates a transaction and broadcasts it to everyone
        void                  submitTrans(int tranID);
        // guardMineBlock determines if the node can mine a block
        bool 				  guardMineBlock();
        // mineBlock mines the next transaction, adds it to the blockChain and broadcasts it
        void                  mineBlock();
        // findNextTransaction finds the next unmined transaction on the longest chain of the blockChain
        EtherBlock            findNextTransaction();
        // findTips finds the blocks which are currently leaf nodes
        void                  findTips(vector<int> &ids, vector<int> &lengths);
    };
}
#endif /* EthereumPeer_hpp */
