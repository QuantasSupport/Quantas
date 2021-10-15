//
//  PBFTReferenceCommittee_Test.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 11/5/20.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef Peer_Test_hpp
#define Peer_Test_hpp

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>
#include <stdio.h>
#include <iomanip>
#include "./../Blockguard/Common/Peer.hpp"

using blockguard::NetworkInterface;


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