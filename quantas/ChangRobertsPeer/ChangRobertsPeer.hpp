/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ChangRobertsPeer_hpp
#define ChangRobertsPeer_hpp

#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas{

    using std::string; 
    using std::ostream;

    //
    // message body type : ID of a process
    //
    struct ChangRobertsMessage{        
        long aPeerId;
    };

    //
    // Example Peer used for network testing
    //
    class ChangRobertsPeer : public Peer<ChangRobertsMessage>{
    public:
        // methods that must be defined when deriving from Peer
        ChangRobertsPeer                             (long);
        ChangRobertsPeer                             (const ChangRobertsPeer &rhs);
        ~ChangRobertsPeer                            ();

        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation ();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound         (const vector<Peer<ChangRobertsMessage>*>& _peers);

        // addintal method that have defulte implementation from Peer but can be overwritten
        void                 log()const { printTo(*_log); };
        ostream&             printTo(ostream&)const;
        friend ostream& operator<<         (ostream&, const ChangRobertsPeer&);

    private:
        bool first_elected;
        long messages_sent;
    };

    Simulation<quantas::ChangRobertsMessage, quantas::ChangRobertsPeer>* generateSim();
}
#endif /* ExamplePeer_hpp */
