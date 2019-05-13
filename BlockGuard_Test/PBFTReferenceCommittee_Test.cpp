//
//  PBFTReferenceCommittee_Test.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 5/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "PBFTReferenceCommittee_Test.hpp"

////////////////////////
// typedef fore group
typedef std::vector<PBFTPeer_Sharded*> aGroup;

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
    refCom.setFaultTolerance(0.3);
    refCom.setToRandom();
    refCom.setGroupSize(1);
    refCom.initNetwork(1024);
    refCom.log();

    assert(refCom.numberOfGroups()              == 1024);
    assert(refCom.getBusyGroups().size()        == 0);
    assert(refCom.getFreeGroups().size()        == 1024);

    for(int i = 0; i < refCom.size(); i++){
        PBFTPeer_Sharded* peer = refCom[i];
        assert(peer->getGroup()                     == i);// group id starts from 0
        assert(peer->getFaultTolerance()            == 0.3);
        assert(peer->getCommittee()                 == -1); // not in a committee
        assert(peer->getGroupMembers()[0]           == peer->id());
        assert(peer->getGroupMembers().size()       == 1);
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
    // note: we are not testing for computation correctness here only making sure thr groups dont change
    for(int i = 0; i < 100; i++){
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
void testRefComRequest (std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testRefComRequest"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testRefComRequest Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void testRefComCommittee (std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testRefComCommittee"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    // this is the standard set up 1024 peer 64 groups 1/3 fault tolerance 
    PBFTReferenceCommittee refCom = PBFTReferenceCommittee();
    refCom.setLog(log);
    refCom.setMaxDelay(1);
    refCom.setFaultTolerance(0.3);
    refCom.setToRandom();
    refCom.setGroupSize(16);
    refCom.initNetwork(1024);
    refCom.log();

    // check and make sure security levels are correct
    assert(SECURITY_LEVEL_1                         == 4);
    assert(SECURITY_LEVEL_2                         == 8);
    assert(SECURITY_LEVEL_3                         == 16);
    assert(SECURITY_LEVEL_4                         == 32);
    assert(SECURITY_LEVEL_5                         == 64);
    assert(refCom.getBusyGroups().size()            == 0);
    assert(refCom.getFreeGroups().size()            == 64);

    refCom.queueRequest();
    assert(refCom.getRequestQueue().size()          == 1);
    assert(refCom.getNestCommitteeId()              == 1);
    double securityLevel = refCom.getRequestQueue().front().securityLevel;

    refCom.makeRequest(); // this will also add a new request to the queue but only one request can be sent per round
    
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
            assert((*peer)->getPhase() == COMMIT); // make sure every peer is ready to commit
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
        }
    }
    ////////////////////////
    // repeat to make sure committees do not interfere with each other

    refCom.queueRequest();
    assert(refCom.getRequestQueue().size()          == 1);
    assert(refCom.getNestCommitteeId()              == 1);
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
            assert((*peer)->getPhase() == COMMIT); // make sure every peer is ready to commit
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
        }
    }

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testRefComCommittee complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
