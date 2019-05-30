//
//  PBFTPeerTest.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 4/15/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef PBFTPeerTest_hpp
#define PBFTPeerTest_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "../BlockGuard/PBFT_Peer.hpp"

void RunPBFT_Tests          (std::string filepath); // run all PBFT tests
void constructors           (std::ostream &log);// test basic peer constructors
void testSettersMutators    (std::ostream &log);// test setters and test Mutators
void testGetters            (std::ostream &log);// test getters

void requestFromLeader      (std::ostream &log);// test request from leader can make consensus
void requestFromPeer        (std::ostream &log);// test request from peer can make consensus
void multiRequest           (std::ostream &log);// test multiple request are queued and are commited 
void slowLeaderConnection   (std::ostream &log);// test when leader has slow connection to network
void slowPeerConnection     (std::ostream &log);// test when peer has slow connection to network
void viewChange             (std::ostream &log);// test that peers do a view change if the leader is byzantine
void byzantineCommit        (std::ostream &log);// test that if the leader is byzantine and (1/3)+1 peers are also byzantine then transaction is committed 
void waitingTime            (std::ostream &log);// test that the time from submittion to confirmation is accurate

#endif /* PBFTPeerTest_hpp */
