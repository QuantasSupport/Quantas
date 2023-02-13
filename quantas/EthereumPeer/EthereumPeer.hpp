/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef EthereumPeer_hpp
#define EthereumPeer_hpp

#include <deque>
#include <mutex>
#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas{

    using std::string; 
    using std::ostream;
    using std::vector;
    using std::mutex;
    using std::lock_guard;
  
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
    public:
        // methods that must be defined when deriving from Peer
        EthereumPeer                             (long);
        EthereumPeer                             (const EthereumPeer&rhs);
        ~EthereumPeer();

        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound(const vector<Peer<EthereumPeerMessage>*>& _peers);

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
        int                   submitRate = 20;
        // rate at which to mine blocks ie 1 in x chance for all n nodes
        int                   mineRate = 40;
        // the id of the next transaction to submit
        static int            currentTransaction;
        static mutex          currentTransaction_mutex;

        // checkInStrm loops through the in stream adding blocks to unlinked or transactions
        void                  checkInStrm();
        // linkBlocks attempts to add unlinkedBlocks to the blockChain
        void                  linkBlocks();
        // guardSubmit checks if the node should submit a transaction
        bool                  guardSubmitTrans();
        // submitTrans creates a transaction and broadcasts it to everyone
        void                  submitTrans();
        // guardMineBlock determines if the node can mine a block
        bool 				  guardMineBlock();
        // mineBlock mines the next transaction, adds it to the blockChain and broadcasts it
        void                  mineBlock();
        // findNextTransaction finds the next unmined transaction on the longest chain of the blockChain
        EtherBlock            findNextTransaction();
        // findTips finds the blocks which are currently leaf nodes
        void                  findTips(vector<int> &ids, vector<int> &lengths);
    };

    Simulation<quantas::EthereumPeerMessage, quantas::EthereumPeer>* generateSim();
}
#endif /* EthereumPeer_hpp */
