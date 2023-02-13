/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KPTPeer_hpp
#define KPTPeer_hpp

#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas {

    using std::string;
    using std::ostream;
    using std::vector;

    struct KPTBlock {
        int                                 minerId        = -1;             // the miner who mined the block
        int                                 tipMiner       = -1;             // the id of the miner who mined the previous block
        int                                 depth          = -1;             // the block's number of ancestors
        int                                 roundMined     = -1;             // the round block was mined

        bool                 operator!=             (const KPTBlock&) const;
        bool                 operator==             (const KPTBlock&) const;
    };

    struct KPTBlockLabel {
        KPTBlock                            block;                           // block
        string                              label          = "unlabeled";    // label of block
    };

    struct KPTMessage {
        vector<KPTBlock>                    blockChain;                      // sender's blockchain
        vector<vector<KPTBlock>>            branches;                        // sender's branches
    };

    class KPTPeer : public Peer<KPTMessage> {
    public:
        // methods that must be defined when deriving from Peer
                             KPTPeer            (long);
                             KPTPeer            (const KPTPeer& rhs);
                             ~KPTPeer           ();

        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation ();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound         (const vector<Peer<KPTMessage>*>& _peers);

        // additional methods that have default implementation from Peer but can be overwritten
        void                 log                () const { printTo(*_log); };
        ostream&             printTo            (ostream&) const;
        friend ostream&      operator<<         (ostream&, const KPTPeer&);

        // vector of mined blocks (i.e., process' main chain)
        vector<KPTBlock>                    blockChain;
        // vector of per-block labels which stores decisions on whether a block is accepted or rejected. If a block is not present in the vector, it is unlabeled.
        vector<KPTBlockLabel>               perBlockLabels;
        // vector of competing branches
        vector<vector<KPTBlock>>            branches;
        // rate at which blocks are mine (i.e., 1 in x chance for all n nodes)
        int                                 mineRate                       = 40;
        // number of accepted blocks (excluding gensis block). A block is considered accepted if all nodes have received said block and are mining on top of it
        int                                 acceptedBlocks                 = 0;


        // checkInStrm checks messages
        void                 checkInStrm            ();
        // guardMineBlock determines if the miner can mine a block
        bool                 guardMineBlock         ();
        // mineBlock has the miner mine a block on top of their blockchain
        void                 mineBlock              ();
        // sendBlockChain sends a miner's blockchain over all present edges
        void                 sendBlockChain         ();
        // createBranch updates or inserts into the container 'branches'
        void                 createBranch           (const vector<KPTBlock>&);
        // updateBlockLabels iterates through a process' blockchain and branches and labels blocks appropriately
        void                 updateBlockLabels      ();
        // updatePerBlockLabels adds labeled block to perBlockLabels
        void                 updatePerBlockLabels   (const KPTBlock&, const string&);
    };

    Simulation<quantas::KPTMessage, quantas::KPTPeer>* generateSim();

}

#endif /* KPTPeer_hpp */

