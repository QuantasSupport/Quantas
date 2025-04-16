/*
Copyright 2022
This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CycleOfTreesPeer_hpp
#define CycleOfTreesPeer_hpp

#include <set>
#include <iostream>
#include "../Common/Peer.hpp"

namespace quantas {

    using std::string;
    using std::ostream;
    using std::vector;
    using std::set;
    using std::list;

    struct CycleOfTreesMessage {
        set<int> nodesMessageHasReached = {};
        int      highestIdInKnot        = -1;
    };

    class CycleOfTreesPeer : public Peer<CycleOfTreesMessage> {
    public:
        // methods that must be defined when deriving from Peer
        CycleOfTreesPeer (NetworkInterface*);
        CycleOfTreesPeer (const CycleOfTreesPeer& rhs);
        ~CycleOfTreesPeer();

        // initialize the configuration of the system
        void                 initParameters    (vector<Peer*>& _peers, json parameters);
        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation() override;
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound        (vector<Peer*>& _peers);

        // highest ID detected in the cycle (knot)
        int                  highestID      = -1;
        // nodes you have received messages from
        set<int>             nodesHeardFrom = { publicId() };

        // total number of edges in the backbone topology
        static int           noOfEdges;
        // total number of nodes in the cycle (knot)
        static int           noOfCycleNodes;

        // checkInStrm checks messages
        void                 checkInStrm ();
        // sendMessage sends a node a message
        void                 sendMessage ();
        // pickEdges picks the edges which will be present at a particular state/round
        void                 pickEdges   ();
        // setHighestID sets the highest ID detected in the cycle to the parameter passed
        void                 setHighestID(int);
    };

}

#endif /* CycleOfTreesPeer_hpp */
