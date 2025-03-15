/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ExamplePeer_hpp
#define ExamplePeer_hpp

#include <iostream>
#include <vector>
#include "../Common/Peer.hpp"


namespace quantas{

    using std::string; 
    using std::ostream;
    using std::vector;

    //
    // Example of a message body type
    //
    class ExampleMessage : public Message {
    public:
        ExampleMessage() {}
        ExampleMessage(string id, string m) : aPeerId(id), message(m) {};
        ExampleMessage* clone() const override {return new ExampleMessage(aPeerId, message);}

        string aPeerId;
        string message;
    };

    //
    // Example Peer used for network testing
    //
    class ExamplePeer : public Peer{
    public:
        // methods that must be defined when deriving from Peer
        ExamplePeer                             (interfaceId);
        ExamplePeer                             (const ExamplePeer &rhs);
        ~ExamplePeer                            ();

        // initialize the configuration of the system
        void                 initParameters(const vector<Peer*>& _peers, json parameters);
        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation ();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound         (const vector<Peer*>& _peers);
    };
}
#endif /* ExamplePeer_hpp */
