//
//  PBFTPeer_Sharded_Test.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 5/7/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef PBFTPeer_Sharded_Test_hpp
#define PBFTPeer_Sharded_Test_hpp

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "../BlockGuard/PBFTPeer_Sharded.hpp"

void RunPBFTPeerShardedTest      (std::string filepath); // run all PBFT tests

// test for committee PBFT running
void oneRequestOneCommittee     (std::ostream &log);
void oneRequestMultiCommittee   (std::ostream &log);
void MultiRequestMultiCommittee (std::ostream &log);

// These are copies of the test from PBFT Peer as of 5/7
void constructorsPBFT_s         (std::ostream &log);// test basic peer constructorsPBFT_s
void testSettersMutatorsPBFT_s  (std::ostream &log);// test setters and test Mutators

// test for group and committee assignment
void testGroups                 (std::ostream &log);// test that peers can be added and removed from a group
void testCommittee              (std::ostream &log);// test that peers can be added and removed from a committee
void testViewChange             (std::ostream &log);// test that transaction are commited correctly and that they include committee size
void testByzantine              (std::ostream &log);// test defeated transsactions
#endif /* PBFTPeer_Sharded_Test_hpp */
