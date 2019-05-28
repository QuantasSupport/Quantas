//
//  ByzantineNetwork_Test.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 5/23/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef ByzantineNetwork_Test_hpp
#define ByzantineNetwork_Test_hpp

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "../BlockGuard/PBFTPeer_Sharded.hpp"
#include "../BlockGuard/ByzantineNetwork.hpp"

//////////////////////////////////////////////////////////////////////
// this is the standard set up for number of peers, group size and fault tolerance 
static const int    GROUP_SIZE  = 8; // 32 groups for 256 peers
static const int    PEERS       = 256; 
static const double FAULT       = 0.3;

void RunByzantineNetworkTest    (std::string filepath);

// Byzantine tests
void testMakeByzantine          (std::ostream &log); // test making peers correct and Byzantine
void testByzantineShuffle       (std::ostream &log); // test that shuffling byzantine 


#endif /* ByzantineNetwork_Test_hpp */