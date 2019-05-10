//
//  PBFTReferenceCommittee_Test.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 5/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "PBFTReferenceCommittee_Test.hpp"

void RunPBFTRefComTest (std::string filepath){
    std::ofstream log;
    log.open(filepath + "/PBFTRefCom.log");
    if (log.fail() ){
        std::cerr << "Error: could not open file at: "<< filepath << std::endl;
    }

    testRefComGroups(log);
    testRefComRequest(log);
    testRefComCommittee(log);
}
void testRefComGroups (std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testRefComGroups"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    // test that group size of one make equal number of peers as there are in the system
    PBFTReferenceCommittee refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(1);
    refCom.setFaultTolerance(1);
    refCom.setToRandom();
    refCom.setGroupSize(1);
    refCom.initNetwork(1024);
    refCom.log();

    for(int i = 0; i < refCom.size(); i++){
        PBFTPeer_Sharded* peer = refCom[i];
        assert(peer->getGroup()                     == i);// group id starts from 0
        assert(peer->getFaultTolerance()            == 1);
        assert(peer->getCommittee()                 == -1); // not in a committee
        assert(peer->getGroupMembers()[0]           == peer->id());
        assert(peer->getGroupMembers().size()       == 1);
        assert(peer->getCommitteeMembers().size()   == 0);
    }

    // test that a group with more then one peer do not overlap
    refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(1);
    refCom.setFaultTolerance(1);
    refCom.setToRandom();
    refCom.setGroupSize(16); // 64 groups
    refCom.initNetwork(1024);
    refCom.log();

    // check eachh group and make sure there memebers do not overlap
    std::vector<std::string> seenPeers = std::vector<std::string>();
    for(int id = 0; id < refCom.getGroupIds().size(); id++){
        std::vector<PBFTPeer_Sharded*> group = refCom.getGroup(id);
        for(int peer = 0; peer < group.size(); peer++){
            std::string peerId = group[peer]->id();
            assert(std::find(seenPeers.begin(), seenPeers.end(), peerId) == seenPeers.end()); // makes sure we have not seen this peer before
            seenPeers.push_back(peerId); // now that we have seen the peer we add it to the list of seen peers
        }
    }

    // make sure peer group member ship does not change over time
    // start with making a list of peers to there orignal groups
    std::map<std::string, int> peerToGroup = std::map<std::string, int>();
    for(int id = 0; id < refCom.getGroupIds().size(); id++){
        std::vector<PBFTPeer_Sharded*> group = refCom.getGroup(id);
        for(int peer = 0; peer < group.size(); peer++){
            std::string peerId = group[peer]->id();
            peerToGroup[peerId] = group[peer]->getGroup();
        }
    }
    // now run for a bunch of rounds and check peer groups each time
    for(int i = 0; i < 100; i++){
        refCom.makeRequest(); // note: we are not testing for computation correctness here only making sure thr groups dont change
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        for(int peer = 0; peer < refCom.size(); peer++){
            assert(refCom[peer]->getGroup() == peerToGroup[refCom[peer]->id()]);
        }
    }
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testRefComGroups Done"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void testRefComRequest (std::ostream &log){

}
void testRefComCommittee (std::ostream &log){

}
