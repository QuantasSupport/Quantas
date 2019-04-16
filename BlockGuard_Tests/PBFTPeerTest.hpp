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
#include "../BlockGuard/PBFT_Peer.hpp"

void constructors();            // test basic peer constructors
void testSettersMutators();     // test setters and test Mutators
void testGetters();             // test getters

void oneRequest();          // test PrePrepare make sure it ends in prepare and send out the right message
void multiRequest();             // test Prepare make sure it ends in either WaitPrepare or commite and send out the right message
void testWaitPrepare();         // test WaitPrepare make sure it ends in either WaitPrepare or commite  and send out the right message
void testCommit();              // test Commit make sure it ends in either WaitCommit or Commite  and send out the right message
void testWaitCommit();          // test WaitCommit make sure it ends in either WaitCommit or IDEAL and send out the right message

void testComputation();         // test to make sure preform computation moves though the phases and does not get stuck

void testBraodcast();           // tests Braodcast
void testCleanLogs();           // test that logs are cleaned after a WaitCommit ends and not before
void testInitPrimary();         // test that correct primary is found
void testFaultyPeers();         // test that fault tolerance is computered correctly


#endif /* PBFTPeerTest_hpp */
