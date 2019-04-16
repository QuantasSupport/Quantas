//
//  PeerTest.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 4/15/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "PBFTPeerTest.hpp"

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

void constructors(){
    PBFT_Peer a = PBFT_Peer("A");

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

    PBFT_Peer b = PBFT_Peer(a);
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
}
void testSettersMutators(){
    //////////////////////////////////////////////////////////////
    // Neighbor tests
    //////////////////////////////////////////////////////////////
    
    a = PBFT_Peer("A");
    b = PBFT_Peer("B");
    c = PBFT_Peer("C");
    
    a.addNeighbor(b, 1);
    
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
    //////////////////////////////////////////////////////////////
    
    a = PBFT_Peer("A");
    
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
    
    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    a.addNeighbor(d, 1);
    a.addNeighbor(e, 1);
    a.addNeighbor(f, 1);
    a.addNeighbor(g, 1);
    a.addNeighbor(h, 1);
    a.addNeighbor(i, 1);
    a.addNeighbor(j, 1);
    
    a.setFaultTolerance(0.3);
    assert(a.faultyPeers()              == 3);
    a.setFaultTolerance(0.0);
    assert(a.faultyPeers()              == 0);
    
    //////////////////////////////////////////////////////////////
    // primary tests
    //////////////////////////////////////////////////////////////
    
    c = PBFT_Peer("C");
    b = PBFT_Peer("B");
    c.addNeighbor(b, 1);
    b.addNeighbor(c, 1);
    
    c.init();
    b.init();

    assert(c.isPrimary()                == false);
    assert(c.getPrimary()               == "B");

    assert(b.isPrimary()                == true);
    assert(b.getPrimary()               == "B");
    
    c.clearPrimary();
    b.clearPrimary();
    
    assert(a.isPrimary()                == false);
    assert(a.getPrimary()               == NO_PRIMARY);

    assert(b.isPrimary()                == false);
    assert(b.getPrimary()               == NO_PRIMARY);

    
    c.initPrimary();
    b.initPrimary();

    assert(c.isPrimary()                == false);
    assert(c.getPrimary()               == "B");

    assert(b.isPrimary()                == true);
    assert(b.getPrimary()               == "B");

    
    a = PBFT_Peer("A");
    c.addNeighbor(a, 1);
    b.addNeighbor(a, 1);
    c.clearPrimary();
    b.clearPrimary();
    a.initPrimary();
    b.initPrimary();
    c.initPrimary();

    assert(a.isPrimary()                == true);
    assert(a.getPrimary()               == "A");

    assert(b.isPrimary()                == false);
    assert(b.getPrimary()               == "A");

    assert(c.isPrimary()                == false);
    assert(c.getPrimary()               == "A");
}

void oneRequest(){
    a = PBFT_Peer("A");
    b = PBFT_Peer("B");
    c = PBFT_Peer("C");
    
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
    /////////////////////////////////////////////////////////////////////
    
    a.makeRequest();
    
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
    /////////////////////////////////////////////////////////////////////
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    
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
    /////////////////////////////////////////////////////////////////////
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    
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
    /////////////////////////////////////////////////////////////////////
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    
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
    
    /////////////////////////////////////////////////////////////////////
    //      round four commited and logs cleaned leger increased
    /////////////////////////////////////////////////////////////////////
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    
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
    /////////////////////////////////////////////////////////////////////
    
    for(int i = 0; i < 10000; i++){
        a.receive();
        b.receive();
        c.receive();
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
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
    
    
    a = PBFT_Peer("A");
    b = PBFT_Peer("B");
    c = PBFT_Peer("C");
    
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
    /////////////////////////////////////////////////////////////////////
    
    b.makeRequest();
    
    assert(b.getRequestLog().size()                 == 1);
    assert(b.getRequestLog()[0].sequenceNumber      == -1);
    assert(b.getRequestLog()[0].client_id           == "B");
    assert(b.getRequestLog()[0].view                == 0);
    assert(b.getRequestLog()[0].type                == REQUEST);
    assert(b.getRequestLog()[0].round               == 0);
    assert(b.getRequestLog()[0].phase               == IDEAL);
    assert(b.getRequestLog()[0].result              == 0);
    
    /////////////////////////////////////////////////////////////////////
    //      round one Pre-Prepare
    /////////////////////////////////////////////////////////////////////
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    
    assert(a.getPhase()                             == IDEAL);
    assert(b.getPhase()                             == IDEAL);
    assert(c.getPhase()                             == IDEAL);
    
    a.transmit();
    b.transmit();
    c.transmit();
    
    // leader should get the request now
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    
    assert(a.getPhase()                             == IDEAL);
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
    /////////////////////////////////////////////////////////////////////
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    
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
    /////////////////////////////////////////////////////////////////////
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    
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
    
    /////////////////////////////////////////////////////////////////////
    //      round four commited and logs cleaned leger increased
    /////////////////////////////////////////////////////////////////////
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    
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
    /////////////////////////////////////////////////////////////////////
    
    for(int i = 0; i < 10000; i++){
        a.receive();
        b.receive();
        c.receive();
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
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
    
}
void multiRequest(){
    /////////////////////////////////////////////////////////////////////
    //      make the request to leader
    /////////////////////////////////////////////////////////////////////
    
    a.makeRequest();
    
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
    /////////////////////////////////////////////////////////////////////
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    
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
    /////////////////////////////////////////////////////////////////////
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    
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
    /////////////////////////////////////////////////////////////////////
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    
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
    
    /////////////////////////////////////////////////////////////////////
    //      round four commited and logs cleaned leger increased
    /////////////////////////////////////////////////////////////////////
    
    a.receive();
    b.receive();
    c.receive();
    a.preformComputation();
    b.preformComputation();
    c.preformComputation();
    
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
    /////////////////////////////////////////////////////////////////////
    
    for(int i = 0; i < 10000; i++){
        a.receive();
        b.receive();
        c.receive();
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
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
}
void testWaitPrepare(){
    
}
void testCommit(){
    
}
void testWaitCommit(){
    
}

void testComputation(){
    
}

void testBraodcast(){
    
}
void testCleanLogs(){
    
}
void testInitPrimary(){
    
}
void testFaultyPeers(){
    
}
