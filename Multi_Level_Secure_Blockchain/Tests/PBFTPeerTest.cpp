//
//  PeerTest.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 4/15/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "PBFTPeerTest.hpp"

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
    PBFT_Peer a = PBFT_Peer("A");
    PBFT_Peer b = PBFT_Peer("B");
    PBFT_Peer c = PBFT_Peer("C");
    PBFT_Peer d = PBFT_Peer("D");
    PBFT_Peer e = PBFT_Peer("E");
    PBFT_Peer f = PBFT_Peer("F");
    PBFT_Peer g = PBFT_Peer("G");
    PBFT_Peer h = PBFT_Peer("H");
    PBFT_Peer i = PBFT_Peer("I");
    PBFT_Peer j = PBFT_Peer("J");
    
    //////////////////////////////////////////////////////////////
    // Neighbor tests
    
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
void testGetters(){
    
}

void testPrePrepare(){
    
}
void testPrepare(){
    
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
