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

   // testRefComGroups(log);
    //testRefComCommittee(log);
    testGlobalLedger(log);
}
void testRefComGroups (std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testRefComGroups"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    // test that group size of one make equal number of peers as there are in the system
    PBFTReferenceCommittee refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(1);
    refCom.setToRandom();
    refCom.setGroupSize(1);
    refCom.setFaultTolerance(0.3);
    refCom.initNetwork(1024);

    assert(refCom.numberOfGroups()              == 1024);
    assert(refCom.getBusyGroups().size()        == 0);
    assert(refCom.getFreeGroups().size()        == 1024);

    for(int i = 0; i < refCom.size(); i++){
        PBFTPeer_Sharded* peer = refCom[i];
        assert(peer->getGroup()                     == i);// group id starts from 0
        assert(peer->getFaultTolerance()            == 0.3);
        assert(peer->getCommittee()                 == -1); // not in a committee
        assert(peer->getGroupMembers().size()       == 0); // self is not included in list og group memebers
        assert(peer->getCommitteeMembers().size()   == 0);
    }

    // test that a group with more then one peer do not overlap
    refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(1);
    refCom.setFaultTolerance(0.3);
    refCom.setToRandom();
    refCom.setGroupSize(16); // 64 groups
    refCom.initNetwork(1024);

    // test each group to make sure it was formed correctly
    for(int id = 0; id < refCom.getGroupIds().size(); id++){
        std::vector<PBFTPeer_Sharded*> group = refCom.getGroup(id);
        assert(group.size() == 16);
        for(int peer = 0; peer < group.size(); peer++){
            assert(group[peer]->getGroup()                     == id);// group id starts from 0
            assert(group[peer]->getFaultTolerance()            == 0.3);
            assert(group[peer]->getCommittee()                 == -1); // not in a committee
            assert(group[peer]->getGroupMembers().size()       == 15); // self is not included in list og group memebers
            assert(group[peer]->getCommitteeMembers().size()   == 0);
        }
    }

    // check each group and make sure there memebers do not overlap
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
    // note: we are not testing for computation correctness here only making sure thr groups dont change
    for(int i = 0; i < 10; i++){
        refCom.makeRequest(); 
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();

        for(int peer = 0; peer < refCom.size(); peer++){
            assert(refCom[peer]->getGroup() == peerToGroup[refCom[peer]->id()]);
        }
    }
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testRefComGroups Done"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void testRefComCommittee (std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testRefComCommittee"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    ///////////////////////////////////////////////////
    // small set of peers to debug
    // this is the standard set up 32 peer 16 groups 1/3 fault tolerance 
    PBFTReferenceCommittee refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(1);
    refCom.setToRandom();
    refCom.setGroupSize(2);
    refCom.setFaultTolerance(1);
    refCom.initNetwork(32);
    refCom.setMaxSecurityLevel(2);
    refCom.setMinSecurityLevel(2);
    refCom.log();
    
    assert(refCom.getBusyGroups().size()            == 0);
    assert(refCom.getFreeGroups().size()            == 16);

    refCom.queueRequest();
    assert(refCom.getRequestQueue().size()          == 1);
    double securityLevel = refCom.getRequestQueue().front().securityLevel;

    refCom.makeRequest(); // this will also add a new request to the queue but only one request can be sent per round
    assert(refCom.getNextCommitteeId()              == 1);
    refCom.log();
    
    // checking busy and free groups to make sure that the right number of groups are taken by the committee
    assert(refCom.getBusyGroups().size()            == securityLevel);
    assert(refCom.getFreeGroups().size()            == refCom.numberOfGroups() - securityLevel);
    assert(refCom.getCurrentCommittees().size()     == 1);

    // check to make sure committee was made correctly
    int committeeId = refCom.getCurrentCommittees().front();
    std::vector<aGroup> committee = refCom.getCommittee(committeeId);
    std::vector<int> committeeGroups = refCom.getBusyGroups();
    // loop though busy groups and make sure each is in this committee
    for(auto groupId = committeeGroups.begin(); groupId != committeeGroups.end(); groupId++){
        aGroup group = refCom.getGroup(*groupId);
        // make sure group properties are maintained
        assert(group.size() == 2);
        for(auto peer = group.begin(); peer != group.end(); peer++){
            assert((*peer)->getCommittee() == committeeId);
        }
    }

    // run a round and recheck
    refCom.receive();
    refCom.preformComputation();
    refCom.transmit();
    refCom.log();

    for(auto groupId = committeeGroups.begin(); groupId != committeeGroups.end(); groupId++){
        aGroup group = refCom.getGroup(*groupId);
        // make sure group properties are maintained
        assert(group.size() == 2);
        for(auto peer = group.begin(); peer != group.end(); peer++){
            assert((*peer)->getCommittee() == committeeId);
            if((*peer)->getPrimary() == (*peer)->id()){
                assert((*peer)->getPhase() == PREPARE_WAIT); // make sure primary got request
            }
        }
    }

    // next round
    refCom.receive();
    refCom.preformComputation();
    refCom.transmit();
    refCom.log();

    for(auto groupId = committeeGroups.begin(); groupId != committeeGroups.end(); groupId++){
        aGroup group = refCom.getGroup(*groupId);
        // make sure group properties are maintained
        assert(group.size() == 2);
        for(auto peer = group.begin(); peer != group.end(); peer++){
            assert((*peer)->getCommittee() == committeeId);
            assert((*peer)->getPhase() == PREPARE_WAIT); // make sure every peer got request
        }
    }

    // commit round
    refCom.receive();
    refCom.preformComputation();
    refCom.transmit();
    refCom.log();

    for(auto groupId = committeeGroups.begin(); groupId != committeeGroups.end(); groupId++){
        aGroup group = refCom.getGroup(*groupId);
        // make sure group properties are maintained
        assert(group.size() == 2);
        for(auto peer = group.begin(); peer != group.end(); peer++){
            assert((*peer)->getCommittee() == committeeId);
            assert((*peer)->getPhase() == COMMIT_WAIT); // make sure every peer is ready to commit
        }
    }

    // committee should be disolved
    refCom.receive();
    refCom.preformComputation();
    refCom.transmit();
    refCom.log();

    for(auto groupId = committeeGroups.begin(); groupId != committeeGroups.end(); groupId++){
        aGroup group = refCom.getGroup(*groupId);
        // make sure group properties are maintained
        assert(group.size() == 2);
        for(auto peer = group.begin(); peer != group.end(); peer++){
            assert((*peer)->getCommittee() == -1); // make sure they are no longer in committee
            assert((*peer)->getPhase() == IDEAL); // make sure every peer is done
        }
    }

    //////////////////////////////////////////////////////////////////////
    // this is the standard set up 1024 peer 64 groups 1/3 fault tolerance 
    refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(1);
    refCom.setToRandom();
    refCom.setGroupSize(16);
    refCom.setFaultTolerance(0.3);
    refCom.initNetwork(1024);
    refCom.setMaxSecurityLevel(2);
    refCom.setMinSecurityLevel(2);

    assert(refCom.getBusyGroups().size()            == 0);
    assert(refCom.getFreeGroups().size()            == 64);

    refCom.queueRequest();
    assert(refCom.getRequestQueue().size()          == 1);
    assert(refCom.getCurrentCommittees().size()     == 0);

    securityLevel = refCom.getRequestQueue().front().securityLevel;

    refCom.makeRequest(); // this will also add a new request to the queue but only one request can be sent per round
    assert(refCom.getRequestQueue().size()          == 1);
    assert(refCom.getNextCommitteeId()              == 1);
    
    // checking busy and free groups to make sure that the right number of groups are taken by the committee
    assert(refCom.getRequestQueue().size()          == 1);
    assert(refCom.getBusyGroups().size()            == securityLevel);
    assert(refCom.getFreeGroups().size()            == refCom.numberOfGroups() - securityLevel);
    assert(refCom.getCurrentCommittees().size()     == 1);

    // check to make sure committee was made correctly
    committeeId = refCom.getCurrentCommittees().front();
    committee = refCom.getCommittee(committeeId);
    committeeGroups = refCom.getBusyGroups();
    // loop though busy groups and make sure each is in this committee
    for(auto groupId = committeeGroups.begin(); groupId != committeeGroups.end(); groupId++){
        aGroup group = refCom.getGroup(*groupId);
        // make sure group properties are maintained
        assert(group.size() == 16);
        for(auto peer = group.begin(); peer != group.end(); peer++){
            assert((*peer)->getCommittee() == committeeId);
        }
    }

    // run a round and recheck
    refCom.receive();
    refCom.preformComputation();
    refCom.transmit();

    for(auto groupId = committeeGroups.begin(); groupId != committeeGroups.end(); groupId++){
        aGroup group = refCom.getGroup(*groupId);
        // make sure group properties are maintained
        assert(group.size() == 16);
        for(auto peer = group.begin(); peer != group.end(); peer++){
            assert((*peer)->getCommittee() == committeeId);
            if((*peer)->getPrimary() == (*peer)->id()){
                assert((*peer)->getPhase() == PREPARE_WAIT); // make sure primary got request
            }
        }
    }

    // next round
    refCom.receive();
    refCom.preformComputation();
    refCom.transmit();

    assert(refCom.getCurrentCommittees().size()     == 1);
    assert(refCom.getRequestQueue().size()          == 1);
    for(auto groupId = committeeGroups.begin(); groupId != committeeGroups.end(); groupId++){
        aGroup group = refCom.getGroup(*groupId);
        // make sure group properties are maintained
        assert(group.size() == 16);
        for(auto peer = group.begin(); peer != group.end(); peer++){
            assert((*peer)->getCommittee() == committeeId);
            assert((*peer)->getPhase() == PREPARE_WAIT); // make sure every peer got request
        }
    }

    // commit round
    refCom.receive();
    refCom.preformComputation();
    refCom.transmit();

    assert(refCom.getCurrentCommittees().size()     == 1);
    assert(refCom.getRequestQueue().size()          == 1);
    for(auto groupId = committeeGroups.begin(); groupId != committeeGroups.end(); groupId++){
        aGroup group = refCom.getGroup(*groupId);
        // make sure group properties are maintained
        assert(group.size() == 16);
        for(auto peer = group.begin(); peer != group.end(); peer++){
            assert((*peer)->getCommittee() == committeeId);
            assert((*peer)->getPhase() == COMMIT_WAIT); // make sure every peer is ready to commit
        }
    }

    refCom.receive();
    refCom.preformComputation();
    refCom.transmit();
    
    // committee should be disolved
    assert(refCom.getCurrentCommittees().size()     == 0);
    assert(refCom.getRequestQueue().size()          == 1);
    assert(refCom.getBusyGroups().size()            == 0);
    assert(refCom.getFreeGroups().size()            == 64);

    for(auto groupId = committeeGroups.begin(); groupId != committeeGroups.end(); groupId++){
        aGroup group = refCom.getGroup(*groupId);
        // make sure group properties are maintained
        assert(group.size() == 16);
        for(auto peer = group.begin(); peer != group.end(); peer++){
            assert((*peer)->getCommittee() == -1); // make sure they are no longer in committee
            assert((*peer)->getPhase() == IDEAL); // make sure every peer is done
            assert((*peer)->getLedger().size() == 1); // make sure every peer comited the request
        }
    }
    ////////////////////////
    // repeat to make sure committees do not interfere with each other

    refCom.queueRequest();
    assert(refCom.getRequestQueue().size()          == 2);
    assert(refCom.getNextCommitteeId()              == 1);
    securityLevel = refCom.getRequestQueue().front().securityLevel;

    refCom.makeRequest(); // this will also add a new request to the queue but only one request can be sent per round
    
    // checking busy and free groups to make sure that the right number of groups are taken by the committee
    assert(refCom.getBusyGroups().size()            == securityLevel);
    assert(refCom.getFreeGroups().size()            == refCom.numberOfGroups() - securityLevel);
    assert(refCom.getCurrentCommittees().size()     == 1);

    // check to make sure committee was made correctly
    committeeId = refCom.getCurrentCommittees().front();
    committee = refCom.getCommittee(committeeId);
    committeeGroups = refCom.getBusyGroups();
    // loop though busy groups and make sure each is in this committee
    for(auto groupId = committeeGroups.begin(); groupId != committeeGroups.end(); groupId++){
        aGroup group = refCom.getGroup(*groupId);
        // make sure group properties are maintained
        assert(group.size() == 16);
        for(auto peer = group.begin(); peer != group.end(); peer++){
            assert((*peer)->getCommittee() == committeeId);
        }
    }

    // run a round and recheck
    refCom.receive();
    refCom.preformComputation();
    refCom.transmit();

    for(auto groupId = committeeGroups.begin(); groupId != committeeGroups.end(); groupId++){
        aGroup group = refCom.getGroup(*groupId);
        // make sure group properties are maintained
        assert(group.size() == 16);
        for(auto peer = group.begin(); peer != group.end(); peer++){
            assert((*peer)->getCommittee() == committeeId);
            if((*peer)->getPrimary() == (*peer)->id()){
                assert((*peer)->getPhase() == PREPARE_WAIT); // make sure primary got request
            }
        }
    }

    // next round
    refCom.receive();
    refCom.preformComputation();
    refCom.transmit();

    for(auto groupId = committeeGroups.begin(); groupId != committeeGroups.end(); groupId++){
        aGroup group = refCom.getGroup(*groupId);
        // make sure group properties are maintained
        assert(group.size() == 16);
        for(auto peer = group.begin(); peer != group.end(); peer++){
            assert((*peer)->getCommittee() == committeeId);
            assert((*peer)->getPhase() == PREPARE_WAIT); // make sure every peer got request
        }
    }

    // commit round
    refCom.receive();
    refCom.preformComputation();
    refCom.transmit();

    for(auto groupId = committeeGroups.begin(); groupId != committeeGroups.end(); groupId++){
        aGroup group = refCom.getGroup(*groupId);
        // make sure group properties are maintained
        assert(group.size() == 16);
        for(auto peer = group.begin(); peer != group.end(); peer++){
            assert((*peer)->getCommittee() == committeeId);
            assert((*peer)->getPhase() == COMMIT_WAIT); // make sure every peer is ready to commit
        }
    }

    // committee should be disolved
    refCom.receive();
    refCom.preformComputation();
    refCom.transmit();

    for(auto groupId = committeeGroups.begin(); groupId != committeeGroups.end(); groupId++){
        aGroup group = refCom.getGroup(*groupId);
        // make sure group properties are maintained
        assert(group.size() == 16);
        for(auto peer = group.begin(); peer != group.end(); peer++){
            assert((*peer)->getCommittee() == -1); // make sure they are no longer in committee
            assert((*peer)->getPhase() == IDEAL); // make sure every peer is done
            assert((*peer)->getLedger().size() == 2); // make sure every peer comited the request
        }
    }

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testRefComCommittee complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void testGlobalLedger(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testGlobalLedger"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    ///////////////////////////////////////////////////
    // small set of peers to debug
    // this is the standard set up 32 peer 16 groups 1/3 fault tolerance 
    PBFTReferenceCommittee refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(1);
    refCom.setToRandom();
    refCom.setGroupSize(2);
    refCom.setFaultTolerance(1);
    refCom.initNetwork(32);
    refCom.log();

    refCom.makeRequest();
    assert(refCom.getRequestQueue().size() == 0); // make sure quest has been serverd
    // request should be done in 5 rounds
    for(int round = 0; round < 5; round++ ){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
    }
    refCom.log();
    assert(refCom.getGlobalLedger().size() == 1); // make sure trasnaction was counted as confirmed by the sytem

    refCom.makeRequest();
    refCom.makeRequest();
    assert(refCom.getRequestQueue().size() == 0); // make sure quest has been serverd
    for(int round = 0; round < 10; round++ ){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
    }

    refCom.log();
    assert(refCom.getGlobalLedger().size() == 3); // two more transactions should be counted

    refCom.makeRequest();
    refCom.makeRequest();
    refCom.makeRequest();
    for(int round = 0; round < 15; round++ ){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
    }

    refCom.log();
    assert(refCom.getGlobalLedger().size() == 6); // three more transactions should be counted

    for(int round = 0; round < 100; round++ ){
        refCom.makeRequest();
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
    }

    refCom.log();
    assert(refCom.getGlobalLedger().size() >= 26); // min number of transactions confirmed should be 26
    assert(refCom.getGlobalLedger().size() <= 106); // max number of transactions confirmed should be 106

    ///////////////////////////////////////////////////
    // this is the standard set up 1024 peer 64 groups 1/3 fault tolerance 
    refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(1);
    refCom.setToRandom();
    refCom.setGroupSize(16);
    refCom.setFaultTolerance(0.3);
    refCom.initNetwork(1024);

    refCom.makeRequest();
    assert(refCom.getRequestQueue().size() == 0); // make sure quest has been serverd
    // request should be done in 5 rounds
    for(int round = 0; round < 5; round++ ){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
    }
    assert(refCom.getGlobalLedger().size() == 1); // make sure trasnaction was counted as confirmed by the sytem

    refCom.makeRequest();
    refCom.makeRequest();
    for(int round = 0; round < 10; round++ ){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
    }

    assert(refCom.getGlobalLedger().size() == 3); // two more transactions should be counted

    refCom.makeRequest();
    refCom.makeRequest();
    refCom.makeRequest();
    for(int round = 0; round < 15; round++ ){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
    }

    assert(refCom.getGlobalLedger().size() == 6); // three more transactions should be counted

    for(int round = 0; round < 100; round++ ){
        refCom.makeRequest();
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
    }

    assert(refCom.getGlobalLedger().size() >= 26); // min number of transactions confirmed should be 26
    assert(refCom.getGlobalLedger().size() <= 106); // max number of transactions confirmed should be 106

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testGlobalLedger complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}