//
//  PBFTPeerTest.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 4/15/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "PBFTPeerTest.hpp"
#include <iostream>

static PBFT_Peer a = PBFT_Peer("A");
static PBFT_Peer b = PBFT_Peer("B");
static PBFT_Peer c = PBFT_Peer("C");
static PBFT_Peer d = PBFT_Peer("D");
static PBFT_Peer e = PBFT_Peer("E");
static PBFT_Peer f = PBFT_Peer("F");
static PBFT_Peer g = PBFT_Peer("G");
static PBFT_Peer h = PBFT_Peer("H");
static PBFT_Peer i = PBFT_Peer("I");
static PBFT_Peer j = PBFT_Peer("J");

void constructors(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"constructors"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;

    PBFT_Peer a = PBFT_Peer("A");
    a.setLogFile(log);    
    a.log();
    // state
    assert(a.getPhase()                 == IDEAL);
    assert(a.neighbors().size()         == 0);
    assert(a.id()                       == "A");
    assert(a.isNeighbor("A")            == false);
    assert(a.isNeighbor("Z")            == false);
    assert(a.isPrimary()                == false);
    assert(a.getPrimary()               == NO_PRIMARY);
    assert(a.faultyPeers()              == 0);
    
    // logs
    assert(a.getRequestLog().size()     == 0);
    assert(a.getPrePrepareLog().size()  == 0);
    assert(a.getPrepareLog().size()     == 0);
    assert(a.getCommitLog().size()      == 0);
    assert(a.getLedger().size()         == 0);
    
    a = PBFT_Peer("A",0.3);
    a.setLogFile(log); 
    a.log();
    // state
    assert(a.getPhase()                 == IDEAL);
    assert(a.neighbors().size()         == 0);
    assert(a.id()                       == "A");
    assert(a.isNeighbor("A")            == false);
    assert(a.isNeighbor("Z")            == false);
    assert(a.isPrimary()                == false);
    assert(a.getPrimary()               == NO_PRIMARY);
    assert(a.faultyPeers()              == 1);
    
    // logs
    assert(a.getRequestLog().size()     == 0);
    assert(a.getPrePrepareLog().size()  == 0);
    assert(a.getPrepareLog().size()     == 0);
    assert(a.getCommitLog().size()      == 0);
    assert(a.getLedger().size()         == 0);

    b = PBFT_Peer(a);
    b.setLogFile(log); 
    b.log();

    // state
    assert(b.getPhase()                 == IDEAL);
    assert(b.neighbors().size()         == 0);
    assert(b.id()                       == "A");
    assert(b.isNeighbor("A")            == false);
    assert(b.isNeighbor("Z")            == false);
    assert(b.isPrimary()                == false);
    assert(a.getPrimary()               == NO_PRIMARY);
    assert(b.faultyPeers()              == 1);
    
    // logs
    assert(b.getRequestLog().size()     == 0);
    assert(b.getPrePrepareLog().size()  == 0);
    assert(b.getPrepareLog().size()     == 0);
    assert(b.getCommitLog().size()      == 0);
    assert(b.getLedger().size()         == 0);
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"constructors Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void testSettersMutators(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testSettersMutators"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;

    //////////////////////////////////////////////////////////////
    // Neighbor tests
    
    
    a = PBFT_Peer("A");
    b = PBFT_Peer("B");
    c = PBFT_Peer("C");
    a.setLogFile(log); 
    b.setLogFile(log);
    c.setLogFile(log);


    a.addNeighbor(b, 1);
    a.log();
    b.log();
    c.log();

    // state
    assert(a.neighbors().size()         == 1);
    assert(a.neighbors()[0]             == "B");
    assert(a.id()                       == "A");
    assert(a.isNeighbor("A")            == false);
    assert(a.isNeighbor("B")            == true);
    assert(a.isNeighbor("Z")            == false);
    assert(a.isPrimary()                == false);
    assert(a.getPrimary()               == NO_PRIMARY);
    assert(a.getDelayToNeighbor("B")    == 1);
    
    
    b.addNeighbor(a, 100);
    a.log();
    b.log();
    c.log();

    // state
    assert(b.neighbors().size()         == 1);
    assert(a.neighbors()[0]             == "A");
    assert(b.id()                       == "B");
    assert(b.isNeighbor("A")            == true);
    assert(b.isNeighbor("B")            == false);
    assert(b.isNeighbor("Z")            == false);
    assert(b.isPrimary()                == false);
    assert(b.getPrimary()               == NO_PRIMARY);
    assert(a.getDelayToNeighbor("A")    == 100);
    
    b.addNeighbor(a, 5);
    a.log();
    b.log();
    c.log();

    // state
    assert(a.neighbors().size()         == 1);
    assert(a.neighbors()[0]             == "A");
    assert(b.id()                       == "B");
    assert(b.isNeighbor("A")            == true);
    assert(b.isNeighbor("B")            == false);
    assert(b.isNeighbor("Z")            == false);
    assert(b.isPrimary()                == false);
    assert(b.getPrimary()               == NO_PRIMARY);
    assert(a.getDelayToNeighbor("A")    == 5);
    
    a.addNeighbor(c, 1);
    a.log();
    b.log();
    c.log();

    assert(a.neighbors().size()         == 2);
    assert(a.neighbors()[0]             == "B");
    assert(a.neighbors()[1]             == "C");
    assert(a.id()                       == "A");
    assert(a.isNeighbor("A")            == false);
    assert(a.isNeighbor("B")            == true);
    assert(a.isNeighbor("C")            == true);
    assert(a.isNeighbor("Z")            == false);
    assert(a.isPrimary()                == false);
    assert(a.getPrimary()               == NO_PRIMARY);
    assert(a.getDelayToNeighbor("C")    == 1);
    
    //////////////////////////////////////////////////////////////
    // fault tolerance tests
    
    
    a = PBFT_Peer("A");
    a.setLogFile(log); 

    a.setFaultTolerance(0.3);
    assert(a.faultyPeers()              == 1);

    a.setFaultTolerance(0.0);
    assert(a.faultyPeers()              == 0);
    
    a = PBFT_Peer("A");
    b = PBFT_Peer("B");
    c = PBFT_Peer("C");
    d = PBFT_Peer("D");
    e = PBFT_Peer("E");
    f = PBFT_Peer("F");
    g = PBFT_Peer("G");
    h = PBFT_Peer("H");
    i = PBFT_Peer("I");
    j = PBFT_Peer("J");
    a.setLogFile(log); 
    b.setLogFile(log);
    c.setLogFile(log);
    d.setLogFile(log);
    e.setLogFile(log);
    f.setLogFile(log);
    g.setLogFile(log);
    h.setLogFile(log);
    i.setLogFile(log);
    j.setLogFile(log);


    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    a.addNeighbor(d, 1);
    a.addNeighbor(e, 1);
    a.addNeighbor(f, 1);
    a.addNeighbor(g, 1);
    a.addNeighbor(h, 1);
    a.addNeighbor(i, 1);
    a.addNeighbor(j, 1);
    
    a.log();
    b.log();
    c.log();
    d.log();
    e.log();
    f.log();
    g.log();
    h.log();
    i.log();
    j.log();

    a.setFaultTolerance(0.3);
    a.log();
    assert(a.faultyPeers()              == 3);
    a.setFaultTolerance(0.0);
    a.log();
    assert(a.faultyPeers()              == 0);
    
    //////////////////////////////////////////////////////////////
    // primary tests
    
    
    c = PBFT_Peer("C");
    b = PBFT_Peer("B");
    c.setLogFile(log);
    b.setLogFile(log);
    c.addNeighbor(b, 1);
    b.addNeighbor(c, 1);
    a.log();
    b.log();
    c.log();
    
    c.init();
    b.init();
    a.log();
    b.log();
    c.log();

    assert(c.isPrimary()                == false);
    assert(c.getPrimary()               == "B");

    assert(b.isPrimary()                == true);
    assert(b.getPrimary()               == "B");
    
    c.clearPrimary();
    b.clearPrimary();
    a.log();
    b.log();
    c.log();
    
    assert(a.isPrimary()                == false);
    assert(a.getPrimary()               == NO_PRIMARY);

    assert(b.isPrimary()                == false);
    assert(b.getPrimary()               == NO_PRIMARY);

    
    c.initPrimary();
    b.initPrimary();
    a.log();
    b.log();
    c.log();

    assert(c.isPrimary()                == false);
    assert(c.getPrimary()               == "B");

    assert(b.isPrimary()                == true);
    assert(b.getPrimary()               == "B");

    
    a = PBFT_Peer("A");
    a.setLogFile(log);
    c.addNeighbor(a, 1);
    b.addNeighbor(a, 1);
    c.clearPrimary();
    b.clearPrimary();
    a.initPrimary();
    b.initPrimary();
    c.initPrimary();
    a.log();
    b.log();
    c.log();

    assert(a.isPrimary()                == true);
    assert(a.getPrimary()               == "A");

    assert(b.isPrimary()                == false);
    assert(b.getPrimary()               == "A");

    assert(c.isPrimary()                == false);
    assert(c.getPrimary()               == "A");

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testSettersMutators Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void requestFromLeader(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"requestFromLeader"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;

    a = PBFT_Peer("A");
    b = PBFT_Peer("B");
    c = PBFT_Peer("C");
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    
    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    
    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    
    a.setFaultTolerance(0.3);
    b.setFaultTolerance(0.3);
    c.setFaultTolerance(0.3);
    
    a.init();
    b.init();
    c.init();
    
    /////////////////////////////////////////////////////////////////////
    //      make the request from leader
    
    
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Making Request"<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;
    a.makeRequest();

    a.log();
    b.log();
    c.log();

    assert(a.getRequestLog().size()                 == 1);
    assert(a.getRequestLog()[0].sequenceNumber      == -1);
    assert(a.getRequestLog()[0].client_id           == "A");
    assert(a.getRequestLog()[0].view                == 0);
    assert(a.getRequestLog()[0].type                == REQUEST);
    assert(a.getRequestLog()[0].round               == 0);
    assert(a.getRequestLog()[0].phase               == IDEAL);
    assert(a.getRequestLog()[0].result              == 0);
    
    /////////////////////////////////////////////////////////////////////
    //      round one Pre-Prepare
    
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round One"<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;
    

    a.log();
    b.log();
    c.log();

    assert(a.getPhase()                             == PREPARE_WAIT);
    assert(b.getPhase()                             == IDEAL);
    assert(c.getPhase()                             == IDEAL);
    
    assert(a.getRequestLog().size()                 == 0);
    assert(a.getPrepareLog().size()                 == 1);
    assert(a.getPrepareLog()[0].sequenceNumber      == 1);
    assert(a.getPrepareLog()[0].client_id           == "A");
    assert(a.getPrepareLog()[0].view                == 0);
    assert(a.getPrepareLog()[0].type                == REPLY);
    assert(a.getPrepareLog()[0].round               == 0);
    assert(a.getPrepareLog()[0].phase               == PRE_PREPARE);
    assert(a.getPrepareLog()[0].result              == 0);
    
    a.transmit();
    b.transmit();
    c.transmit();
    
    /////////////////////////////////////////////////////////////////////
    //      round two Prepare
    
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round One"<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;    

    a.log();
    b.log();
    c.log();

    // checking prepare log
    
    assert(a.getPrepareLog().size()                 == 1);
    assert(a.getPrepareLog()[0].sequenceNumber      == 1);
    assert(a.getPrepareLog()[0].client_id           == "A");
    assert(a.getPrepareLog()[0].creator_id          == "A");
    assert(a.getPrepareLog()[0].view                == 0);
    assert(a.getPrepareLog()[0].type                == REPLY);
    assert(a.getPrepareLog()[0].round               == 0);
    assert(a.getPrepareLog()[0].phase               == PREPARE);
    assert(a.getPrepareLog()[0].result              == 0);
    
    assert(b.getPrepareLog().size()                 == 1);
    assert(b.getPrepareLog()[0].sequenceNumber      == 1);
    assert(b.getPrepareLog()[0].client_id           == "A");
    assert(b.getPrepareLog()[0].creator_id          == "B");
    assert(b.getPrepareLog()[0].view                == 0);
    assert(b.getPrepareLog()[0].type                == REPLY);
    assert(b.getPrepareLog()[0].round               == 0);
    assert(b.getPrepareLog()[0].phase               == PREPARE);
    assert(b.getPrepareLog()[0].result              == 0);
    
    assert(c.getPrepareLog().size()                 == 1);
    assert(c.getPrepareLog()[0].sequenceNumber      == 1);
    assert(c.getPrepareLog()[0].client_id           == "A");
    assert(c.getPrepareLog()[0].creator_id          == "C");
    assert(c.getPrepareLog()[0].view                == 0);
    assert(c.getPrepareLog()[0].type                == REPLY);
    assert(c.getPrepareLog()[0].round               == 0);
    assert(c.getPrepareLog()[0].phase               == PREPARE);
    assert(c.getPrepareLog()[0].result              == 0);
    
    assert(a.getPhase()                             == PREPARE_WAIT);
    assert(b.getPhase()                             == PREPARE_WAIT);
    assert(c.getPhase()                             == PREPARE_WAIT);
    
    a.transmit();
    b.transmit();
    c.transmit();
    
    /////////////////////////////////////////////////////////////////////
    //      round three commit
    
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round Three"<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;        

    a.log();
    b.log();
    c.log();

    // checking prepare log
    
    assert(a.getPrepareLog().size()                 == 3);
    assert(a.getPrepareLog()[0].sequenceNumber      == 1);
    assert(a.getPrepareLog()[0].client_id           == "A");
    assert(a.getPrepareLog()[0].creator_id          == "A");
    assert(a.getPrepareLog()[0].view                == 0);
    assert(a.getPrepareLog()[0].phase               == PREPARE);
    assert(a.getPrepareLog()[1].sequenceNumber      == 1);
    assert(a.getPrepareLog()[1].client_id           == "A");
    assert(b.getPrepareLog()[1].creator_id          == "B" ||
           b.getPrepareLog()[1].creator_id          == "C");
    assert(a.getPrepareLog()[1].view                == 0);
    assert(a.getPrepareLog()[1].phase               == PREPARE);
    assert(a.getPrepareLog()[2].sequenceNumber      == 1);
    assert(a.getPrepareLog()[2].client_id           == "A");
    assert(b.getPrepareLog()[1].creator_id          == "B" ||
           b.getPrepareLog()[1].creator_id          == "C");
    assert(a.getPrepareLog()[2].view                == 0);
    assert(a.getPrepareLog()[2].phase               == PREPARE);
    assert(a.getPrepareLog()[1].creator_id          != a.getPrepareLog()[2].creator_id);
    
    assert(b.getPrepareLog().size()                 == 3);
    assert(b.getPrepareLog()[0].sequenceNumber      == 1);
    assert(b.getPrepareLog()[0].client_id           == "A");
    assert(b.getPrepareLog()[0].creator_id          == "B");
    assert(b.getPrepareLog()[0].view                == 0);
    assert(b.getPrepareLog()[0].phase               == PREPARE);
    assert(b.getPrepareLog()[1].sequenceNumber      == 1);
    assert(b.getPrepareLog()[1].client_id           == "A");
    assert(b.getPrepareLog()[1].creator_id          == "C" ||
           b.getPrepareLog()[1].creator_id          == "B");
    assert(b.getPrepareLog()[1].view                == 0);
    assert(b.getPrepareLog()[1].phase               == PREPARE);
    assert(b.getPrepareLog()[2].sequenceNumber      == 1);
    assert(b.getPrepareLog()[2].client_id           == "A");
    assert(b.getPrepareLog()[2].creator_id          == "C" ||
           b.getPrepareLog()[2].creator_id          == "B");
    assert(b.getPrepareLog()[2].view                == 0);
    assert(b.getPrepareLog()[2].phase               == PREPARE);
    assert(b.getPrepareLog()[1].creator_id          != b.getPrepareLog()[2].creator_id);
    
    assert(c.getPrepareLog().size()                 == 3);
    assert(c.getPrepareLog()[0].sequenceNumber      == 1);
    assert(c.getPrepareLog()[0].client_id           == "A");
    assert(c.getPrepareLog()[0].creator_id          == "C");
    assert(c.getPrepareLog()[0].view                == 0);
    assert(c.getPrepareLog()[0].phase               == PREPARE);
    assert(c.getPrepareLog()[1].sequenceNumber      == 1);
    assert(c.getPrepareLog()[1].client_id           == "A");
    assert(c.getPrepareLog()[1].creator_id          == "A" ||
           c.getPrepareLog()[1].creator_id          == "B");
    assert(c.getPrepareLog()[1].view                == 0);
    assert(c.getPrepareLog()[1].phase               == PREPARE);
    assert(c.getPrepareLog()[2].sequenceNumber      == 1);
    assert(c.getPrepareLog()[2].client_id           == "A");
    assert(c.getPrepareLog()[2].creator_id          == "A" ||
           c.getPrepareLog()[2].creator_id          == "B");
    assert(c.getPrepareLog()[2].view                == 0);
    assert(c.getPrepareLog()[2].phase               == PREPARE);
    assert(c.getPrepareLog()[1].creator_id          != c.getPrepareLog()[2].creator_id);
    
    assert(a.getPhase()                             == COMMIT_WAIT);
    assert(b.getPhase()                             == COMMIT_WAIT);
    assert(c.getPhase()                             == COMMIT_WAIT);
    
    // checking commit log
    
    assert(a.getCommitLog().size()                 == 1);
    assert(a.getCommitLog()[0].sequenceNumber      == 1);
    assert(a.getCommitLog()[0].client_id           == "A");
    assert(a.getCommitLog()[0].creator_id          == "A");
    assert(a.getCommitLog()[0].view                == 0);
    assert(a.getCommitLog()[0].type                == REPLY);
    assert(a.getCommitLog()[0].round               == 0);
    assert(a.getCommitLog()[0].phase               == COMMIT);
    
    assert(b.getCommitLog().size()                 == 1);
    assert(b.getCommitLog()[0].sequenceNumber      == 1);
    assert(b.getCommitLog()[0].client_id           == "A");
    assert(b.getCommitLog()[0].creator_id          == "B");
    assert(b.getCommitLog()[0].view                == 0);
    assert(b.getCommitLog()[0].type                == REPLY);
    assert(b.getCommitLog()[0].round               == 0);
    assert(b.getCommitLog()[0].phase               == COMMIT);
    
    assert(c.getCommitLog().size()                 == 1);
    assert(c.getCommitLog()[0].sequenceNumber      == 1);
    assert(c.getCommitLog()[0].client_id           == "A");
    assert(c.getCommitLog()[0].creator_id          == "C");
    assert(c.getCommitLog()[0].view                == 0);
    assert(c.getCommitLog()[0].type                == REPLY);
    assert(c.getCommitLog()[0].round               == 0);
    assert(c.getCommitLog()[0].phase               == COMMIT);
    
    a.transmit();
    b.transmit();
    c.transmit();
    
    /////////////////////////////////////////////////////////////////////
    //      round four commited and logs cleaned leger increased
    
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round Four"<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;        
    
    a.log();
    b.log();
    c.log();

    // checking logs
    
    assert(a.getRequestLog().size()                 == 0);
    assert(a.getPrePrepareLog().size()              == 0);
    assert(a.getPrepareLog().size()                 == 0);
    assert(a.getCommitLog().size()                  == 0);
    assert(a.getLedger().size()                     == 1);
    assert(a.getLedger()[0].sequenceNumber          == 1);
    assert(a.getLedger()[0].client_id               == "A");
    assert(a.getLedger()[0].sequenceNumber          == 1);
    assert(a.getLedger()[0].type                    == REPLY);
    assert(a.getLedger()[0].round                   == 4);
    assert(a.getLedger()[0].client_id               == "A");
    
    assert(b.getRequestLog().size()                 == 0);
    assert(b.getPrePrepareLog().size()              == 0);
    assert(b.getPrepareLog().size()                 == 0);
    assert(b.getCommitLog().size()                  == 0);
    assert(b.getLedger().size()                     == 1);
    assert(b.getLedger()[0].sequenceNumber          == 1);
    assert(b.getLedger()[0].client_id               == "A");
    assert(b.getLedger()[0].sequenceNumber          == 1);
    assert(b.getLedger()[0].type                    == REPLY);
    assert(b.getLedger()[0].round                   == 4);
    assert(b.getLedger()[0].client_id               == "A");
    
    assert(c.getRequestLog().size()                 == 0);
    assert(c.getPrePrepareLog().size()              == 0);
    assert(c.getPrepareLog().size()                 == 0);
    assert(c.getCommitLog().size()                  == 0);
    assert(c.getLedger().size()                     == 1);
    assert(c.getLedger()[0].sequenceNumber          == 1);
    assert(c.getLedger()[0].client_id               == "A");
    assert(c.getLedger()[0].sequenceNumber          == 1);
    assert(c.getLedger()[0].type                    == REPLY);
    assert(c.getLedger()[0].round                   == 4);
    assert(c.getLedger()[0].client_id               == "A");
    
    assert(a.getPhase()                             == IDEAL);
    assert(b.getPhase()                             == IDEAL);
    assert(c.getPhase()                             == IDEAL);
    
    /////////////////////////////////////////////////////////////////////
    //      check to make sure nothing happens durring ideal time
    
    
    for(int i = 0; i < 10000; i++){
        a.receive();
        b.receive();
        c.receive();
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round "<< std::to_string(i)<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;        

        a.transmit();
        b.transmit();
        c.transmit();
        
        a.log();
        b.log();
        c.log();

        assert(a.getRequestLog().size()                 == 0);
        assert(a.getPrePrepareLog().size()              == 0);
        assert(a.getPrepareLog().size()                 == 0);
        assert(a.getCommitLog().size()                  == 0);
        assert(a.getLedger().size()                     == 1);
        assert(a.getLedger()[0].sequenceNumber          == 1);
        assert(a.getLedger()[0].client_id               == "A");
        assert(a.getLedger()[0].sequenceNumber          == 1);
        assert(a.getLedger()[0].type                    == REPLY);
        assert(a.getLedger()[0].round                   == 4);
        assert(a.getLedger()[0].client_id               == "A");
        
        assert(b.getRequestLog().size()                 == 0);
        assert(b.getPrePrepareLog().size()              == 0);
        assert(b.getPrepareLog().size()                 == 0);
        assert(b.getCommitLog().size()                  == 0);
        assert(b.getLedger().size()                     == 1);
        assert(b.getLedger()[0].sequenceNumber          == 1);
        assert(b.getLedger()[0].client_id               == "A");
        assert(b.getLedger()[0].sequenceNumber          == 1);
        assert(b.getLedger()[0].type                    == REPLY);
        assert(b.getLedger()[0].round                   == 4);
        assert(b.getLedger()[0].client_id               == "A");
        
        assert(c.getRequestLog().size()                 == 0);
        assert(c.getPrePrepareLog().size()              == 0);
        assert(c.getPrepareLog().size()                 == 0);
        assert(c.getCommitLog().size()                  == 0);
        assert(c.getLedger().size()                     == 1);
        assert(c.getLedger()[0].sequenceNumber          == 1);
        assert(c.getLedger()[0].client_id               == "A");
        assert(c.getLedger()[0].sequenceNumber          == 1);
        assert(c.getLedger()[0].type                    == REPLY);
        assert(c.getLedger()[0].round                   == 4);
        assert(c.getLedger()[0].client_id               == "A");
        
        assert(a.getPhase()                             == IDEAL);
        assert(b.getPhase()                             == IDEAL);
        assert(c.getPhase()                             == IDEAL);
    }
}
void requestFromPeer(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"requestFromPeer"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;

    a = PBFT_Peer("A");
    b = PBFT_Peer("B");
    c = PBFT_Peer("C");
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);

    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    
    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    
    a.setFaultTolerance(0.3);
    b.setFaultTolerance(0.3);
    c.setFaultTolerance(0.3);
    
    a.init();
    b.init();
    c.init();

    /////////////////////////////////////////////////////////////////////
    //      make the request to leader from another peer
    
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Making Request"<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;    
    b.makeRequest();

    a.log();
    b.log();
    c.log();

    assert(b.getRequestLog().size()                 == 1);
    assert(b.getRequestLog()[0].sequenceNumber      == -1);
    assert(b.getRequestLog()[0].client_id           == "B");
    assert(b.getRequestLog()[0].view                == 0);
    assert(b.getRequestLog()[0].type                == REQUEST);
    assert(b.getRequestLog()[0].round               == 0);
    assert(b.getRequestLog()[0].phase               == IDEAL);
    assert(b.getRequestLog()[0].result              == 0);
    
    /////////////////////////////////////////////////////////////////////
    //        round one submit request
    
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round One"<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;    

    
    a.log();
    b.log();
    c.log();

    assert(a.getPhase()                             == IDEAL);
    assert(b.getPhase()                             == IDEAL);
    assert(c.getPhase()                             == IDEAL);
    
    a.transmit();
    b.transmit();
    c.transmit();
    
    /////////////////////////////////////////////////////////////////////
    //        round two request should hit leader
    a.receive();
    b.receive();
    c.receive();

    assert(a.getRequestLog().size()                 == 1);
    
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round Two"<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;    

    
    a.log();
    b.log();
    c.log();

    assert(a.getPhase()                             == PREPARE_WAIT);
    assert(b.getPhase()                             == IDEAL);
    assert(c.getPhase()                             == IDEAL);
    
    assert(a.getRequestLog().size()                 == 0);
    assert(a.getPrepareLog().size()                 == 1);
    assert(a.getPrepareLog()[0].sequenceNumber      == 1);
    assert(a.getPrepareLog()[0].client_id           == "A");
    assert(a.getPrepareLog()[0].view                == 0);
    assert(a.getPrepareLog()[0].type                == REPLY);
    assert(a.getPrepareLog()[0].round               == 0);
    assert(a.getPrepareLog()[0].phase               == PRE_PREPARE);
    assert(a.getPrepareLog()[0].result              == 0);
    
    a.transmit();
    b.transmit();
    c.transmit();
    
    /////////////////////////////////////////////////////////////////////
    //      round three Prepare
    
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round Three"<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;    

    
    a.log();
    b.log();
    c.log();

    // checking prepare log
    
    assert(a.getPrepareLog().size()                 == 1);
    assert(a.getPrepareLog()[0].sequenceNumber      == 1);
    assert(a.getPrepareLog()[0].client_id           == "B");
    assert(a.getPrepareLog()[0].creator_id          == "A");
    assert(a.getPrepareLog()[0].view                == 0);
    assert(a.getPrepareLog()[0].type                == REPLY);
    assert(a.getPrepareLog()[0].round               == 0);
    assert(a.getPrepareLog()[0].phase               == PREPARE);
    assert(a.getPrepareLog()[0].result              == 0);
    
    assert(b.getPrepareLog().size()                 == 1);
    assert(b.getPrepareLog()[0].sequenceNumber      == 1);
    assert(b.getPrepareLog()[0].client_id           == "B");
    assert(b.getPrepareLog()[0].creator_id          == "B");
    assert(b.getPrepareLog()[0].view                == 0);
    assert(b.getPrepareLog()[0].type                == REPLY);
    assert(b.getPrepareLog()[0].round               == 0);
    assert(b.getPrepareLog()[0].phase               == PREPARE);
    assert(b.getPrepareLog()[0].result              == 0);
    
    assert(c.getPrepareLog().size()                 == 1);
    assert(c.getPrepareLog()[0].sequenceNumber      == 1);
    assert(c.getPrepareLog()[0].client_id           == "B");
    assert(c.getPrepareLog()[0].creator_id          == "C");
    assert(c.getPrepareLog()[0].view                == 0);
    assert(c.getPrepareLog()[0].type                == REPLY);
    assert(c.getPrepareLog()[0].round               == 0);
    assert(c.getPrepareLog()[0].phase               == PREPARE);
    assert(c.getPrepareLog()[0].result              == 0);
    
    assert(a.getPhase()                             == PREPARE_WAIT);
    assert(b.getPhase()                             == PREPARE_WAIT);
    assert(c.getPhase()                             == PREPARE_WAIT);
    
    a.transmit();
    b.transmit();
    c.transmit();
    
    /////////////////////////////////////////////////////////////////////
    //      round four Commit
    
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round Four"<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;    
    

    a.log();
    b.log();
    c.log();

    // checking prepare log
    
    assert(a.getPrepareLog().size()                 == 3);
    assert(a.getPrepareLog()[0].sequenceNumber      == 1);
    assert(a.getPrepareLog()[0].client_id           == "B");
    assert(a.getPrepareLog()[0].creator_id          == "A");
    assert(a.getPrepareLog()[0].view                == 0);
    assert(a.getPrepareLog()[0].phase               == PREPARE);
    assert(a.getPrepareLog()[1].sequenceNumber      == 1);
    assert(a.getPrepareLog()[1].client_id           == "B");
    assert(b.getPrepareLog()[1].creator_id          == "B" ||
           b.getPrepareLog()[1].creator_id          == "C");
    assert(a.getPrepareLog()[1].view                == 0);
    assert(a.getPrepareLog()[1].phase               == PREPARE);
    assert(a.getPrepareLog()[2].sequenceNumber      == 1);
    assert(a.getPrepareLog()[2].client_id           == "B");
    assert(b.getPrepareLog()[2].creator_id          == "B" ||
           b.getPrepareLog()[2].creator_id          == "C");
    assert(a.getPrepareLog()[2].view                == 0);
    assert(a.getPrepareLog()[2].phase               == PREPARE);
    assert(a.getPrepareLog()[1].creator_id          != a.getPrepareLog()[2].creator_id);
    
    assert(b.getPrepareLog().size()                 == 3);
    assert(b.getPrepareLog()[0].sequenceNumber      == 1);
    assert(b.getPrepareLog()[0].client_id           == "B");
    assert(b.getPrepareLog()[0].creator_id          == "B");
    assert(b.getPrepareLog()[0].view                == 0);
    assert(b.getPrepareLog()[0].phase               == PREPARE);
    assert(b.getPrepareLog()[1].sequenceNumber      == 1);
    assert(b.getPrepareLog()[1].client_id           == "B");
    assert(b.getPrepareLog()[1].creator_id          == "C" ||
           b.getPrepareLog()[1].creator_id          == "B");
    assert(b.getPrepareLog()[1].view                == 0);
    assert(b.getPrepareLog()[1].phase               == PREPARE);
    assert(b.getPrepareLog()[2].sequenceNumber      == 1);
    assert(b.getPrepareLog()[2].client_id           == "B");
    assert(b.getPrepareLog()[2].creator_id          == "C" ||
           b.getPrepareLog()[2].creator_id          == "B");
    assert(b.getPrepareLog()[2].view                == 0);
    assert(b.getPrepareLog()[2].phase               == PREPARE);
    assert(b.getPrepareLog()[1].creator_id          != b.getPrepareLog()[2].creator_id);
    
    assert(c.getPrepareLog().size()                 == 3);
    assert(c.getPrepareLog()[0].sequenceNumber      == 1);
    assert(c.getPrepareLog()[0].client_id           == "B");
    assert(c.getPrepareLog()[0].creator_id          == "C");
    assert(c.getPrepareLog()[0].view                == 0);
    assert(c.getPrepareLog()[0].phase               == PREPARE);
    assert(c.getPrepareLog()[1].sequenceNumber      == 1);
    assert(c.getPrepareLog()[1].client_id           == "B");
    assert(c.getPrepareLog()[1].creator_id          == "A" ||
           c.getPrepareLog()[1].creator_id          == "B");
    assert(c.getPrepareLog()[1].view                == 0);
    assert(c.getPrepareLog()[1].phase               == PREPARE);
    assert(c.getPrepareLog()[2].sequenceNumber      == 1);
    assert(c.getPrepareLog()[2].client_id           == "B");
    assert(c.getPrepareLog()[2].creator_id          == "A" ||
           c.getPrepareLog()[2].creator_id          == "B");
    assert(c.getPrepareLog()[2].view                == 0);
    assert(c.getPrepareLog()[2].phase               == PREPARE);
    assert(c.getPrepareLog()[1].creator_id          != c.getPrepareLog()[2].creator_id);
    
    assert(a.getPhase()                             == COMMIT_WAIT);
    assert(b.getPhase()                             == COMMIT_WAIT);
    assert(c.getPhase()                             == COMMIT_WAIT);
    
    // checking commit log
    
    assert(a.getCommitLog().size()                 == 1);
    assert(a.getCommitLog()[0].sequenceNumber      == 1);
    assert(a.getCommitLog()[0].client_id           == "A");
    assert(a.getCommitLog()[0].creator_id          == "A");
    assert(a.getCommitLog()[0].view                == 0);
    assert(a.getCommitLog()[0].type                == REPLY);
    assert(a.getCommitLog()[0].round               == 0);
    assert(a.getCommitLog()[0].phase               == COMMIT);
    
    assert(b.getCommitLog().size()                 == 1);
    assert(b.getCommitLog()[0].sequenceNumber      == 1);
    assert(b.getCommitLog()[0].client_id           == "A");
    assert(b.getCommitLog()[0].creator_id          == "B");
    assert(b.getCommitLog()[0].view                == 0);
    assert(b.getCommitLog()[0].type                == REPLY);
    assert(b.getCommitLog()[0].round               == 0);
    assert(b.getCommitLog()[0].phase               == COMMIT);
    
    assert(c.getCommitLog().size()                 == 1);
    assert(c.getCommitLog()[0].sequenceNumber      == 1);
    assert(c.getCommitLog()[0].client_id           == "A");
    assert(c.getCommitLog()[0].creator_id          == "C");
    assert(c.getCommitLog()[0].view                == 0);
    assert(c.getCommitLog()[0].type                == REPLY);
    assert(c.getCommitLog()[0].round               == 0);
    assert(c.getCommitLog()[0].phase               == COMMIT);
    
    a.transmit();
    b.transmit();
    c.transmit();
    
    /////////////////////////////////////////////////////////////////////
    //      round five commited and logs cleaned leger increased
    
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round Five"<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;    
    
    a.log();
    b.log();
    c.log();

    // checking logs
    
    assert(a.getRequestLog().size()                 == 0);
    assert(a.getPrePrepareLog().size()              == 0);
    assert(a.getPrepareLog().size()                 == 0);
    assert(a.getCommitLog().size()                  == 0);
    assert(a.getLedger().size()                     == 1);
    assert(a.getLedger()[0].sequenceNumber          == 1);
    assert(a.getLedger()[0].client_id               == "B");
    assert(a.getLedger()[0].sequenceNumber          == 1);
    assert(a.getLedger()[0].type                    == REPLY);
    assert(a.getLedger()[0].round                   == 4);
    assert(a.getLedger()[0].client_id               == "A");
    
    assert(b.getRequestLog().size()                 == 0);
    assert(b.getPrePrepareLog().size()              == 0);
    assert(b.getPrepareLog().size()                 == 0);
    assert(b.getCommitLog().size()                  == 0);
    assert(b.getLedger().size()                     == 1);
    assert(b.getLedger()[0].sequenceNumber          == 1);
    assert(b.getLedger()[0].client_id               == "B");
    assert(b.getLedger()[0].sequenceNumber          == 1);
    assert(b.getLedger()[0].type                    == REPLY);
    assert(b.getLedger()[0].round                   == 4);
    assert(b.getLedger()[0].client_id               == "A");
    
    assert(c.getRequestLog().size()                 == 0);
    assert(c.getPrePrepareLog().size()              == 0);
    assert(c.getPrepareLog().size()                 == 0);
    assert(c.getCommitLog().size()                  == 0);
    assert(c.getLedger().size()                     == 1);
    assert(c.getLedger()[0].sequenceNumber          == 1);
    assert(c.getLedger()[0].client_id               == "B");
    assert(c.getLedger()[0].sequenceNumber          == 1);
    assert(c.getLedger()[0].type                    == REPLY);
    assert(c.getLedger()[0].round                   == 4);
    assert(c.getLedger()[0].client_id               == "A");
    
    assert(a.getPhase()                             == IDEAL);
    assert(b.getPhase()                             == IDEAL);
    assert(c.getPhase()                             == IDEAL);
    
    /////////////////////////////////////////////////////////////////////
    //      check to make sure nothing happens durring ideal time
    
    for(int i = 0; i < 10000; i++){
        a.receive();
        b.receive();
        c.receive();
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round "<< std::to_string(i)<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;    

        a.log();
        b.log();
        c.log();

        a.transmit();
        b.transmit();
        c.transmit();
        
        assert(a.getRequestLog().size()                 == 0);
        assert(a.getPrePrepareLog().size()              == 0);
        assert(a.getPrepareLog().size()                 == 0);
        assert(a.getCommitLog().size()                  == 0);
        assert(a.getLedger().size()                     == 1);
        assert(a.getLedger()[0].sequenceNumber          == 1);
        assert(a.getLedger()[0].client_id               == "A");
        assert(a.getLedger()[0].sequenceNumber          == 1);
        assert(a.getLedger()[0].type                    == REPLY);
        assert(a.getLedger()[0].round                   == 4);
        assert(a.getLedger()[0].client_id               == "A");
        
        assert(b.getRequestLog().size()                 == 0);
        assert(b.getPrePrepareLog().size()              == 0);
        assert(b.getPrepareLog().size()                 == 0);
        assert(b.getCommitLog().size()                  == 0);
        assert(b.getLedger().size()                     == 1);
        assert(b.getLedger()[0].sequenceNumber          == 1);
        assert(b.getLedger()[0].client_id               == "A");
        assert(b.getLedger()[0].sequenceNumber          == 1);
        assert(b.getLedger()[0].type                    == REPLY);
        assert(b.getLedger()[0].round                   == 4);
        assert(b.getLedger()[0].client_id               == "A");
        
        assert(c.getRequestLog().size()                 == 0);
        assert(c.getPrePrepareLog().size()              == 0);
        assert(c.getPrepareLog().size()                 == 0);
        assert(c.getCommitLog().size()                  == 0);
        assert(c.getLedger().size()                     == 1);
        assert(c.getLedger()[0].sequenceNumber          == 1);
        assert(c.getLedger()[0].client_id               == "A");
        assert(c.getLedger()[0].sequenceNumber          == 1);
        assert(c.getLedger()[0].type                    == REPLY);
        assert(c.getLedger()[0].round                   == 4);
        assert(c.getLedger()[0].client_id               == "A");
        
        assert(a.getPhase()                             == IDEAL);
        assert(b.getPhase()                             == IDEAL);
        assert(c.getPhase()                             == IDEAL);
    }
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"requestFromPeer Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;    
}
void multiRequest(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"multiRequest"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;

    a = PBFT_Peer("A");
    b = PBFT_Peer("B");
    c = PBFT_Peer("C");
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);

    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    
    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    
    a.setFaultTolerance(0.3);
    b.setFaultTolerance(0.3);
    c.setFaultTolerance(0.3);
    
    a.init();
    b.init();
    c.init();

    /////////////////////////////////////////////////////////////////////
    //      make first request
    
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"making first request"<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;
    a.makeRequest();
    a.log();

    assert(a.getRequestLog().size()                 == 1);
    assert(a.getRequestLog()[0].sequenceNumber      == -1);
    assert(a.getRequestLog()[0].client_id           == "A");
    assert(a.getRequestLog()[0].view                == 0);
    assert(a.getRequestLog()[0].type                == REQUEST);
    assert(a.getRequestLog()[0].round               == 0);
    assert(a.getRequestLog()[0].phase               == IDEAL);
    assert(a.getRequestLog()[0].result              == 0);
    
    /////////////////////////////////////////////////////////////////////
    //      round one first request hits leader next is sent

    a.receive();
    b.receive();
    c.receive();

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round Zero"<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;
    b.makeRequest();
    assert(b.getRequestLog().size()                 == 1);
    assert(b.getRequestLog()[0].sequenceNumber      == -1);
    assert(b.getRequestLog()[0].client_id           == "B");
    assert(b.getRequestLog()[0].view                == 0);
    assert(b.getRequestLog()[0].type                == REQUEST);
    assert(b.getRequestLog()[0].round               == 0);
    assert(b.getRequestLog()[0].phase               == IDEAL);
    assert(b.getRequestLog()[0].result              == 0);

    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round One"<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;
    a.log();
    b.log();
    c.log();
    
    assert(a.getPhase()                             == PREPARE_WAIT);
    assert(b.getPhase()                             == IDEAL);
    assert(c.getPhase()                             == IDEAL);

    assert(a.getPrepareLog().size()                 == 1);
    assert(a.getPrepareLog()[0].sequenceNumber      == 1);
    assert(a.getPrepareLog()[0].client_id           == "A");
    assert(a.getPrepareLog()[0].view                == 0);
    assert(a.getPrepareLog()[0].type                == REPLY);
    assert(a.getPrepareLog()[0].round               == 0);
    assert(a.getPrepareLog()[0].phase               == PRE_PREPARE);
    assert(a.getPrepareLog()[0].result              == 0);
    
    a.transmit();
    b.transmit();
    c.transmit();
        
    assert(a.getRequestLog().size()                 == 1);
    assert(a.getRequestLog()[0].sequenceNumber      == -1);
    assert(a.getRequestLog()[0].client_id           == "B");
    assert(a.getRequestLog()[0].view                == 0);
    assert(a.getRequestLog()[0].type                == REQUEST);
    assert(a.getRequestLog()[0].round               == 0);
    assert(a.getRequestLog()[0].phase               == IDEAL);
    assert(a.getRequestLog()[0].result              == 0);
    
    /////////////////////////////////////////////////////////////////////
    //      round two Prepare make another request, send, and queue it
    
    
    a.receive();
    b.receive();
    c.receive();

    c.makeRequest();
    assert(c.getRequestLog().size()                 == 1);
    assert(c.getRequestLog()[0].sequenceNumber      == -1);
    assert(c.getRequestLog()[0].client_id           == "C");
    assert(c.getRequestLog()[0].view                == 0);
    assert(c.getRequestLog()[0].type                == REQUEST);
    assert(c.getRequestLog()[0].round               == 1);
    assert(c.getRequestLog()[0].phase               == IDEAL);
    assert(c.getRequestLog()[0].result              == 0);
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round Two"<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    a.log();
    b.log();
    c.log();

    // checking prepare log
    
    assert(a.getPrepareLog().size()                 == 1);
    assert(a.getPrepareLog()[0].sequenceNumber      == 1);
    assert(a.getPrepareLog()[0].client_id           == "A");
    assert(a.getPrepareLog()[0].creator_id          == "A");
    assert(a.getPrepareLog()[0].view                == 0);
    assert(a.getPrepareLog()[0].type                == REPLY);
    assert(a.getPrepareLog()[0].round               == 0);
    assert(a.getPrepareLog()[0].phase               == PREPARE);
    assert(a.getPrepareLog()[0].result              == 0);
    
    assert(b.getPrepareLog().size()                 == 1);
    assert(b.getPrepareLog()[0].sequenceNumber      == 1);
    assert(b.getPrepareLog()[0].client_id           == "A");
    assert(b.getPrepareLog()[0].creator_id          == "B");
    assert(b.getPrepareLog()[0].view                == 0);
    assert(b.getPrepareLog()[0].type                == REPLY);
    assert(b.getPrepareLog()[0].round               == 0);
    assert(b.getPrepareLog()[0].phase               == PREPARE);
    assert(b.getPrepareLog()[0].result              == 0);
    
    assert(c.getPrepareLog().size()                 == 1);
    assert(c.getPrepareLog()[0].sequenceNumber      == 1);
    assert(c.getPrepareLog()[0].client_id           == "A");
    assert(c.getPrepareLog()[0].creator_id          == "C");
    assert(c.getPrepareLog()[0].view                == 0);
    assert(c.getPrepareLog()[0].type                == REPLY);
    assert(c.getPrepareLog()[0].round               == 0);
    assert(c.getPrepareLog()[0].phase               == PREPARE);
    assert(c.getPrepareLog()[0].result              == 0);
    
    assert(a.getPhase()                             == PREPARE_WAIT);
    assert(b.getPhase()                             == PREPARE_WAIT);
    assert(c.getPhase()                             == PREPARE_WAIT);
    
    a.transmit();
    b.transmit();
    c.transmit();

    assert(a.getRequestLog().size()                 == 2);
    assert(a.getRequestLog()[0].sequenceNumber      == -1);
    assert(a.getRequestLog()[0].client_id           == "B");
    assert(a.getRequestLog()[0].view                == 0);
    assert(a.getRequestLog()[0].type                == REQUEST);
    assert(a.getRequestLog()[0].round               == 0);
    assert(a.getRequestLog()[0].phase               == IDEAL);
    assert(a.getRequestLog()[0].result              == 0);
    assert(a.getRequestLog()[1].sequenceNumber      == -1);
    assert(a.getRequestLog()[1].client_id           == "C");
    assert(a.getRequestLog()[1].view                == 0);
    assert(a.getRequestLog()[1].type                == REQUEST);
    assert(a.getRequestLog()[1].round               == 1);
    assert(a.getRequestLog()[1].phase               == IDEAL);
    assert(a.getRequestLog()[1].result              == 0);
    
    /////////////////////////////////////////////////////////////////////
    //      round three commit and make another request
    
    
    a.receive();
    b.receive();
    c.receive();

    c.makeRequest();
    assert(c.getRequestLog().size()                 == 1);
    assert(c.getRequestLog()[0].sequenceNumber      == -1);
    assert(c.getRequestLog()[0].client_id           == "C");
    assert(c.getRequestLog()[0].view                == 0);
    assert(c.getRequestLog()[0].type                == REQUEST);
    assert(c.getRequestLog()[0].round               == 2);
    assert(c.getRequestLog()[0].phase               == IDEAL);
    assert(c.getRequestLog()[0].result              == 0);

    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round Three"<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;

    a.log();
    b.log();
    c.log();

    // checking prepare log
    
    assert(a.getPrepareLog().size()                 == 3);
    assert(a.getPrepareLog()[0].sequenceNumber      == 1);
    assert(a.getPrepareLog()[0].client_id           == "A");
    assert(a.getPrepareLog()[0].creator_id          == "A");
    assert(a.getPrepareLog()[0].view                == 0);
    assert(a.getPrepareLog()[0].phase               == PREPARE);
    assert(a.getPrepareLog()[1].sequenceNumber      == 1);
    assert(a.getPrepareLog()[1].client_id           == "A");
    assert(a.getPrepareLog()[1].creator_id          == "B");
    assert(a.getPrepareLog()[1].view                == 0);
    assert(a.getPrepareLog()[1].phase               == PREPARE);
    assert(a.getPrepareLog()[2].sequenceNumber      == 1);
    assert(a.getPrepareLog()[2].client_id           == "A");
    assert(a.getPrepareLog()[2].creator_id          == "C");
    assert(a.getPrepareLog()[2].view                == 0);
    assert(a.getPrepareLog()[2].phase               == PREPARE);
    assert(a.getPrepareLog()[1].creator_id          != a.getPrepareLog()[2].creator_id);
    
    assert(b.getPrepareLog().size()                 == 3);
    assert(b.getPrepareLog()[0].sequenceNumber      == 1);
    assert(b.getPrepareLog()[0].client_id           == "A");
    assert(b.getPrepareLog()[0].creator_id          == "B");
    assert(b.getPrepareLog()[0].view                == 0);
    assert(b.getPrepareLog()[0].phase               == PREPARE);
    assert(b.getPrepareLog()[1].sequenceNumber      == 1);
    assert(b.getPrepareLog()[1].client_id           == "A");
    assert(b.getPrepareLog()[1].creator_id          == "C" ||
           b.getPrepareLog()[1].creator_id          == "B");
    assert(b.getPrepareLog()[1].view                == 0);
    assert(b.getPrepareLog()[1].phase               == PREPARE);
    assert(b.getPrepareLog()[2].sequenceNumber      == 1);
    assert(b.getPrepareLog()[2].client_id           == "A");
    assert(b.getPrepareLog()[2].creator_id          == "C" ||
           b.getPrepareLog()[2].creator_id          == "B");
    assert(b.getPrepareLog()[2].view                == 0);
    assert(b.getPrepareLog()[2].phase               == PREPARE);
    assert(b.getPrepareLog()[1].creator_id          != b.getPrepareLog()[2].creator_id);
    
    assert(c.getPrepareLog().size()                 == 3);
    assert(c.getPrepareLog()[0].sequenceNumber      == 1);
    assert(c.getPrepareLog()[0].client_id           == "A");
    assert(c.getPrepareLog()[0].creator_id          == "C");
    assert(c.getPrepareLog()[0].view                == 0);
    assert(c.getPrepareLog()[0].phase               == PREPARE);
    assert(c.getPrepareLog()[1].sequenceNumber      == 1);
    assert(c.getPrepareLog()[1].client_id           == "A");
    assert(c.getPrepareLog()[1].creator_id          == "A" ||
           c.getPrepareLog()[1].creator_id          == "B");
    assert(c.getPrepareLog()[1].view                == 0);
    assert(c.getPrepareLog()[1].phase               == PREPARE);
    assert(c.getPrepareLog()[2].sequenceNumber      == 1);
    assert(c.getPrepareLog()[2].client_id           == "A");
    assert(c.getPrepareLog()[2].creator_id          == "A" ||
           c.getPrepareLog()[2].creator_id          == "B");
    assert(c.getPrepareLog()[2].view                == 0);
    assert(c.getPrepareLog()[2].phase               == PREPARE);
    assert(c.getPrepareLog()[1].creator_id          != c.getPrepareLog()[2].creator_id);
    
    assert(a.getPhase()                             == COMMIT_WAIT);
    assert(b.getPhase()                             == COMMIT_WAIT);
    assert(c.getPhase()                             == COMMIT_WAIT);
    
    // checking commit log
    
    assert(a.getCommitLog().size()                 == 1);
    assert(a.getCommitLog()[0].sequenceNumber      == 1);
    assert(a.getCommitLog()[0].client_id           == "A");
    assert(a.getCommitLog()[0].creator_id          == "A");
    assert(a.getCommitLog()[0].view                == 0);
    assert(a.getCommitLog()[0].type                == REPLY);
    assert(a.getCommitLog()[0].round               == 0);
    assert(a.getCommitLog()[0].phase               == COMMIT);
    
    assert(b.getCommitLog().size()                 == 1);
    assert(b.getCommitLog()[0].sequenceNumber      == 1);
    assert(b.getCommitLog()[0].client_id           == "A");
    assert(b.getCommitLog()[0].creator_id          == "B");
    assert(b.getCommitLog()[0].view                == 0);
    assert(b.getCommitLog()[0].type                == REPLY);
    assert(b.getCommitLog()[0].round               == 0);
    assert(b.getCommitLog()[0].phase               == COMMIT);
    
    assert(c.getCommitLog().size()                 == 1);
    assert(c.getCommitLog()[0].sequenceNumber      == 1);
    assert(c.getCommitLog()[0].client_id           == "A");
    assert(c.getCommitLog()[0].creator_id          == "C");
    assert(c.getCommitLog()[0].view                == 0);
    assert(c.getCommitLog()[0].type                == REPLY);
    assert(c.getCommitLog()[0].round               == 0);
    assert(c.getCommitLog()[0].phase               == COMMIT);
    
    a.transmit();
    b.transmit();
    c.transmit();

    assert(a.getRequestLog().size()                 == 3);
    assert(a.getRequestLog()[0].sequenceNumber      == -1);
    assert(a.getRequestLog()[0].client_id           == "B");
    assert(a.getRequestLog()[0].view                == 0);
    assert(a.getRequestLog()[0].type                == REQUEST);
    assert(a.getRequestLog()[0].round               == 0);
    assert(a.getRequestLog()[0].phase               == IDEAL);
    assert(a.getRequestLog()[0].result              == 0);
    assert(a.getRequestLog()[1].sequenceNumber      == -1);
    assert(a.getRequestLog()[1].client_id           == "C");
    assert(a.getRequestLog()[1].view                == 0);
    assert(a.getRequestLog()[1].type                == REQUEST);
    assert(a.getRequestLog()[1].round               == 1);
    assert(a.getRequestLog()[1].phase               == IDEAL);
    assert(a.getRequestLog()[1].result              == 0);
    assert(a.getRequestLog()[2].sequenceNumber      == -2);
    assert(a.getRequestLog()[2].client_id           == "C");
    assert(a.getRequestLog()[2].view                == 0);
    assert(a.getRequestLog()[2].type                == REQUEST);
    assert(a.getRequestLog()[2].round               == 2);
    assert(a.getRequestLog()[2].phase               == IDEAL);
    assert(a.getRequestLog()[2].result              == 0);
    
    /////////////////////////////////////////////////////////////////////
    //      round four commited and logs cleaned leger increased
    //      next request is proposed, make another
    
    
    a.receive();
    b.receive();
    c.receive();

    b.makeRequest();
    assert(b.getRequestLog().size()                 == 1);
    assert(b.getRequestLog()[0].sequenceNumber      == -1);
    assert(b.getRequestLog()[0].client_id           == "B");
    assert(b.getRequestLog()[0].view                == 0);
    assert(b.getRequestLog()[0].type                == REQUEST);
    assert(b.getRequestLog()[0].round               == 3);
    assert(b.getRequestLog()[0].phase               == IDEAL);
    assert(b.getRequestLog()[0].result              == 0);

    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round Four"<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;

    a.log();
    b.log();
    c.log();

    // checking logs
    
    assert(a.getRequestLog().size()                 == 2);
    assert(a.getPrePrepareLog().size()              == 0);
    assert(a.getPrepareLog().size()                 == 0);
    assert(a.getCommitLog().size()                  == 0);
    assert(a.getLedger().size()                     == 1);
    assert(a.getLedger()[0].sequenceNumber          == 1);
    assert(a.getLedger()[0].client_id               == "A");
    assert(a.getLedger()[0].sequenceNumber          == 1);
    assert(a.getLedger()[0].type                    == REPLY);
    assert(a.getLedger()[0].round                   == 4);
    assert(a.getLedger()[0].client_id               == "A");
    
    assert(b.getRequestLog().size()                 == 0);
    assert(b.getPrePrepareLog().size()              == 0);
    assert(b.getPrepareLog().size()                 == 0);
    assert(b.getCommitLog().size()                  == 0);
    assert(b.getLedger().size()                     == 1);
    assert(b.getLedger()[0].sequenceNumber          == 1);
    assert(b.getLedger()[0].client_id               == "A");
    assert(b.getLedger()[0].sequenceNumber          == 1);
    assert(b.getLedger()[0].type                    == REPLY);
    assert(b.getLedger()[0].round                   == 4);
    assert(b.getLedger()[0].client_id               == "A");
    
    assert(c.getRequestLog().size()                 == 0);
    assert(c.getPrePrepareLog().size()              == 0);
    assert(c.getPrepareLog().size()                 == 0);
    assert(c.getCommitLog().size()                  == 0);
    assert(c.getLedger().size()                     == 1);
    assert(c.getLedger()[0].sequenceNumber          == 1);
    assert(c.getLedger()[0].client_id               == "A");
    assert(c.getLedger()[0].sequenceNumber          == 1);
    assert(c.getLedger()[0].type                    == REPLY);
    assert(c.getLedger()[0].round                   == 4);
    assert(c.getLedger()[0].client_id               == "A");
    
    assert(a.getPhase()                             == PREPARE_WAIT);
    assert(b.getPhase()                             == IDEAL);
    assert(c.getPhase()                             == IDEAL);
    
    /////////////////////////////////////////////////////////////////////
    // make sure all request are committed

    int totalRequestsMade = 5; // prev made 5 request
    int requester = 'a';
    for(int i = 0; i < 1024; i++){
        switch (requester){
            case 'a':
                a.makeRequest();
                requester++;
                totalRequestsMade++;
                break;
            case 'b':
                b.makeRequest();
                requester++;
                totalRequestsMade++;
                break;
            case 'c':
                c.makeRequest();
                requester = 'a';
                totalRequestsMade++;
                break;
        }

        c.receive();
        b.receive();
        c.receive();
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round "<< std::to_string(i)<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;

        a.log();
        b.log();
        c.log();
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    assert(totalRequestsMade                        == 1029);
    assert(a.getLedger().size()                     == 257); // 256 from loop and one from prev tests
    assert(a.getRequestLog().size()                 == 1029 - 257);
    assert(b.getLedger().size()                     == 257); // 256 from loop and one from prev tests
    assert(c.getLedger().size()                     == 257); // 256 from loop and one from prev tests
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"multiRequest Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;

}
void slowLeaderConnection(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"slowLeaderConnection "<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;

    a = PBFT_Peer("A");
    b = PBFT_Peer("B");
    c = PBFT_Peer("C");
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    
    a.addNeighbor(b, 16);
    a.addNeighbor(c, 16);
    
    b.addNeighbor(a, 16);
    b.addNeighbor(c, 1);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 16);
    
    // set fault tolerance higher so that needs confirmation from 2 peers (myself + another)
    a.setFaultTolerance(0.4);
    b.setFaultTolerance(0.4);
    c.setFaultTolerance(0.4);
    
    a.init();
    b.init();
    c.init();

    int totalRequestsMade = 0;
    int requester = 'a';
    for(int i = 0; i < 1024; i++){
        switch (requester){
            case 'a':
                a.makeRequest();
                requester++;
                totalRequestsMade++;
                break;
            case 'b':
                b.makeRequest();
                requester++;
                totalRequestsMade++;
                break;
            case 'c':
                c.makeRequest();
                requester = 'a';
                totalRequestsMade++;
                break;
        }

        c.receive();
        b.receive();
        c.receive();
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round "<< std::to_string(i)<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;

        a.log();
        b.log();
        c.log();
        a.transmit();
        b.transmit();
        c.transmit();
    }
    assert(a.getLedger.size() == b.getLedger.size() == c.getLedger.size() == 64); // becouse request are slow coming in from leader whole system is slow
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"slowLeaderConnection Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void slowPeerConnection(std::ostream &log){
log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"slowLeaderConnection "<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;

    a = PBFT_Peer("A");
    b = PBFT_Peer("B");
    c = PBFT_Peer("C");
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    
    a.addNeighbor(b, 1);
    a.addNeighbor(c, 16);
    
    b.addNeighbor(a, 1);
    b.addNeighbor(c, 16);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    
    // set fault tolerance higher so that needs confirmation from 2 peers (myself + another)
    a.setFaultTolerance(0.4);
    b.setFaultTolerance(0.4);
    c.setFaultTolerance(0.4);
    
    a.init();
    b.init();
    c.init();

    int totalRequestsMade = 0;
    int requester = 'a';
    for(int i = 0; i < 1024; i++){
        switch (requester){
            case 'a':
                a.makeRequest();
                requester++;
                totalRequestsMade++;
                break;
            case 'b':
                b.makeRequest();
                requester++;
                totalRequestsMade++;
                break;
            case 'c':
                c.makeRequest();
                requester = 'a';
                totalRequestsMade++;
                break;
        }

        c.receive();
        b.receive();
        c.receive();
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!"<<"Round "<< std::to_string(i)<< std::setw(LOG_WIDTH)<< std::right<<"!"<<"###############################"<< std::endl;

        a.log();
        b.log();
        c.log();
        a.transmit();
        b.transmit();
        c.transmit();
    }
    assert(a.getLedger.size() == b.getLedger.size() == 1024); // these too can confirm with out C so all transactions are committed
    assert(c.getLedger.size() == 64); // c has slow connection so it's leger is behind
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"slowLeaderConnection Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
