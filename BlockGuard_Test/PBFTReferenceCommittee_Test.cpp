//
//  PBFTReferenceCommittee_Test.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 5/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "PBFTReferenceCommittee_Test.hpp"

//////////////////////////////////////////////////////////////////////
// this is the standard set up for number of peers, group size and fault tolerance 
static const int    GROUP_SIZE  = 4; // 32 groups for 256 peers
static const int    PEERS       = 128;
static const double FAULT       = 0.3;


void RunPBFTRefComTest (std::string filepath){
    std::ofstream log;
    log.open(filepath + "/PBFTRefCom.log");
    if (log.fail() ){
        std::cerr << "Error: could not open file at: "<< filepath << std::endl;
    }
    testInit(log);
    testRefComGroups(log);
    testRefComCommittee(log);
    testGlobalLedger(log);
    testSimultaneousRequest(log);
    testByzantineConfirmationRate(log);
    testShuffle(log);
    testByzantineVsDelay(log);
}

void testInit(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testInit"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    
    PBFTReferenceCommittee refCom = PBFTReferenceCommittee();    
    refCom.setLog(log);
    refCom.setToRandom();
    refCom.setGroupSize(8);
    refCom.setFaultTolerance(FAULT);
    refCom.initNetwork(256);

    assert(refCom.securityLevel5()          == 32);
    assert(refCom.securityLevel4()          == 16);
    assert(refCom.securityLevel3()          == 8);
    assert(refCom.securityLevel2()          == 4);
    assert(refCom.securityLevel1()          == 2);

    assert(refCom.getGroupSize()            == 8);
    assert(refCom.numberOfGroups()          == 32);
    assert(refCom.size()                    == 256);
    assert(refCom.getNextSquenceNumber()    == 0);
    assert(refCom.getNextCommitteeId()      == 0);

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testInit Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}

