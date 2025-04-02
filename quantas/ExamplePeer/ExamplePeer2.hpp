/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ExamplePeer2_hpp
#define ExamplePeer2_hpp

#include <iostream>
#include <vector>
#include "../Common/Peer.hpp"
#include "ExamplePeer.hpp"


namespace quantas{

    using std::string; 
    using std::ostream;
    using std::vector;

    //
    // Example Peer used for network testing
    //
    class ExamplePeer2 : public Peer{
    public:
        // methods that must be defined when deriving from Peer
        ExamplePeer2                             (interfaceId);
        ExamplePeer2                             (const ExamplePeer2 &rhs);
        ~ExamplePeer2                            ();
        ExamplePeer2(ExamplePeer* rhs) {
            ExamplePeer* oldPeer = reinterpret_cast<ExamplePeer*> (rhs);
    
            _publicId = std::move(rhs->_publicId);
            _internalId = std::move(rhs->_internalId);
            _neighbors = std::move(rhs->_neighbors);
            _inBoundChannels = std::move(rhs->_inBoundChannels);
            _outBoundChannels = std::move(rhs->_outBoundChannels);
            _inStream = std::move(rhs->_inStream);
            msgsSent = std::move(rhs->msgsSent);
        }

        // initialize the configuration of the system
        void                 initParameters(const vector<Peer*>& _peers, json parameters);
        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation ();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound         (const vector<Peer*>& _peers);

        int msgsSent = 0;
    };
}
#endif /* ExamplePeer2_hpp */
