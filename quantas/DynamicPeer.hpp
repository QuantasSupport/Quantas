/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DynamicPeer_hpp
#define DynamicPeer_hpp

#include "./Common/Peer.hpp"

namespace quantas {

    using std::string;
    using std::ostream;
    using std::vector;

    struct DynamicBlock {
        int                          minerId               = -1;    // miner who mined the transaction
        int                          tipMiner              = -1;    // the id of the miner who mined the previous block
        int                          depth                 = -1;    // distance from genesis block
        int                          roundMined            = -1;    // round block was mined

        bool operator!= (const DynamicBlock&) const;
    };

    struct DynamicMessage {
        vector<DynamicBlock>         blockChain          {};        // sender's blockchain
    };

    class DynamicPeer : public Peer<DynamicMessage> {
    public:
        // methods that must be defined when deriving from Peer
                             DynamicPeer        (long);
                             DynamicPeer        (const DynamicPeer& rhs);
                             ~DynamicPeer       ();

        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation ();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound         (const vector<Peer<DynamicMessage>*>& _peers);

      
        // additional methods that have default implementation from Peer but can be overwritten
        void                 log()                         const { printTo(*_log); };
        ostream&             printTo            (ostream&) const;
        friend ostream&      operator<<         (ostream&, const DynamicPeer&);

      
        // vector of mined blocks
        vector<DynamicBlock>         blockChain;
        // rate at which blocks are mine (i.e., 1 in x chance for all n nodes)
        int                          mineRate            = 40;
        // checkInStrm checks messages
        void                         checkInStrm();
        // guardMineBlock determines if the miner can mine a block
        bool                         guardMineBlock();
        // mineBlock mines the next transaction, adds it to the blockChain and broadcasts it
        void                         mineBlock();
        // sendBlockChain sends miner's blockchain over all present edges
        void                         sendBlockChain();
        // number of accepted blocks (excluding gensis block). A block is considered accepted if all nodes have received said block and are mining on top of it
        static int                   acceptedBlocks;
      
    };
}

#endif /* DynamicPeer_hpp */
