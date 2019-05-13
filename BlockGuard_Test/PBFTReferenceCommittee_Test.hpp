//
//  PBFTReferenceCommittee_Test.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 5/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef PBFTReferenceCommittee_Test_hpp
#define PBFTReferenceCommittee_Test_hpp

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "../BlockGuard/PBFTPeer_Sharded.hpp"
#include "../BlockGuard/PBFTReferenceCommittee.hpp"

void RunPBFTRefComTest              (std::string filepath); // run all PBFT tests

// test for group and committee assignment and maintance 
void testRefComGroups               (std::ostream &log); // test that groups are made correclty and groups and peers have a many to one relationship
void testRefComCommittee            (std::ostream &log); // test that one committee is formed for a transaction 
                                                         //      committees are formed correctly and peers are 
                                                         //      released from committees when consusnes if done
void testGlobalLedger               (std::ostream &log); // test to make sure global ledger is calculated correctly
#endif /* PBFTReferenceCommittee_Test_hpp */