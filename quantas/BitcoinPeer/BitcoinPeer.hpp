/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef BitcoinPeer_hpp
#define BitcoinPeer_hpp

#include <deque>
#include <mutex>
#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas {

    using std::string;
    using std::ostream;
    using std::vector;
    using std::mutex;
    using std::lock_guard;

    struct BitcoinTrans {
        int id = -1; // the transaction id
        int roundSubmitted = -1; // the round the transaction was submitted
    };

    struct BitcoinBlock {
        int 				minerId = -1; // node who mined the transaction
        BitcoinTrans		trans;            // transaction 
        int                 tipMiner = -1; // the id of the node who mined the previous block
        int                 length = 1;  // the length of the blockchain
    };

    struct BitcoinMessage {

        BitcoinBlock		block; // the block being sent
        bool				mined = false; // decides if it's a mined block or submitted transaction
    };

    class BitcoinPeer : public Peer<BitcoinMessage> {
    public:
        // methods that must be defined when deriving from Peer
        BitcoinPeer(long);
        BitcoinPeer(const BitcoinPeer& rhs);
        ~BitcoinPeer();

        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound(const vector<Peer<BitcoinMessage>*>& _peers);

        // addintal method that have defulte implementation from Peer but can be overwritten
        void                 log()const { printTo(*_log); };
        ostream& printTo(ostream&)const;
        friend ostream& operator<<         (ostream&, const BitcoinPeer&);



        // vector of vectors of blocks that have been mined
        vector<vector<BitcoinBlock>> blockChain{ { vector<BitcoinBlock> { BitcoinBlock() } } };
        // vector of blocks which haven't yet been linked to their previous block
        vector<BitcoinBlock>         unlinkedBlocks;
        // vector of recieved transactions
        vector<BitcoinBlock>		  transactions;
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
        BitcoinBlock          findNextTransaction();
    };

    Simulation<quantas::BitcoinMessage, quantas::BitcoinPeer>* generateSim();
};
#endif /* BitcoinPeer_hpp */
