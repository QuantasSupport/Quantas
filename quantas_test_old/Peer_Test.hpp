/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef Peer_Test_hpp
#define Peer_Test_hpp

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>
#include <stdio.h>
#include <iomanip>
#include "./../quantas/Common/Peer.hpp"

using quantas::NetworkInterface;


void RunPeerTests    (std::string filepath);

void testConst      (std::ostream &log); // test constructors 
void testNeighbor   (std::ostream &log); 
void testRecTrans   (std::ostream &log); // test receive transmit

/*

stub class to test abstract class peer
as well as empty message for testing

*/
struct TestMessage{};

class TestPeer : public NetworkInterface<TestMessage>{
public:
    // methods that must be defined when deriving from NetworkInterface
    TestPeer                                (): NetworkInterface<TestMessage>()                         {};
    TestPeer                                (long id): NetworkInterface(id)                             {std::cout << "Default const called";};
    TestPeer                                (const TestPeer &rhs): NetworkInterface<TestMessage>(rhs)   {std::cout << "Default const called";};
    ~TestPeer                               ()                                                          {std::cout << "testPeer destructor called";};
    void                 performComputation ();
    void                 makeRequest        ();
    void                 log                ()const                                                     {printTo(*_log);};
    std::ostream&        printTo            (std::ostream &out)const                                    {out << "TestPeer printTo called"; return out;};
    friend std::ostream& operator<<         (std::ostream &out, const TestPeer &peer)                   {out << "TestPeer passed to insertion (<<) operator"; peer.printTo(out); return out;};
};


#endif