void testRefComGroups (std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testRefComGroups"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    // test that group size of one make equal number of peers as there are in the system
    PBFTReferenceCommittee refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(1);
    refCom.setToRandom();
    refCom.setGroupSize(1);
    refCom.setFaultTolerance(FAULT);
    refCom.initNetwork(PEERS);

    assert(refCom.numberOfGroups()              == PEERS);
    assert(refCom.getBusyGroups().size()        == 0);
    assert(refCom.getFreeGroups().size()        == PEERS);

    for(int i = 0; i < refCom.size(); i++){
        PBFTPeer_Sharded* peer = refCom[i];
        assert(peer->getGroup()                     == i);// group id starts from 0
        assert(peer->getFaultTolerance()            == FAULT);
        assert(peer->getCommittee()                 == -1); // not in a committee
        assert(peer->getGroupMembers().size()       == 0); // self is not included in list og group memebers
        assert(peer->getCommitteeMembers().size()   == 0);
    }

    // test that a group with more then one peer do not overlap
    refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(1);
    refCom.setFaultTolerance(FAULT);
    refCom.setToRandom();
    refCom.setGroupSize(GROUP_SIZE); // 64 groups
    refCom.initNetwork(PEERS);

    // test each group to make sure it was formed correctly
    for(int id = 0; id < refCom.getGroupIds().size(); id++){
        std::vector<PBFTPeer_Sharded*> group = refCom.getGroup(id);
        assert(group.size() == GROUP_SIZE);
        for(int peer = 0; peer < group.size(); peer++){
            assert(group[peer]->getGroup()                     == id);// group id starts from 0
            assert(group[peer]->getFaultTolerance()            == FAULT);
            assert(group[peer]->getCommittee()                 == -1); // not in a committee
            assert(group[peer]->getGroupMembers().size()       == GROUP_SIZE-1); // self is not included in list og group memebers
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
    // this is the standard set up 
    refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(1);
    refCom.setToRandom();
    refCom.setGroupSize(GROUP_SIZE);
    refCom.setFaultTolerance(FAULT);
    refCom.initNetwork(PEERS);
    refCom.setMaxSecurityLevel(2);
    refCom.setMinSecurityLevel(2);

    assert(refCom.getBusyGroups().size()            == 0);
    assert(refCom.getFreeGroups().size()            == PEERS/GROUP_SIZE);

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
        assert(group.size() == GROUP_SIZE);
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
        assert(group.size() == GROUP_SIZE);
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
        assert(group.size() == GROUP_SIZE);
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
        assert(group.size() == GROUP_SIZE);
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
    assert(refCom.getFreeGroups().size()            == PEERS/GROUP_SIZE);

    for(auto groupId = committeeGroups.begin(); groupId != committeeGroups.end(); groupId++){
        aGroup group = refCom.getGroup(*groupId);
        // make sure group properties are maintained
        assert(group.size() == GROUP_SIZE);
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
        assert(group.size() == GROUP_SIZE);
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
        assert(group.size() == GROUP_SIZE);
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
        assert(group.size() == GROUP_SIZE);
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
        assert(group.size() == GROUP_SIZE);
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
        assert(group.size() == GROUP_SIZE);
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

    refCom.makeRequest(refCom.securityLevel1());
    assert(refCom.getRequestQueue().size() == 0); // make sure transaction has been serverd
    // request should be done in 5 rounds
    for(int round = 0; round < 5; round++ ){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
    }
    refCom.log();
    assert(refCom.getGlobalLedger().size() == 1); // make sure trasnaction was counted as confirmed by the sytem

    refCom.makeRequest(refCom.securityLevel1());
    refCom.makeRequest(refCom.securityLevel1());
    assert(refCom.getRequestQueue().size() == 0); // make sure transaction has been serverd
    for(int round = 0; round < 10; round++ ){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
    }

    refCom.log();
    assert(refCom.getGlobalLedger().size() == 3); // two more transactions should be counted

    refCom.makeRequest(refCom.securityLevel1());
    refCom.makeRequest(refCom.securityLevel1());
    refCom.makeRequest(refCom.securityLevel1());
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
    // this is the standard set up
    refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(1);
    refCom.setToRandom();
    refCom.setGroupSize(GROUP_SIZE);
    refCom.setFaultTolerance(FAULT);
    refCom.initNetwork(PEERS);

    refCom.makeRequest(refCom.securityLevel1());
    assert(refCom.getRequestQueue().size() == 0); // make sure quest has been serverd
    // request should be done in 5 rounds
    for(int round = 0; round < 5; round++ ){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
    }
    assert(refCom.getGlobalLedger().size() == 1); // make sure trasnaction was counted as confirmed by the sytem

    refCom.makeRequest(refCom.securityLevel1());
    refCom.makeRequest(refCom.securityLevel1());
    for(int round = 0; round < 10; round++ ){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
    }

    assert(refCom.getGlobalLedger().size() == 3); // two more transactions should be counted

    refCom.makeRequest(refCom.securityLevel1());
    refCom.makeRequest(refCom.securityLevel1());
    refCom.makeRequest(refCom.securityLevel1());
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

    // range becouse this uses the geometric distribution
    assert(refCom.getGlobalLedger().size() >= 26); // min number of transactions confirmed should be 26
    assert(refCom.getGlobalLedger().size() <= 106); // max number of transactions confirmed should be 106

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testGlobalLedger complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}

void testSimultaneousRequest(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testSimultaneousRequest"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;

    PBFTReferenceCommittee refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(1);
    refCom.setToRandom();
    refCom.setGroupSize(8);
    refCom.setFaultTolerance(FAULT);
    refCom.initNetwork(256);
    refCom.log();

    // quick test for queue request with a Specific security level
    refCom.queueRequest(refCom.securityLevel5());
    refCom.queueRequest(refCom.securityLevel4());
    refCom.queueRequest(refCom.securityLevel3());
    refCom.queueRequest(refCom.securityLevel2());
    refCom.queueRequest(refCom.securityLevel1());

    assert(refCom.getRequestQueue()[0].securityLevel == refCom.securityLevel5());
    assert(refCom.getRequestQueue()[1].securityLevel == refCom.securityLevel4());
    assert(refCom.getRequestQueue()[2].securityLevel == refCom.securityLevel3());
    assert(refCom.getRequestQueue()[3].securityLevel == refCom.securityLevel2());
    assert(refCom.getRequestQueue()[4].securityLevel == refCom.securityLevel1());

    refCom.clearQueue();

    // 16 committees can go simultaneous and it takes 5 rounds for a cmmittee to process a transaction thus 32 in 10 rounds 
    //  add 16 every round 32 should go though 16 should be processing and 112 should be queued: total 16*10 = 160
    for(int i = 0; i < 10; i++){
        for(int i = 0; i < 16; i++){
            refCom.makeRequest(refCom.securityLevel1());
        }
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
    }

    assert(refCom.getGlobalLedger().size()          == 32);
    assert(refCom.getBusyGroups().size()            == 32);
    assert(refCom.getFreeGroups().size()            == 0);
    assert(refCom.getRequestQueue().size()          == 112);

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testSimultaneousRequest Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}

void testByzantineConfirmationRate(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testByzantineConfirmationRate Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    
    PBFTReferenceCommittee refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(1);
    refCom.setToRandom();
    refCom.setGroupSize(GROUP_SIZE);
    refCom.setFaultTolerance(FAULT);
    refCom.initNetwork(PEERS);
    refCom.makeByzantines(PEERS*0.1);
    refCom.log();

    // number of groups by 1/2 becouse a committee needs at least two groups
    // this should make the make number of groups the system can half at one time
    int numberOfRequests = refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    // count byz leaders
    int byzLeaders = 0;
    for(int i = 0; i < refCom.getCurrentCommittees().size()/2; i++){
        std::vector<aGroup> groupsInCom = refCom.getCommittee(refCom.getCurrentCommittees()[i]);
        for(auto group = groupsInCom.begin(); group != groupsInCom.end(); group++){
            for(auto peer = group->begin(); peer != group->end(); peer++){
                if((*peer)->isPrimary() && (*peer)->isByzantine()){
                    byzLeaders++;
                }
            }
        }
    }

    // run for 5 rounds for a view change
    for(int i = 0; i < 5; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }

    assert(refCom.getGlobalLedger().size()      == numberOfRequests-byzLeaders);
    assert(refCom.getCurrentCommittees().size() == byzLeaders); // make sure the committees that did not commit and are view changeing are still alive

    // count byz leaders
    byzLeaders = 0;
    for(int i = 0; i < refCom.getCurrentCommittees().size()/2; i++){
        std::vector<aGroup> groupsInCom = refCom.getCommittee(refCom.getCurrentCommittees()[i]);
        for(auto group = groupsInCom.begin(); group != groupsInCom.end(); group++){
            for(auto peer = group->begin(); peer != group->end(); peer++){
                if((*peer)->isPrimary() && (*peer)->isByzantine()){
                    byzLeaders++;
                }
            }
        }
    }
    
    // run for another 5 rounds and all trnasactions should be approved (makes sure non of the transactions are lost)
    log << "-- run for another 5 rounds and all trnasactions should be approved --"<< std::endl;
    for(int i = 0; i < 5; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }

    assert(refCom.getGlobalLedger().size()      == numberOfRequests-byzLeaders);
    assert(refCom.getCurrentCommittees().size() == byzLeaders); // make sure the committees that did not commit and are view changeing are still alive

    // do this until there all transactions are committed or we have gone over the upper bound of rounds possable to commit (i.e. they will never commit)
    int upperBound = PEERS*0.5;
    int i = 0;
    while(refCom.getGlobalLedger().size() < numberOfRequests ){
        i++;
        // count byz leaders
        byzLeaders = 0;
        for(int i = 0; i < refCom.getCurrentCommittees().size()/2; i++){
            std::vector<aGroup> groupsInCom = refCom.getCommittee(refCom.getCurrentCommittees()[i]);
            for(auto group = groupsInCom.begin(); group != groupsInCom.end(); group++){
                for(auto peer = group->begin(); peer != group->end(); peer++){
                    if((*peer)->isPrimary() && (*peer)->isByzantine()){
                        byzLeaders++;
                    }
                }
            }
        }
        
        // run for another 5 rounds and all trnasactions should be approved (makes sure non of the transactions are lost)
        log << "-- run for another 5 rounds and all trnasactions should be approved --"<< std::endl;
        for(int i = 0; i < 5; i++){
            refCom.receive();
            refCom.preformComputation();
            refCom.transmit();
            refCom.log();
        }
        
        assert(i                                    < upperBound);
        assert(refCom.getGlobalLedger().size()      == numberOfRequests-byzLeaders);
        assert(refCom.getCurrentCommittees().size() == byzLeaders); // make sure the committees that did not commit and are view changeing are still alive
    }

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testByzantineConfirmationRate Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}

void testShuffle(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testByzantineShuffle"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    PBFTReferenceCommittee refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(1);
    refCom.setToRandom();
    refCom.setGroupSize(GROUP_SIZE);
    refCom.setFaultTolerance(FAULT);
    refCom.initNetwork(PEERS);
    refCom.log();

    refCom.makeByzantines(PEERS*FAULT); // make enough Byzantine peers to "break" the system
    int byzantine = 0;
    int correct = 0;
    for(int i = 0; i < refCom.size(); i++){
        if(refCom[i]->isByzantine()){
            byzantine++;
        }else{
            correct++;
        }
    }

    assert(byzantine                        == int(PEERS*FAULT));
    assert(correct                          == PEERS - byzantine);
    assert(refCom.getByzantine().size()     == byzantine);
    assert(refCom.getCorrect().size()       == correct);


    refCom.makeCorrect((PEERS*FAULT)-1); // make all but one correct 
    byzantine = 0;
    correct = 0;
    for(int i = 0; i < refCom.size(); i++){
        if(refCom[i]->isByzantine()){
            byzantine++;
        }else{
            correct++;
        }
    }

    assert(byzantine                        == 1);
    assert(correct                          == PEERS - 1);
    assert(refCom.getByzantine().size()     == byzantine);
    assert(refCom.getCorrect().size()       == correct);

    refCom.makeCorrect(1); // make last one correct
    byzantine = 0;
    correct = 0;
    for(int i = 0; i < refCom.size(); i++){
        if(refCom[i]->isByzantine()){
            byzantine++;
        }else{
            correct++;
        }
    }

    assert(byzantine                        == 0);
    assert(correct                          == PEERS);
    assert(refCom.getByzantine().empty()    == true);
    assert(refCom.getCorrect().size()       == correct);

    // refresh system for new tests
    refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(1);
    refCom.setToRandom();
    refCom.setGroupSize(GROUP_SIZE);
    refCom.setFaultTolerance(FAULT);
    refCom.initNetwork(PEERS);
    refCom.log();

    refCom.makeByzantines(PEERS*0.25); // make 25% of the peers Byzantine
    byzantine = 0;
    correct = 0;
    for(int i = 0; i < refCom.size(); i++){
        if(refCom[i]->isByzantine()){
            byzantine++;
        }else{
            correct++;
        }
    }

    assert(byzantine                        == (int)PEERS*0.25);
    assert(correct                          == PEERS - byzantine);
    assert(refCom.getByzantine().size()     == byzantine);
    assert(refCom.getCorrect().size()       == correct);

    refCom.shuffleByzantines(PEERS*0.25); // now shuffle them
    byzantine = 0;
    correct = 0;
    for(int i = 0; i < refCom.size(); i++){
        if(refCom[i]->isByzantine()){
            byzantine++;
        }else{
            correct++;
        }
    }
    // make sure then number has not changed
    assert(byzantine                        == (int)PEERS*0.25);
    assert(correct                          == PEERS - byzantine);
    assert(refCom.getByzantine().size()     == byzantine);
    assert(refCom.getCorrect().size()       == correct);

    // now do this a bunch of times to make sure it holds
    for(int i = 0; i < 25; i++){
        refCom.shuffleByzantines(PEERS*0.25); // now shuffle them
        byzantine = 0;
        correct = 0;
        for(int i = 0; i < refCom.size(); i++){
            if(refCom[i]->isByzantine()){
                byzantine++;
            }else{
                correct++;
            }
        }
        // make sure then number has not changed
        assert(byzantine                        == (int)PEERS*0.25);
        assert(correct                          == PEERS - byzantine);
        assert(refCom.getByzantine().size()     == byzantine);
        assert(refCom.getCorrect().size()       == correct);
    }

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testByzantineShuffle Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}

void testByzantineVsDelay(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testByzantineVsDelay"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    /////////////////////////////////////////////
    // Delay 1
    //
    
    int Delay = 1;
    PBFTReferenceCommittee refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(Delay);
    refCom.setToRandom();
    refCom.setGroupSize(2);
    refCom.setFaultTolerance(FAULT);
    refCom.initNetwork(64);
    refCom.makeByzantines(64*0.25); // make 25% of the peers Byzantine
    refCom.log();

    
    // number of groups by 1/2 becouse a committee needs at least two groups
    // this should make the make be the nunmber of groups the system can have at one time
    int numberOfRequests = refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    // 5 phases for PBFT * whatever the delay is * the total number of byz peers (max number of view changes)
    int maxRoundsToConfirmTrans = (5*Delay)*(64*0.25);
    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone
    
    numberOfRequests += refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone
    
    numberOfRequests += refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone
    
    /////////////////////////////////////////////
    // Delay 2
    //
    Delay = 2;
    refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(Delay);
    refCom.setMinDelay(Delay);
    refCom.setToRandom();
    refCom.setGroupSize(2);
    refCom.setFaultTolerance(FAULT);
    refCom.initNetwork(64);
    refCom.makeByzantines(64*0.25); // make 25% of the peers Byzantine
    refCom.log();
    
    
    // number of groups by 1/2 becouse a committee needs at least two groups
    // this should make the make be the nunmber of groups the system can have at one time
    numberOfRequests = refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    // 5 phases for PBFT * whatever the delay is * the total number of byz peers (max number of view changes)
    maxRoundsToConfirmTrans = (5*Delay)*(64*0.25);
    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone

    numberOfRequests += refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone
    
    numberOfRequests += refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone
    
    /////////////////////////////////////////////
    // Delay 4
    //
    Delay = 4;
    refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(Delay);
    refCom.setMinDelay(Delay);
    refCom.setToRandom();
    refCom.setGroupSize(2);
    refCom.setFaultTolerance(FAULT);
    refCom.initNetwork(64);
    refCom.makeByzantines(64*0.25); // make 25% of the peers Byzantine
    refCom.log();
    
    
    // number of groups by 1/2 becouse a committee needs at least two groups
    // this should make the make be the nunmber of groups the system can have at one time
    numberOfRequests = refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    // 5 phases for PBFT * whatever the delay is * the total number of byz peers (max number of view changes)
    maxRoundsToConfirmTrans = (5*Delay)*(64*0.25);
    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone
    
    numberOfRequests += refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone
    
    numberOfRequests += refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone
    
    /////////////////////////////////////////////
    // Delay 8
    //
    Delay = 8;
    refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(Delay);
    refCom.setMinDelay(Delay);
    refCom.setToRandom();
    refCom.setGroupSize(2);
    refCom.setFaultTolerance(FAULT);
    refCom.initNetwork(64);
    refCom.makeByzantines(64*0.25); // make 25% of the peers Byzantine
    refCom.log();
    
    
    // number of groups by 1/2 becouse a committee needs at least two groups
    // this should make the make be the nunmber of groups the system can have at one time
    numberOfRequests = refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    // 5 phases for PBFT * whatever the delay is * the total number of byz peers (max number of view changes)
    maxRoundsToConfirmTrans = (5*Delay)*(64*0.25);
    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone
    
    numberOfRequests += refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone
    
    numberOfRequests += refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone
    
    /////////////////////////////////////////////
    // Delay 16
    //
    Delay = 16;
    refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(Delay);
    refCom.setMinDelay(Delay);
    refCom.setToRandom();
    refCom.setGroupSize(2);
    refCom.setFaultTolerance(FAULT);
    refCom.initNetwork(64);
    refCom.makeByzantines(64*0.25); // make 25% of the peers Byzantineas
    refCom.log();
    
    
    // number of groups by 1/2 becouse a committee needs at least two groups
    // this should make the make be the nunmber of groups the system can have at one time
    numberOfRequests = refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    // 5 phases for PBFT * whatever the delay is * the total number of byz peers (max number of view changes)
    maxRoundsToConfirmTrans = (5*Delay)*(64*0.25);
    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone
    
    // again
    numberOfRequests += refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }

    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone
    
    // and again
    numberOfRequests += refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone
    
    /////////////////////////////////////////////
    // Delay 32 with shuffle
    //
    Delay = 32;
    refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(Delay);
    refCom.setMinDelay(Delay);
    refCom.setToRandom();
    refCom.setGroupSize(2);
    refCom.setFaultTolerance(FAULT);
    refCom.initNetwork(64);
    refCom.makeByzantines(64*0.25); // make 25% of the peers Byzantine
    refCom.log();
    
    
    // number of groups by 1/2 becouse a committee needs at least two groups
    // this should make the make be the nunmber of groups the system can have at one time
    numberOfRequests = refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    // 5 phases for PBFT * whatever the delay is * the total number of byz peers (max number of view changes)
    maxRoundsToConfirmTrans = (5*Delay)*(64*0.25);
    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone
    
    numberOfRequests += refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone
    
    numberOfRequests += refCom.getGroupIds().size()/2;
    for(int i = 0; i < refCom.getGroupIds().size()/2; i++){
        refCom.makeRequest(refCom.securityLevel1());
    }
    
    for(int i = 0; i < maxRoundsToConfirmTrans; i++){
        refCom.receive();
        refCom.preformComputation();
        refCom.transmit();
        refCom.log();
    }
    
    assert(refCom.getGlobalLedger().size()      == numberOfRequests); // make sure all committees commited
    assert(refCom.getCurrentCommittees().size() == 0); // make sure the committees are gone
    
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testByzantineVsDelay Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
