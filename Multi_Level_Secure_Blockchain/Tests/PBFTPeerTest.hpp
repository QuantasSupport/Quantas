//
//  PeerTest.hpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 4/15/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef PBFTPeerTest_hpp
#define PBFTPeerTest_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include "../Multi_Level_Secure_Blockchain/PBFT_Peer.hpp"

//
// Test message body type
//
struct ExampleMessage{
    
    // Primitives
    int                 intData;
    char                charData;
    bool                boolData;
    
    // std ADT
    std::string         stringData;
    std::vector<int>    vectorData;
};

void constructors(); // test basic peer constructors
void testSetters();
void testGetters();
void testMutators();

void testReceive();
void testTransmit();
void test
void testByzantine();


#endif /* PBFTPeerTest_hpp */
