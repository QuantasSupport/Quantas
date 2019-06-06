//
//  PBFTPeer_Sharded_Test.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 5/7/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <algorithm>
#include "PBFTPeer_Sharded_Test.hpp"

void RunPBFTPeerShardedTest(std::string filepath){
    std::ofstream log;
    log.open(filepath + "/PBFT_Sharded.log");
    if (log.fail() ){
        std::cerr << "Error: could not open file at: "<< filepath << std::endl;
    }
    constructorsPBFT_s(log);
    testSettersMutatorsPBFT_s(log);
    testGroups(log);
    testCommittee(log);
    oneRequestOneCommittee(log);
    oneRequestMultiCommittee(log);
    ///////////////
    // MultiRequestMultiCommittee();
    testViewChange(log);
    testByzantine(log);
    waitingTimeSharded(log);
}
void constructorsPBFT_s(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"constructors"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;

    PBFTPeer_Sharded a = PBFTPeer_Sharded("A");
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
    
    a = PBFTPeer_Sharded("A");
    a.setFaultTolerance(0.3);
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

    PBFTPeer_Sharded b = PBFTPeer_Sharded(a);
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
void testSettersMutatorsPBFT_s(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testSettersMutators"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;

    //////////////////////////////////////////////////////////////
    // Neighbor tests
    
    
    PBFTPeer_Sharded a = PBFTPeer_Sharded("A");
    PBFTPeer_Sharded b = PBFTPeer_Sharded("B");
    PBFTPeer_Sharded c = PBFTPeer_Sharded("C");
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
    assert(a.neighbors()[0]             == "B");
    assert(b.id()                       == "B");
    assert(b.isNeighbor("A")            == true);
    assert(b.isNeighbor("B")            == false);
    assert(b.isNeighbor("Z")            == false);
    assert(b.isPrimary()                == false);
    assert(b.getPrimary()               == NO_PRIMARY);
    assert(b.getDelayToNeighbor("A")    == 100);
    
    b.addNeighbor(a, 5);
    a.log();
    b.log();
    c.log();

    // state
    assert(a.neighbors().size()         == 1);
    assert(a.neighbors()[0]             == "B");
    assert(b.id()                       == "B");
    assert(b.isNeighbor("A")            == true);
    assert(b.isNeighbor("B")            == false);
    assert(b.isNeighbor("Z")            == false);
    assert(b.isPrimary()                == false);
    assert(b.getPrimary()               == NO_PRIMARY);
    assert(b.getDelayToNeighbor("A")    == 5);
    
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

    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    
    a.setFaultTolerance(0.3);
    b.setFaultTolerance(0.3);
    c.setFaultTolerance(0.3);
    
    assert(a.faultyPeers() == 1);
    assert(b.faultyPeers() == 1);
    assert(c.faultyPeers() == 1);
    
    PBFTPeer_Sharded d = PBFTPeer_Sharded("D");
    PBFTPeer_Sharded e = PBFTPeer_Sharded("E");
    PBFTPeer_Sharded f = PBFTPeer_Sharded("F");
    d.setFaultTolerance(0.3);
    e.setFaultTolerance(0.3);
    f.setFaultTolerance(0.3);
    
    d.setLogFile(log);
    e.setLogFile(log);
    f.setLogFile(log);
    
    d.addNeighbor(b, 1);
    d.addNeighbor(c, 1);
    d.addNeighbor(a, 1);
    d.addNeighbor(e, 1);
    d.addNeighbor(f, 1);
    
    e.addNeighbor(b, 1);
    e.addNeighbor(c, 1);
    e.addNeighbor(a, 1);
    e.addNeighbor(d, 1);
    e.addNeighbor(f, 1);
    
    f.addNeighbor(b, 1);
    f.addNeighbor(c, 1);
    f.addNeighbor(a, 1);
    f.addNeighbor(e, 1);
    f.addNeighbor(d, 1);
    
    d.setCommittee(1);
    e.setCommittee(1);
    f.setCommittee(1);
    
    a.addCommitteeMember(d);
    a.addCommitteeMember(e);
    a.addCommitteeMember(f);
    
    b.addCommitteeMember(d);
    b.addCommitteeMember(e);
    b.addCommitteeMember(f);
    
    c.addCommitteeMember(d);
    c.addCommitteeMember(e);
    c.addCommitteeMember(f);
    
    d.addCommitteeMember(a);
    d.addCommitteeMember(b);
    d.addCommitteeMember(c);
    d.addCommitteeMember(e);
    d.addCommitteeMember(f);
    
    e.addCommitteeMember(a);
    e.addCommitteeMember(b);
    e.addCommitteeMember(c);
    e.addCommitteeMember(d);
    e.addCommitteeMember(f);
    
    f.addCommitteeMember(a);
    f.addCommitteeMember(b);
    f.addCommitteeMember(c);
    f.addCommitteeMember(d);
    f.addCommitteeMember(e);
    
    assert(a.faultyPeers() == 2);
    assert(b.faultyPeers() == 2);
    assert(c.faultyPeers() == 2);
    assert(d.faultyPeers() == 2);
    assert(e.faultyPeers() == 2);
    assert(f.faultyPeers() == 2);
    
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testSettersMutators Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void testGroups(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"Test Groups"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;

    PBFTPeer_Sharded a = PBFTPeer_Sharded("A");
    PBFTPeer_Sharded b = PBFTPeer_Sharded("B");
    PBFTPeer_Sharded c = PBFTPeer_Sharded("C");
    PBFTPeer_Sharded d = PBFTPeer_Sharded("D");

    a.setLogFile(log); 
    b.setLogFile(log);
    c.setLogFile(log);
    d.setLogFile(log);

    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    a.addNeighbor(d, 1);

    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    b.addNeighbor(d, 1);

    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    c.addNeighbor(d, 1);

    d.addNeighbor(b, 1);
    d.addNeighbor(c, 1);
    d.addNeighbor(a, 1);

    a.log();
    b.log();
    c.log();
    d.log();

    //////////////////////////////////////////////////////////////
    // test group assignment

    a.setGroup(1);
    b.setGroup(1);
    c.setGroup(1);
    d.setGroup(1);

    a.addGroupMember(b);
    a.addGroupMember(c);
    a.addGroupMember(d);

    b.addGroupMember(a);
    b.addGroupMember(c);
    b.addGroupMember(d);

    c.addGroupMember(b);
    c.addGroupMember(a);
    c.addGroupMember(d);

    d.addGroupMember(b);
    d.addGroupMember(c);
    d.addGroupMember(a);

    assert(a.getGroup()                == 1);
    assert(b.getGroup()                == 1);
    assert(c.getGroup()                == 1);
    assert(d.getGroup()                == 1);

    assert(a.getGroupMembers().size()  == 3);
    assert(b.getGroupMembers().size()  == 3);
    assert(c.getGroupMembers().size()  == 3);
    assert(d.getGroupMembers().size()  == 3);

    assert(std::find(a.getGroupMembers().begin(), a.getGroupMembers().end(), "B") != a.getGroupMembers().end());
    assert(std::find(a.getGroupMembers().begin(), a.getGroupMembers().end(), "C") != a.getGroupMembers().end());
    assert(std::find(a.getGroupMembers().begin(), a.getGroupMembers().end(), "D") != a.getGroupMembers().end());

    assert(std::find(b.getGroupMembers().begin(), b.getGroupMembers().end(), "A") != a.getGroupMembers().end());
    assert(std::find(b.getGroupMembers().begin(), b.getGroupMembers().end(), "C") != a.getGroupMembers().end());
    assert(std::find(b.getGroupMembers().begin(), b.getGroupMembers().end(), "D") != a.getGroupMembers().end());

    assert(std::find(c.getGroupMembers().begin(), c.getGroupMembers().end(), "B") != a.getGroupMembers().end());
    assert(std::find(c.getGroupMembers().begin(), c.getGroupMembers().end(), "A") != a.getGroupMembers().end());
    assert(std::find(c.getGroupMembers().begin(), c.getGroupMembers().end(), "D") != a.getGroupMembers().end());

    assert(std::find(d.getGroupMembers().begin(), d.getGroupMembers().end(), "B") != a.getGroupMembers().end());
    assert(std::find(d.getGroupMembers().begin(), d.getGroupMembers().end(), "C") != a.getGroupMembers().end());
    assert(std::find(d.getGroupMembers().begin(), d.getGroupMembers().end(), "A") != a.getGroupMembers().end());

    //////////////////////////////////////////////////////////////
    // test group re-assigment 

    d.setGroup(2);

    a.setGroup(1);
    b.setGroup(1);
    c.setGroup(1);

    a.addGroupMember(b);
    a.addGroupMember(c);

    b.addGroupMember(a);
    b.addGroupMember(c);

    c.addGroupMember(b);
    c.addGroupMember(a);

    assert(a.getGroup()                == 1);
    assert(b.getGroup()                == 1);
    assert(c.getGroup()                == 1);
    assert(d.getGroup()                == 2);

    assert(a.getGroupMembers().size()  == 2);
    assert(b.getGroupMembers().size()  == 2);
    assert(c.getGroupMembers().size()  == 2);
    assert(d.getGroupMembers().size()  == 0);

    assert(std::find(a.getGroupMembers().begin(), a.getGroupMembers().end(), "B") != a.getGroupMembers().end());
    assert(std::find(a.getGroupMembers().begin(), a.getGroupMembers().end(), "C") != a.getGroupMembers().end());

    assert(std::find(b.getGroupMembers().begin(), b.getGroupMembers().end(), "A") != a.getGroupMembers().end());
    assert(std::find(b.getGroupMembers().begin(), b.getGroupMembers().end(), "C") != a.getGroupMembers().end());

    assert(std::find(c.getGroupMembers().begin(), c.getGroupMembers().end(), "B") != a.getGroupMembers().end());
    assert(std::find(c.getGroupMembers().begin(), c.getGroupMembers().end(), "A") != a.getGroupMembers().end());

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"Test Groups Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void testCommittee(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"Test Committee"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;

    PBFTPeer_Sharded a = PBFTPeer_Sharded("A");
    PBFTPeer_Sharded b = PBFTPeer_Sharded("B");
    PBFTPeer_Sharded c = PBFTPeer_Sharded("C");
    PBFTPeer_Sharded d = PBFTPeer_Sharded("D");

    a.setLogFile(log); 
    b.setLogFile(log);
    c.setLogFile(log);
    d.setLogFile(log);

    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    a.addNeighbor(d, 1);

    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    b.addNeighbor(d, 1);

    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    c.addNeighbor(d, 1);

    d.addNeighbor(b, 1);
    d.addNeighbor(c, 1);
    d.addNeighbor(a, 1);

    a.log();
    b.log();
    c.log();
    d.log();

    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    d.setCommittee(1);

    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    a.addCommitteeMember(d);

    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    b.addCommitteeMember(d);

    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    c.addCommitteeMember(d);

    d.addCommitteeMember(b);
    d.addCommitteeMember(c);
    d.addCommitteeMember(a);

    assert(a.getCommittee()                == 1);
    assert(b.getCommittee()                == 1);
    assert(c.getCommittee()                == 1);
    assert(d.getCommittee()                == 1);

    assert(a.getCommitteeMembers().size()  == 3);
    assert(b.getCommitteeMembers().size()  == 3);
    assert(c.getCommitteeMembers().size()  == 3);
    assert(d.getCommitteeMembers().size()  == 3);

    assert(std::find(a.getCommitteeMembers().begin(), a.getCommitteeMembers().end(), "B") != a.getCommitteeMembers().end());
    assert(std::find(a.getCommitteeMembers().begin(), a.getCommitteeMembers().end(), "C") != a.getCommitteeMembers().end());
    assert(std::find(a.getCommitteeMembers().begin(), a.getCommitteeMembers().end(), "D") != a.getCommitteeMembers().end());

    assert(std::find(b.getCommitteeMembers().begin(), b.getCommitteeMembers().end(), "A") != a.getCommitteeMembers().end());
    assert(std::find(b.getCommitteeMembers().begin(), b.getCommitteeMembers().end(), "C") != a.getCommitteeMembers().end());
    assert(std::find(b.getCommitteeMembers().begin(), b.getCommitteeMembers().end(), "D") != a.getCommitteeMembers().end());

    assert(std::find(c.getCommitteeMembers().begin(), c.getCommitteeMembers().end(), "B") != a.getCommitteeMembers().end());
    assert(std::find(c.getCommitteeMembers().begin(), c.getCommitteeMembers().end(), "A") != a.getCommitteeMembers().end());
    assert(std::find(c.getCommitteeMembers().begin(), c.getCommitteeMembers().end(), "D") != a.getCommitteeMembers().end());

    assert(std::find(d.getCommitteeMembers().begin(), d.getCommitteeMembers().end(), "B") != a.getCommitteeMembers().end());
    assert(std::find(d.getCommitteeMembers().begin(), d.getCommitteeMembers().end(), "C") != a.getCommitteeMembers().end());
    assert(std::find(d.getCommitteeMembers().begin(), d.getCommitteeMembers().end(), "A") != a.getCommitteeMembers().end());

    //////////////////////////////////////////////////////////////
    // test committee re-assigment 

    d.setCommittee(2);

    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);

    a.addCommitteeMember(b);
    a.addCommitteeMember(c);

    b.addCommitteeMember(a);
    b.addCommitteeMember(c);

    c.addCommitteeMember(b);
    c.addCommitteeMember(a);

    assert(a.getCommittee()                == 1);
    assert(b.getCommittee()                == 1);
    assert(c.getCommittee()                == 1);
    assert(d.getCommittee()                == 2);

    assert(a.getCommitteeMembers().size()  == 2);
    assert(b.getCommitteeMembers().size()  == 2);
    assert(c.getCommitteeMembers().size()  == 2);
    assert(d.getCommitteeMembers().size()  == 0);

    assert(std::find(a.getCommitteeMembers().begin(), a.getCommitteeMembers().end(), "B") != a.getCommitteeMembers().end());
    assert(std::find(a.getCommitteeMembers().begin(), a.getCommitteeMembers().end(), "C") != a.getCommitteeMembers().end());

    assert(std::find(b.getCommitteeMembers().begin(), b.getCommitteeMembers().end(), "A") != a.getCommitteeMembers().end());
    assert(std::find(b.getCommitteeMembers().begin(), b.getCommitteeMembers().end(), "C") != a.getCommitteeMembers().end());

    assert(std::find(c.getCommitteeMembers().begin(), c.getCommitteeMembers().end(), "B") != a.getCommitteeMembers().end());
    assert(std::find(c.getCommitteeMembers().begin(), c.getCommitteeMembers().end(), "A") != a.getCommitteeMembers().end());

    //////////////////////////////////////////////////////////////
    // test leader selction

    a = PBFTPeer_Sharded("A");
    b = PBFTPeer_Sharded("B");
    c = PBFTPeer_Sharded("C");
    d = PBFTPeer_Sharded("D");

    a.setLogFile(log); 
    b.setLogFile(log);
    c.setLogFile(log);
    d.setLogFile(log);

    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    a.addNeighbor(d, 1);

    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    b.addNeighbor(d, 1);

    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    c.addNeighbor(d, 1);

    d.addNeighbor(b, 1);
    d.addNeighbor(c, 1);
    d.addNeighbor(a, 1);

    a.log();
    b.log();
    c.log();
    d.log();

    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    d.setCommittee(1);

    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    a.addCommitteeMember(d);

    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    b.addCommitteeMember(d);

    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    c.addCommitteeMember(d);

    d.addCommitteeMember(b);
    d.addCommitteeMember(c);
    d.addCommitteeMember(a);

    a.init();
    b.init();
    c.init();
    d.init();

    assert(a.getPrimary() == "A");
    assert(b.getPrimary() == "A");
    assert(c.getPrimary() == "A");
    assert(d.getPrimary() == "A");

    a = PBFTPeer_Sharded("A");
    b = PBFTPeer_Sharded("B");
    c = PBFTPeer_Sharded("C");
    d = PBFTPeer_Sharded("D");

    a.setLogFile(log); 
    b.setLogFile(log);
    c.setLogFile(log);
    d.setLogFile(log);

    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    a.addNeighbor(d, 1);

    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    b.addNeighbor(d, 1);

    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    c.addNeighbor(d, 1);

    d.addNeighbor(b, 1);
    d.addNeighbor(c, 1);
    d.addNeighbor(a, 1);

    a.log();
    b.log();
    c.log();
    d.log();

    a.setCommittee(2);
    b.setCommittee(1);
    c.setCommittee(1);
    d.setCommittee(1);

    b.addCommitteeMember(c);
    b.addCommitteeMember(d);

    c.addCommitteeMember(b);
    c.addCommitteeMember(d);

    d.addCommitteeMember(b);
    d.addCommitteeMember(c);

    a.init();
    b.init();
    c.init();
    d.init();

    assert(a.getPrimary() == "A");
    assert(b.getPrimary() == "B");
    assert(c.getPrimary() == "B");
    assert(d.getPrimary() == "B");

    //////////////////////////////////////////////////////////////
    // test fault tolerace
    
    a = PBFTPeer_Sharded("A");
    b = PBFTPeer_Sharded("B");
    c = PBFTPeer_Sharded("C");
    d = PBFTPeer_Sharded("D");

    a.setLogFile(log); 
    b.setLogFile(log);
    c.setLogFile(log);
    d.setLogFile(log);

    a.setFaultTolerance(1); 
    b.setFaultTolerance(1);
    c.setFaultTolerance(1);
    d.setFaultTolerance(1);

    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    a.addNeighbor(d, 1);

    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    b.addNeighbor(d, 1);

    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    c.addNeighbor(d, 1);

    d.addNeighbor(b, 1);
    d.addNeighbor(c, 1);
    d.addNeighbor(a, 1);

    a.log();
    b.log();
    c.log();
    d.log();

    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    d.setCommittee(1);

    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    a.addCommitteeMember(d);

    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    b.addCommitteeMember(d);

    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    c.addCommitteeMember(d);

    d.addCommitteeMember(b);
    d.addCommitteeMember(c);
    d.addCommitteeMember(a);

    a.init();
    b.init();
    c.init();
    d.init();

    assert(a.faultyPeers() == 4);
    assert(b.faultyPeers() == 4);
    assert(c.faultyPeers() == 4);
    assert(d.faultyPeers() == 4);

    a = PBFTPeer_Sharded("A");
    b = PBFTPeer_Sharded("B");
    c = PBFTPeer_Sharded("C");
    d = PBFTPeer_Sharded("D");

    a.setLogFile(log); 
    b.setLogFile(log);
    c.setLogFile(log);
    d.setLogFile(log);

    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    a.addNeighbor(d, 1);

    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    b.addNeighbor(d, 1);

    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    c.addNeighbor(d, 1);

    d.addNeighbor(b, 1);
    d.addNeighbor(c, 1);
    d.addNeighbor(a, 1);

    a.log();
    b.log();
    c.log();
    d.log();

    a.setFaultTolerance(1); 
    b.setFaultTolerance(1);
    c.setFaultTolerance(1);
    d.setFaultTolerance(1);

    a.setCommittee(2);
    b.setCommittee(1);
    c.setCommittee(1);
    d.setCommittee(1);

    b.addCommitteeMember(c);
    b.addCommitteeMember(d);

    c.addCommitteeMember(b);
    c.addCommitteeMember(d);

    d.addCommitteeMember(b);
    d.addCommitteeMember(c);

    a.init();
    b.init();
    c.init();
    d.init();

    assert(a.faultyPeers() == 1);
    assert(b.faultyPeers() == 3);
    assert(c.faultyPeers() == 3);
    assert(d.faultyPeers() == 3);

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"Test Committee Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void oneRequestOneCommittee(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"One Request One Committee"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    PBFTPeer_Sharded a = PBFTPeer_Sharded("A");
    PBFTPeer_Sharded b = PBFTPeer_Sharded("B");
    PBFTPeer_Sharded c = PBFTPeer_Sharded("C");

    a.setFaultTolerance(1);
    b.setFaultTolerance(1);
    c.setFaultTolerance(1);

    a.setLogFile(log); 
    b.setLogFile(log);
    c.setLogFile(log);

    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);

    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);

    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);

    a.log();
    b.log();
    c.log();

    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);

    a.addCommitteeMember(b);
    a.addCommitteeMember(c);

    b.addCommitteeMember(a);
    b.addCommitteeMember(c);

    c.addCommitteeMember(b);
    c.addCommitteeMember(a);

    a.init();
    b.init();
    c.init();

    a.makeRequest();
    for(int i = 0; i < 4; i++){
        a.receive();
        b.receive();
        c.receive();

        a.preformComputation();
        b.preformComputation();
        c.preformComputation();

        a.transmit();
        b.transmit();
        c.transmit();
    }

    assert(a.getRequestLog().size()                 == 0);
    assert(a.getPrePrepareLog().size()              == 0);
    assert(a.getPrepareLog().size()                 == 0);
    assert(a.getCommitLog().size()                  == 0);
    assert(a.getLedger().size()                     == 1);
    assert(a.getLedger()[0].sequenceNumber          == 1);
    assert(a.getLedger()[0].type                    == REPLY);
    assert(a.getLedger()[0].commit_round            == 4);
    assert(a.getLedger()[0].client_id               == "A");
    
    assert(b.getRequestLog().size()                 == 0);
    assert(b.getPrePrepareLog().size()              == 0);
    assert(b.getPrepareLog().size()                 == 0);
    assert(b.getCommitLog().size()                  == 0);
    assert(b.getLedger().size()                     == 1);
    assert(b.getLedger()[0].sequenceNumber          == 1);
    assert(b.getLedger()[0].type                    == REPLY);
    assert(b.getLedger()[0].commit_round            == 4);
    assert(b.getLedger()[0].client_id               == "A");
    
    assert(c.getRequestLog().size()                 == 0);
    assert(c.getPrePrepareLog().size()              == 0);
    assert(c.getPrepareLog().size()                 == 0);
    assert(c.getCommitLog().size()                  == 0);
    assert(c.getLedger().size()                     == 1);
    assert(c.getLedger()[0].sequenceNumber          == 1);
    assert(c.getLedger()[0].type                    == REPLY);
    assert(c.getLedger()[0].commit_round            == 4);
    assert(c.getLedger()[0].client_id               == "A");
    
    assert(a.getPhase()                             == IDEAL);
    assert(b.getPhase()                             == IDEAL);
    assert(c.getPhase()                             == IDEAL);

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"One Request One Committee Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void oneRequestMultiCommittee(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"One Request Multi Committee"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    
    PBFTPeer_Sharded a = PBFTPeer_Sharded("A");
    PBFTPeer_Sharded b = PBFTPeer_Sharded("B");
    PBFTPeer_Sharded c = PBFTPeer_Sharded("C");
    PBFTPeer_Sharded d = PBFTPeer_Sharded("D");
    PBFTPeer_Sharded e = PBFTPeer_Sharded("E");
    PBFTPeer_Sharded f = PBFTPeer_Sharded("F");

    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    a.addNeighbor(d, 1);
    a.addNeighbor(e, 1);
    a.addNeighbor(f, 1);

    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    b.addNeighbor(d, 1);
    b.addNeighbor(e, 1);
    b.addNeighbor(f, 1);

    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    c.addNeighbor(d, 1);
    c.addNeighbor(e, 1);
    c.addNeighbor(f, 1);

    d.addNeighbor(b, 1);
    d.addNeighbor(c, 1);
    d.addNeighbor(a, 1);
    d.addNeighbor(e, 1);
    d.addNeighbor(f, 1);

    e.addNeighbor(b, 1);
    e.addNeighbor(c, 1);
    e.addNeighbor(d, 1);
    e.addNeighbor(a, 1);
    e.addNeighbor(f, 1);

    f.addNeighbor(b, 1);
    f.addNeighbor(c, 1);
    f.addNeighbor(d, 1);
    f.addNeighbor(e, 1);
    f.addNeighbor(a, 1);

    // make first committee

    a.setFaultTolerance(1);
    b.setFaultTolerance(1);
    c.setFaultTolerance(1);

    a.setLogFile(log); 
    b.setLogFile(log);
    c.setLogFile(log);

    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);

    a.addCommitteeMember(b);
    a.addCommitteeMember(c);

    b.addCommitteeMember(a);
    b.addCommitteeMember(c);

    c.addCommitteeMember(b);
    c.addCommitteeMember(a);

    a.init();
    b.init();
    c.init();

    a.log();
    b.log();
    c.log();

    // make second committee

    d.setFaultTolerance(1);
    e.setFaultTolerance(1);
    f.setFaultTolerance(1);

    d.setLogFile(log); 
    e.setLogFile(log);
    f.setLogFile(log);

    d.setCommittee(2);
    e.setCommittee(2);
    f.setCommittee(2);

    d.addCommitteeMember(e);
    d.addCommitteeMember(f);

    e.addCommitteeMember(d);
    e.addCommitteeMember(f);

    f.addCommitteeMember(d);
    f.addCommitteeMember(e);

    d.init();
    e.init();
    f.init();

    d.log();
    e.log();
    f.log();

    // two committees 1 = {a,b,c} and 2 = {d,e,f}
    
    a.makeRequest();
    for(int i = 0; i < 5; i++){
        a.receive();
        b.receive();
        c.receive();
        d.receive();
        e.receive();
        f.receive();

        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        d.preformComputation();
        e.preformComputation();
        f.preformComputation();

        a.transmit();
        b.transmit();
        c.transmit();
        d.transmit();
        e.transmit();
        f.transmit();
    }

    assert(a.getRequestLog().size()                 == 0);
    assert(a.getPrePrepareLog().size()              == 0);
    assert(a.getPrepareLog().size()                 == 0);
    assert(a.getCommitLog().size()                  == 0);
    assert(a.getLedger().size()                     == 1);
    assert(a.getLedger()[0].sequenceNumber          == 1);
    assert(a.getLedger()[0].type                    == REPLY);
    assert(a.getLedger()[0].commit_round            == 4);
    assert(a.getLedger()[0].client_id               == "A");
    
    assert(b.getRequestLog().size()                 == 0);
    assert(b.getPrePrepareLog().size()              == 0);
    assert(b.getPrepareLog().size()                 == 0);
    assert(b.getCommitLog().size()                  == 0);
    assert(b.getLedger().size()                     == 1);
    assert(b.getLedger()[0].sequenceNumber          == 1);
    assert(b.getLedger()[0].type                    == REPLY);
    assert(b.getLedger()[0].commit_round            == 4);
    assert(b.getLedger()[0].client_id               == "A");
    
    assert(c.getRequestLog().size()                 == 0);
    assert(c.getPrePrepareLog().size()              == 0);
    assert(c.getPrepareLog().size()                 == 0);
    assert(c.getCommitLog().size()                  == 0);
    assert(c.getLedger().size()                     == 1);
    assert(c.getLedger()[0].sequenceNumber          == 1);
    assert(c.getLedger()[0].type                    == REPLY);
    assert(c.getLedger()[0].commit_round            == 4);
    assert(c.getLedger()[0].client_id               == "A");
    
    assert(d.getRequestLog().size()                 == 0);
    assert(d.getPrePrepareLog().size()              == 0);
    assert(d.getPrepareLog().size()                 == 0);
    assert(d.getCommitLog().size()                  == 0);
    assert(d.getLedger().size()                     == 0);

    assert(e.getRequestLog().size()                 == 0);
    assert(e.getPrePrepareLog().size()              == 0);
    assert(e.getPrepareLog().size()                 == 0);
    assert(e.getCommitLog().size()                  == 0);
    assert(e.getLedger().size()                     == 0);

    assert(f.getRequestLog().size()                 == 0);
    assert(f.getPrePrepareLog().size()              == 0);
    assert(f.getPrepareLog().size()                 == 0);
    assert(f.getCommitLog().size()                  == 0);
    assert(f.getLedger().size()                     == 0);

    assert(a.getPhase()                             == IDEAL);
    assert(b.getPhase()                             == IDEAL);
    assert(c.getPhase()                             == IDEAL);
    assert(d.getPhase()                             == IDEAL);
    assert(e.getPhase()                             == IDEAL);
    assert(f.getPhase()                             == IDEAL);

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"One Request Multi Committee Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void MultiRequestMultiCommittee(std::ostream &log){
    log<< std::endl<< "+++++++++++++++++++++++++++++++"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"Multi Request Multi Committee incomplete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"+++++++++++++++++++++++++++++++"<< std::endl;
    
    //log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"Multi Request Multi Committee Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}

/////////////////////////////////////////
//
void testViewChange(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testViewChange "<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    PBFTPeer_Sharded a = PBFTPeer_Sharded("A");
    PBFTPeer_Sharded b = PBFTPeer_Sharded("B");
    PBFTPeer_Sharded c = PBFTPeer_Sharded("C");
    
    a.setFaultTolerance(0.6);
    b.setFaultTolerance(0.6);
    c.setFaultTolerance(0.6);
    
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    
    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    
    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    
    a.log();
    b.log();
    c.log();
    
    a.makeByzantine();
    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    
    a.init();
    b.init();
    c.init();
    
    ///////////////////////////////////////////
    // Byzantine leader
    
    a.makeRequest();
    
    for(int i = 0; i < 5; i++){
        a.log();
        b.log();
        c.log();
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    // view change should have happend here none of the peers should have left the committee
    assert(a.isPrimary()                == false);
    assert(b.isPrimary()                == true);
    assert(c.isPrimary()                == false);
    
    assert(a.getCommittee()             == 1);
    assert(b.getCommittee()             == 1);
    assert(c.getCommittee()             == 1);
    
    assert(a.getLedger().size()         == 0);
    assert(b.getLedger().size()         == 0);
    assert(c.getLedger().size()         == 0);
    
    assert(a.getCommitLog().size()      == 3);
    assert(b.getCommitLog().size()      == 3);
    assert(c.getCommitLog().size()      == 3);
    
    assert(a.getPrepareLog().size()     == 3);
    assert(b.getPrepareLog().size()     == 4);
    assert(c.getPrepareLog().size()     == 3);
    
    assert(a.getPrePrepareLog().size()  == 0);
    assert(b.getPrePrepareLog().size()  == 0);
    assert(c.getPrePrepareLog().size()  == 0);
    
    assert(a.getRequestLog().size()     == 0);
    assert(b.getRequestLog().size()     == 0);
    assert(c.getRequestLog().size()     == 0);
    
    for(int i = 0; i < 5; i++){
        a.log();
        b.log();
        c.log();
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    // now that B was leader trnasaction should commit and peers should be free from committee
    assert(a.isPrimary()                    == false);
    assert(b.isPrimary()                    == true);
    assert(c.isPrimary()                    == false);
    
    assert(a.getLedger().size()             == 1);
    assert(b.getLedger().size()             == 1);
    assert(c.getLedger().size()             == 1);
    
    assert(a.getCommitteeSizes()[0]         == 3);
    assert(b.getCommitteeSizes()[0]         == 3);
    assert(c.getCommitteeSizes()[0]         == 3);
    
    assert(a.getCommittee()                 == -1);
    assert(b.getCommittee()                 == -1);
    assert(c.getCommittee()                 == -1);
    
    assert(a.getLedger()[0].defeated        == false);
    assert(b.getLedger()[0].defeated        == false);
    assert(c.getLedger()[0].defeated        == false);
    
    assert(a.getCommitLog().size()          == 0);
    assert(b.getCommitLog().size()          == 0);
    assert(c.getCommitLog().size()          == 0);
    
    assert(a.getPrepareLog().size()         == 0);
    assert(b.getPrepareLog().size()         == 0);
    assert(c.getPrepareLog().size()         == 0);
    
    assert(a.getPrePrepareLog().size()      == 0);
    assert(b.getPrePrepareLog().size()      == 0);
    assert(c.getPrePrepareLog().size()      == 0);
    
    assert(a.getRequestLog().size()         == 0);
    assert(b.getRequestLog().size()         == 0);
    assert(c.getRequestLog().size()         == 0);
    
    ///////////////////////////////////////////
    // repeate Byzantine leader to make sure this keeps happing
    
    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    
    a.init();
    b.init();
    c.init();

    a.makeByzantine();
    a.makeRequest();
    
    for(int i = 0; i < 10; i++){
        a.log();
        b.log();
        c.log();
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    assert(a.isPrimary()                == false);
    assert(b.isPrimary()                == true);
    assert(c.isPrimary()                == false);
    
    assert(a.getLedger().size()         == 2);
    assert(b.getLedger().size()         == 2);
    assert(c.getLedger().size()         == 2);
    
    assert(a.getCommitteeSizes()[0]     == 3);
    assert(b.getCommitteeSizes()[0]     == 3);
    assert(c.getCommitteeSizes()[0]     == 3);
    assert(a.getCommitteeSizes()[1]     == 3);
    assert(b.getCommitteeSizes()[1]     == 3);
    assert(c.getCommitteeSizes()[1]     == 3);
    
    assert(a.getCommittee()             == -1);
    assert(b.getCommittee()             == -1);
    assert(c.getCommittee()             == -1);
    
    assert(a.getLedger()[0].defeated    == false);
    assert(b.getLedger()[0].defeated    == false);
    assert(c.getLedger()[0].defeated    == false);
    
    assert(a.getCommitLog().size()      == 0);
    assert(b.getCommitLog().size()      == 0);
    assert(c.getCommitLog().size()      == 0);
    
    assert(a.getPrepareLog().size()     == 0);
    assert(b.getPrepareLog().size()     == 0);
    assert(c.getPrepareLog().size()     == 0);
    
    assert(a.getPrePrepareLog().size()  == 0);
    assert(b.getPrePrepareLog().size()  == 0);
    assert(c.getPrePrepareLog().size()  == 0);
    
    assert(a.getRequestLog().size()     == 0);
    assert(b.getRequestLog().size()     == 0);
    assert(c.getRequestLog().size()     == 0);

    
    ///////////////////////////////////////////
    // test that committee size matchs ledger (add more committee members and test to make sure sizes line up)
    PBFTPeer_Sharded z = PBFTPeer_Sharded("Z");
    
    z.setFaultTolerance(1);
    
    z.setLogFile(log);

    a.addNeighbor(z, 1);
    b.addNeighbor(z, 1);
    c.addNeighbor(z, 1);

    z.addNeighbor(a, 1);
    z.addNeighbor(b, 1);
    z.addNeighbor(c, 1);
    
    z.log();

    // rm a from committee to adjust byz
    a.setCommittee(-1);
    a.makeCorrect();

    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    z.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    a.addCommitteeMember(z);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    b.addCommitteeMember(z);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    c.addCommitteeMember(z);
    
    z.addCommitteeMember(a);
    z.addCommitteeMember(b);
    z.addCommitteeMember(c);
    
    a.init();
    b.init();
    c.init();
    z.init();
    
    b.makeRequest(); // b is leader becouse of view change
    
    for(int i = 0; i < 5; i++){
        a.log();
        b.log();
        c.log();
        z.log();
        
        a.receive();
        b.receive();
        c.receive();
        z.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        z.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
        z.transmit();
    }
    
    assert(a.getLedger().size()         == 3);
    assert(b.getLedger().size()         == 3);
    assert(c.getLedger().size()         == 3);
    assert(z.getLedger().size()         == 1);
    
    assert(a.getCommitteeSizes()[0]     == 3);
    assert(b.getCommitteeSizes()[0]     == 3);
    assert(c.getCommitteeSizes()[0]     == 3);
    assert(z.getCommitteeSizes()[0]     == 4);
    assert(a.getCommitteeSizes()[1]     == 3);
    assert(b.getCommitteeSizes()[1]     == 3);
    assert(c.getCommitteeSizes()[1]     == 3);
    assert(a.getCommitteeSizes()[2]     == 4);
    assert(b.getCommitteeSizes()[2]     == 4);
    assert(c.getCommitteeSizes()[2]     == 4);
    
    assert(a.getCommittee()             == -1);
    assert(b.getCommittee()             == -1);
    assert(c.getCommittee()             == -1);
    assert(z.getCommittee()             == -1);
    
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testViewChange Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void testByzantine(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testByzantine "<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    
    ///////////////////////////////////////////
    // Byzantine majority and honest leader should view change for byz leader and commit defeated trnasaction
    
    PBFTPeer_Sharded a = PBFTPeer_Sharded("A");
    PBFTPeer_Sharded b = PBFTPeer_Sharded("B");
    PBFTPeer_Sharded c = PBFTPeer_Sharded("C");
    
    a.setFaultTolerance(0.6); // this will allow for defeated transactions with only 2 Byzantine peers
    b.setFaultTolerance(0.6);
    c.setFaultTolerance(0.6);
    
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    
    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    
    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    
    a.log();
    b.log();
    c.log();
    
    b.makeByzantine();
    c.makeByzantine();
    
    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    
    a.init();
    b.init();
    c.init();

    a.makeRequest();
    
    for(int i = 0; i < 10; i++){
        a.log();
        b.log();
        c.log();
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    // make sure that there was no view change and that the transaction was committed as defeated
    assert(a.isPrimary()                    == false);
    assert(b.isPrimary()                    == true);
    assert(c.isPrimary()                    == false);
    
    assert(a.getLedger().size()             == 1);
    assert(b.getLedger().size()             == 1);
    assert(c.getLedger().size()             == 1);
    
    assert(a.getCommitteeSizes()[0]         == 3);
    assert(b.getCommitteeSizes()[0]         == 3);
    assert(c.getCommitteeSizes()[0]         == 3);
    
    assert(a.getCommittee()                 == -1);
    assert(b.getCommittee()                 == -1);
    assert(c.getCommittee()                 == -1);
    
    assert(a.getLedger()[0].defeated        == true);
    assert(b.getLedger()[0].defeated        == true);
    assert(c.getLedger()[0].defeated        == true);
    
    assert(a.getCommitLog().size()          == 0);
    assert(b.getCommitLog().size()          == 0);
    assert(c.getCommitLog().size()          == 0);
    
    assert(a.getPrepareLog().size()         == 0);
    assert(b.getPrepareLog().size()         == 0);
    assert(c.getPrepareLog().size()         == 0);
    
    assert(a.getPrePrepareLog().size()      == 0);
    assert(b.getPrePrepareLog().size()      == 0);
    assert(c.getPrePrepareLog().size()      == 0);
    
    assert(a.getRequestLog().size()         == 0);
    assert(b.getRequestLog().size()         == 0);
    assert(c.getRequestLog().size()         == 0);
    
    ///////////////////////////////////////////
    // Byzantine majority and Byzantine leader commit defeated transaction
    
    a = PBFTPeer_Sharded("A");
    b = PBFTPeer_Sharded("B");
    c = PBFTPeer_Sharded("C");
    
    a.setFaultTolerance(0.6); // this will allow for defeated transactions with only 2 Byzantine peers
    b.setFaultTolerance(0.6);
    c.setFaultTolerance(0.6);
    
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    
    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    
    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    
    a.log();
    b.log();
    c.log();
    
    a.makeByzantine();
    c.makeByzantine();
    
    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    
    a.init();
    b.init();
    c.init();
    
    a.makeRequest();
    
    for(int i = 0; i < 5; i++){
        a.log();
        b.log();
        c.log();
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    // make sure that there was no view change and that the transaction was committed as defeated
    assert(a.isPrimary()                    == true);
    assert(b.isPrimary()                    == false);
    assert(c.isPrimary()                    == false);
    
    assert(a.getLedger().size()             == 1);
    assert(b.getLedger().size()             == 1);
    assert(c.getLedger().size()             == 1);
    
    assert(a.getCommitteeSizes()[0]         == 3);
    assert(b.getCommitteeSizes()[0]         == 3);
    assert(c.getCommitteeSizes()[0]         == 3);
    
    assert(a.getCommittee()                 == -1);
    assert(b.getCommittee()                 == -1);
    assert(c.getCommittee()                 == -1);
    
    assert(a.getLedger()[0].defeated        == true);
    assert(b.getLedger()[0].defeated        == true);
    assert(c.getLedger()[0].defeated        == true);
    
    assert(a.getCommitLog().size()          == 0);
    assert(b.getCommitLog().size()          == 0);
    assert(c.getCommitLog().size()          == 0);
    
    assert(a.getPrepareLog().size()         == 0);
    assert(b.getPrepareLog().size()         == 0);
    assert(c.getPrepareLog().size()         == 0);
    
    assert(a.getPrePrepareLog().size()      == 0);
    assert(b.getPrePrepareLog().size()      == 0);
    assert(c.getPrePrepareLog().size()      == 0);
    
    assert(a.getRequestLog().size()         == 0);
    assert(b.getRequestLog().size()         == 0);
    assert(c.getRequestLog().size()         == 0);
    
    ///////////////////////////////////////////
    // Honest majority byz leader does not commit defeated transaction
    
    a = PBFTPeer_Sharded("A");
    b = PBFTPeer_Sharded("B");
    c = PBFTPeer_Sharded("C");
    
    a.setFaultTolerance(0.6); // this will allow for defeated transactions with only 2 Byzantine peers
    b.setFaultTolerance(0.6);
    c.setFaultTolerance(0.6);
    
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    
    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    
    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    
    a.log();
    b.log();
    c.log();
    
    a.makeByzantine();
    
    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    
    a.init();
    b.init();
    c.init();
    
    a.makeRequest();
    
    for(int i = 0; i < 5; i++){
        a.log();
        b.log();
        c.log();
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    // make sure that there was a view change and that the transaction was not committed as defeated
    assert(a.isPrimary()                    == false);
    assert(b.isPrimary()                    == true);
    assert(c.isPrimary()                    == false);
    
    assert(a.getLedger().size()             == 0);
    assert(b.getLedger().size()             == 0);
    assert(c.getLedger().size()             == 0);
    
    for(int i = 0; i < 5; i++){
        a.log();
        b.log();
        c.log();
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    assert(a.isPrimary()                    == false);
    assert(b.isPrimary()                    == true);
    assert(c.isPrimary()                    == false);
    
    assert(a.getLedger().size()             == 1);
    assert(b.getLedger().size()             == 1);
    assert(c.getLedger().size()             == 1);
    
    assert(a.getCommitteeSizes()[0]         == 3);
    assert(b.getCommitteeSizes()[0]         == 3);
    assert(c.getCommitteeSizes()[0]         == 3);
    
    assert(a.getCommittee()                 == -1);
    assert(b.getCommittee()                 == -1);
    assert(c.getCommittee()                 == -1);
    
    assert(a.getLedger()[0].defeated        == false);
    assert(b.getLedger()[0].defeated        == false);
    assert(c.getLedger()[0].defeated        == false);
    
    assert(a.getCommitLog().size()          == 0);
    assert(b.getCommitLog().size()          == 0);
    assert(c.getCommitLog().size()          == 0);
    
    assert(a.getPrepareLog().size()         == 0);
    assert(b.getPrepareLog().size()         == 0);
    assert(c.getPrepareLog().size()         == 0);
    
    assert(a.getPrePrepareLog().size()      == 0);
    assert(b.getPrePrepareLog().size()      == 0);
    assert(c.getPrePrepareLog().size()      == 0);
    
    assert(a.getRequestLog().size()         == 0);
    assert(b.getRequestLog().size()         == 0);
    assert(c.getRequestLog().size()         == 0);
    
    /////////////////////////////////////////////////////////////
    // testing slow peers and byz majority
    //
    
    a = PBFTPeer_Sharded("A");
    b = PBFTPeer_Sharded("B");
    c = PBFTPeer_Sharded("C");
    
    a.setFaultTolerance(0.6); // this will allow for defeated transactions with only 2 Byzantine peers
    b.setFaultTolerance(0.6);
    c.setFaultTolerance(0.6);
    
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    
    a.addNeighbor(b, 5);
    a.addNeighbor(c, 5);
    
    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    
    a.log();
    b.log();
    c.log();
    
    b.makeByzantine();
    c.makeByzantine();
    
    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    
    a.init();
    b.init();
    c.init();
    
    a.makeRequest();
    
    for(int i = 0; i < 25; i++){
        a.log();
        b.log();
        c.log();
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    assert(a.isPrimary()                    == false);
    assert(b.isPrimary()                    == true);
    assert(c.isPrimary()                    == false);
    
    assert(a.getLedger().size()             == 1);
    assert(b.getLedger().size()             == 1);
    assert(c.getLedger().size()             == 1);
    
    assert(a.getCommitteeSizes()[0]         == 3);
    assert(b.getCommitteeSizes()[0]         == 3);
    assert(c.getCommitteeSizes()[0]         == 3);
    
    assert(a.getCommittee()                 == -1);
    assert(b.getCommittee()                 == -1);
    assert(c.getCommittee()                 == -1);
    
    assert(a.getLedger()[0].defeated        == true);
    assert(b.getLedger()[0].defeated        == true);
    assert(c.getLedger()[0].defeated        == true);
    
    assert(a.getCommitLog().size()          == 0);
    assert(b.getCommitLog().size()          == 0);
    assert(c.getCommitLog().size()          == 0);
    
    assert(a.getPrepareLog().size()         == 0);
    assert(b.getPrepareLog().size()         == 0);
    assert(c.getPrepareLog().size()         == 0);
    
    assert(a.getPrePrepareLog().size()      == 0);
    assert(b.getPrePrepareLog().size()      == 0);
    assert(c.getPrePrepareLog().size()      == 0);
    
    assert(a.getRequestLog().size()         == 0);
    assert(b.getRequestLog().size()         == 0);
    assert(c.getRequestLog().size()         == 0);
    
    a = PBFTPeer_Sharded("A");
    b = PBFTPeer_Sharded("B");
    c = PBFTPeer_Sharded("C");
    
    a.setFaultTolerance(0.6); // this will allow for defeated transactions with only 2 Byzantine peers
    b.setFaultTolerance(0.6);
    c.setFaultTolerance(0.6);
    
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    
    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    
    b.addNeighbor(a, 5);
    b.addNeighbor(c, 5);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    
    a.log();
    b.log();
    c.log();
    
    b.makeByzantine();
    c.makeByzantine();
    
    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    
    a.init();
    b.init();
    c.init();
    
    a.makeRequest();
    
    for(int i = 0; i < 25; i++){
        a.log();
        b.log();
        c.log();
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    assert(a.isPrimary()                    == false);
    assert(b.isPrimary()                    == true);
    assert(c.isPrimary()                    == false);
    
    assert(a.getLedger().size()             == 1);
    assert(b.getLedger().size()             == 1);
    assert(c.getLedger().size()             == 1);
    
    assert(a.getCommitteeSizes()[0]         == 3);
    assert(b.getCommitteeSizes()[0]         == 3);
    assert(c.getCommitteeSizes()[0]         == 3);
    
    assert(a.getCommittee()                 == -1);
    assert(b.getCommittee()                 == -1);
    assert(c.getCommittee()                 == -1);
    
    assert(a.getLedger()[0].defeated        == true);
    assert(b.getLedger()[0].defeated        == true);
    assert(c.getLedger()[0].defeated        == true);
    
    assert(a.getCommitLog().size()          == 0);
    assert(b.getCommitLog().size()          == 0);
    assert(c.getCommitLog().size()          == 0);
    
    assert(a.getPrepareLog().size()         == 0);
    assert(b.getPrepareLog().size()         == 0);
    assert(c.getPrepareLog().size()         == 0);
    
    assert(a.getPrePrepareLog().size()      == 0);
    assert(b.getPrePrepareLog().size()      == 0);
    assert(c.getPrePrepareLog().size()      == 0);
    
    assert(a.getRequestLog().size()         == 0);
    assert(b.getRequestLog().size()         == 0);
    assert(c.getRequestLog().size()         == 0);
    
    a = PBFTPeer_Sharded("A");
    b = PBFTPeer_Sharded("B");
    c = PBFTPeer_Sharded("C");
    
    a.setFaultTolerance(0.6); // this will allow for defeated transactions with only 2 Byzantine peers
    b.setFaultTolerance(0.6);
    c.setFaultTolerance(0.6);
    
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    
    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    
    b.addNeighbor(a, 5);
    b.addNeighbor(c, 5);
    
    c.addNeighbor(b, 5);
    c.addNeighbor(a, 5);
    
    a.log();
    b.log();
    c.log();
    
    b.makeByzantine();
    c.makeByzantine();
    
    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    
    a.init();
    b.init();
    c.init();
    
    a.makeRequest();
    
    for(int i = 0; i < 25; i++){
        a.log();
        b.log();
        c.log();
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    assert(a.isPrimary()                    == false);
    assert(b.isPrimary()                    == true);
    assert(c.isPrimary()                    == false);
    
    assert(a.getLedger().size()             == 1);
    assert(b.getLedger().size()             == 1);
    assert(c.getLedger().size()             == 1);
    
    assert(a.getCommitteeSizes()[0]         == 3);
    assert(b.getCommitteeSizes()[0]         == 3);
    assert(c.getCommitteeSizes()[0]         == 3);
    
    assert(a.getCommittee()                 == -1);
    assert(b.getCommittee()                 == -1);
    assert(c.getCommittee()                 == -1);
    
    assert(a.getLedger()[0].defeated        == true);
    assert(b.getLedger()[0].defeated        == true);
    assert(c.getLedger()[0].defeated        == true);
    
    assert(a.getCommitLog().size()          == 0);
    assert(b.getCommitLog().size()          == 0);
    assert(c.getCommitLog().size()          == 0);
    
    assert(a.getPrepareLog().size()         == 0);
    assert(b.getPrepareLog().size()         == 0);
    assert(c.getPrepareLog().size()         == 0);
    
    assert(a.getPrePrepareLog().size()      == 0);
    assert(b.getPrePrepareLog().size()      == 0);
    assert(c.getPrePrepareLog().size()      == 0);
    
    assert(a.getRequestLog().size()         == 0);
    assert(b.getRequestLog().size()         == 0);
    assert(c.getRequestLog().size()         == 0);
    
    a = PBFTPeer_Sharded("A");
    b = PBFTPeer_Sharded("B");
    c = PBFTPeer_Sharded("C");
    
    a.setFaultTolerance(0.6); // this will allow for defeated transactions with only 2 Byzantine peers
    b.setFaultTolerance(0.6);
    c.setFaultTolerance(0.6);
    
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    
    a.addNeighbor(b, 5);
    a.addNeighbor(c, 5);
    
    b.addNeighbor(a, 5);
    b.addNeighbor(c, 5);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    
    a.log();
    b.log();
    c.log();
    
    b.makeByzantine();
    c.makeByzantine();
    
    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    
    a.init();
    b.init();
    c.init();
    
    a.makeRequest();
    
    for(int i = 0; i < 25; i++){
        a.log();
        b.log();
        c.log();
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    assert(a.isPrimary()                    == false);
    assert(b.isPrimary()                    == true);
    assert(c.isPrimary()                    == false);
    
    assert(a.getLedger().size()             == 1);
    assert(b.getLedger().size()             == 1);
    assert(c.getLedger().size()             == 1);
    
    assert(a.getCommitteeSizes()[0]         == 3);
    assert(b.getCommitteeSizes()[0]         == 3);
    assert(c.getCommitteeSizes()[0]         == 3);
    
    assert(a.getCommittee()                 == -1);
    assert(b.getCommittee()                 == -1);
    assert(c.getCommittee()                 == -1);
    
    assert(a.getLedger()[0].defeated        == true);
    assert(b.getLedger()[0].defeated        == true);
    assert(c.getLedger()[0].defeated        == true);
    
    assert(a.getCommitLog().size()          == 0);
    assert(b.getCommitLog().size()          == 0);
    assert(c.getCommitLog().size()          == 0);
    
    assert(a.getPrepareLog().size()         == 0);
    assert(b.getPrepareLog().size()         == 0);
    assert(c.getPrepareLog().size()         == 0);
    
    assert(a.getPrePrepareLog().size()      == 0);
    assert(b.getPrePrepareLog().size()      == 0);
    assert(c.getPrePrepareLog().size()      == 0);
    
    assert(a.getRequestLog().size()         == 0);
    assert(b.getRequestLog().size()         == 0);
    assert(c.getRequestLog().size()         == 0);
    
    /////////////////////////////////////////////////////////////
    // testing slow peers and honest majority
    //
    
    a = PBFTPeer_Sharded("A");
    b = PBFTPeer_Sharded("B");
    c = PBFTPeer_Sharded("C");
    
    a.setFaultTolerance(0.6); // this will allow for defeated transactions with only 2 Byzantine peers
    b.setFaultTolerance(0.6);
    c.setFaultTolerance(0.6);
    
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    
    a.addNeighbor(b, 5);
    a.addNeighbor(c, 5);
    
    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    
    a.log();
    b.log();
    c.log();
    
    a.makeByzantine();
    
    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    
    a.init();
    b.init();
    c.init();
    
    a.makeRequest();
    
    for(int i = 0; i < 25; i++){
        a.log();
        b.log();
        c.log();
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    assert(a.isPrimary()                    == false);
    assert(b.isPrimary()                    == true);
    assert(c.isPrimary()                    == false);
    
    assert(a.getLedger().size()             == 1);
    assert(b.getLedger().size()             == 1);
    assert(c.getLedger().size()             == 1);
    
    assert(a.getCommitteeSizes()[0]         == 3);
    assert(b.getCommitteeSizes()[0]         == 3);
    assert(c.getCommitteeSizes()[0]         == 3);
    
    assert(a.getCommittee()                 == -1);
    assert(b.getCommittee()                 == -1);
    assert(c.getCommittee()                 == -1);
    
    assert(a.getLedger()[0].defeated        == false);
    assert(b.getLedger()[0].defeated        == false);
    assert(c.getLedger()[0].defeated        == false);
    
    assert(a.getCommitLog().size()          == 0);
    assert(b.getCommitLog().size()          == 0);
    assert(c.getCommitLog().size()          == 0);
    
    assert(a.getPrepareLog().size()         == 0);
    assert(b.getPrepareLog().size()         == 0);
    assert(c.getPrepareLog().size()         == 0);
    
    assert(a.getPrePrepareLog().size()      == 0);
    assert(b.getPrePrepareLog().size()      == 0);
    assert(c.getPrePrepareLog().size()      == 0);
    
    assert(a.getRequestLog().size()         == 0);
    assert(b.getRequestLog().size()         == 0);
    assert(c.getRequestLog().size()         == 0);
    
    a = PBFTPeer_Sharded("A");
    b = PBFTPeer_Sharded("B");
    c = PBFTPeer_Sharded("C");
    
    a.setFaultTolerance(0.6); // this will allow for defeated transactions with only 2 Byzantine peers
    b.setFaultTolerance(0.6);
    c.setFaultTolerance(0.6);
    
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    
    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    
    b.addNeighbor(a, 5);
    b.addNeighbor(c, 5);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    
    a.log();
    b.log();
    c.log();
    
    a.makeByzantine();
    
    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    
    a.init();
    b.init();
    c.init();
    
    a.makeRequest();
    
    for(int i = 0; i < 25; i++){
        a.log();
        b.log();
        c.log();
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    assert(a.isPrimary()                    == false);
    assert(b.isPrimary()                    == true);
    assert(c.isPrimary()                    == false);
    
    assert(a.getLedger().size()             == 1);
    assert(b.getLedger().size()             == 1);
    assert(c.getLedger().size()             == 1);
    
    assert(a.getCommitteeSizes()[0]         == 3);
    assert(b.getCommitteeSizes()[0]         == 3);
    assert(c.getCommitteeSizes()[0]         == 3);
    
    assert(a.getCommittee()                 == -1);
    assert(b.getCommittee()                 == -1);
    assert(c.getCommittee()                 == -1);
    
    assert(a.getLedger()[0].defeated        == false);
    assert(b.getLedger()[0].defeated        == false);
    assert(c.getLedger()[0].defeated        == false);
    
    assert(a.getCommitLog().size()          == 0);
    assert(b.getCommitLog().size()          == 0);
    assert(c.getCommitLog().size()          == 0);
    
    assert(a.getPrepareLog().size()         == 0);
    assert(b.getPrepareLog().size()         == 0);
    assert(c.getPrepareLog().size()         == 0);
    
    assert(a.getPrePrepareLog().size()      == 0);
    assert(b.getPrePrepareLog().size()      == 0);
    assert(c.getPrePrepareLog().size()      == 0);
    
    assert(a.getRequestLog().size()         == 0);
    assert(b.getRequestLog().size()         == 0);
    assert(c.getRequestLog().size()         == 0);
    
    a = PBFTPeer_Sharded("A");
    b = PBFTPeer_Sharded("B");
    c = PBFTPeer_Sharded("C");
    
    a.setFaultTolerance(0.6); // this will allow for defeated transactions with only 2 Byzantine peers
    b.setFaultTolerance(0.6);
    c.setFaultTolerance(0.6);
    
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    
    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    
    b.addNeighbor(a, 5);
    b.addNeighbor(c, 5);
    
    c.addNeighbor(b, 5);
    c.addNeighbor(a, 5);
    
    a.log();
    b.log();
    c.log();
    
    a.makeByzantine();
    
    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    
    a.init();
    b.init();
    c.init();
    
    a.makeRequest();
    
    for(int i = 0; i < 25; i++){
        a.log();
        b.log();
        c.log();
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    assert(a.isPrimary()                    == false);
    assert(b.isPrimary()                    == true);
    assert(c.isPrimary()                    == false);
    
    assert(a.getLedger().size()             == 1);
    assert(b.getLedger().size()             == 1);
    assert(c.getLedger().size()             == 1);
    
    assert(a.getCommitteeSizes()[0]         == 3);
    assert(b.getCommitteeSizes()[0]         == 3);
    assert(c.getCommitteeSizes()[0]         == 3);
    
    assert(a.getCommittee()                 == -1);
    assert(b.getCommittee()                 == -1);
    assert(c.getCommittee()                 == -1);
    
    assert(a.getLedger()[0].defeated        == false);
    assert(b.getLedger()[0].defeated        == false);
    assert(c.getLedger()[0].defeated        == false);
    
    assert(a.getCommitLog().size()          == 0);
    assert(b.getCommitLog().size()          == 0);
    assert(c.getCommitLog().size()          == 0);
    
    assert(a.getPrepareLog().size()         == 0);
    assert(b.getPrepareLog().size()         == 0);
    assert(c.getPrepareLog().size()         == 0);
    
    assert(a.getPrePrepareLog().size()      == 0);
    assert(b.getPrePrepareLog().size()      == 0);
    assert(c.getPrePrepareLog().size()      == 0);
    
    assert(a.getRequestLog().size()         == 0);
    assert(b.getRequestLog().size()         == 0);
    assert(c.getRequestLog().size()         == 0);
    
    a = PBFTPeer_Sharded("A");
    b = PBFTPeer_Sharded("B");
    c = PBFTPeer_Sharded("C");
    
    a.setFaultTolerance(0.6); // this will allow for defeated transactions with only 2 Byzantine peers
    b.setFaultTolerance(0.6);
    c.setFaultTolerance(0.6);
    
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    
    a.addNeighbor(b, 5);
    a.addNeighbor(c, 5);
    
    b.addNeighbor(a, 5);
    b.addNeighbor(c, 5);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    
    a.log();
    b.log();
    c.log();
    
    a.makeByzantine();
    
    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    
    a.init();
    b.init();
    c.init();
    
    a.makeRequest();
    
    for(int i = 0; i < 25; i++){
        a.log();
        b.log();
        c.log();
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    assert(a.isPrimary()                    == false);
    assert(b.isPrimary()                    == true);
    assert(c.isPrimary()                    == false);
    
    assert(a.getLedger().size()             == 1);
    assert(b.getLedger().size()             == 1);
    assert(c.getLedger().size()             == 1);
    
    assert(a.getCommitteeSizes()[0]         == 3);
    assert(b.getCommitteeSizes()[0]         == 3);
    assert(c.getCommitteeSizes()[0]         == 3);
    
    assert(a.getCommittee()                 == -1);
    assert(b.getCommittee()                 == -1);
    assert(c.getCommittee()                 == -1);
    
    assert(a.getLedger()[0].defeated        == false);
    assert(b.getLedger()[0].defeated        == false);
    assert(c.getLedger()[0].defeated        == false);
    
    assert(a.getCommitLog().size()          == 0);
    assert(b.getCommitLog().size()          == 0);
    assert(c.getCommitLog().size()          == 0);
    
    assert(a.getPrepareLog().size()         == 0);
    assert(b.getPrepareLog().size()         == 0);
    assert(c.getPrepareLog().size()         == 0);
    
    assert(a.getPrePrepareLog().size()      == 0);
    assert(b.getPrePrepareLog().size()      == 0);
    assert(c.getPrePrepareLog().size()      == 0);
    
    assert(a.getRequestLog().size()         == 0);
    assert(b.getRequestLog().size()         == 0);
    assert(c.getRequestLog().size()         == 0);
    
    /////////////////////////////////////////////////////////////
    // testing slow peers and honest majority multi view changes
    //
    
    a = PBFTPeer_Sharded("A");
    b = PBFTPeer_Sharded("B");
    c = PBFTPeer_Sharded("C");
    PBFTPeer_Sharded d = PBFTPeer_Sharded("D");
    PBFTPeer_Sharded e = PBFTPeer_Sharded("E");

    a.setFaultTolerance(0.6); // this will allow for defeated transactions with only 2 Byzantine peers
    b.setFaultTolerance(0.6);
    c.setFaultTolerance(0.6);
    d.setFaultTolerance(0.6);
    e.setFaultTolerance(0.6);
    
    
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    e.setLogFile(log);
    d.setLogFile(log);
    
    a.addNeighbor(b, 5);
    a.addNeighbor(c, 1);
    a.addNeighbor(e, 5);
    a.addNeighbor(d, 1);
    
    b.addNeighbor(a, 5);
    b.addNeighbor(c, 1);
    b.addNeighbor(e, 1);
    b.addNeighbor(d, 1);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    c.addNeighbor(e, 1);
    c.addNeighbor(d, 1);
    
    e.addNeighbor(b, 1);
    e.addNeighbor(a, 5);
    e.addNeighbor(c, 1);
    e.addNeighbor(d, 1);
    
    d.addNeighbor(b, 1);
    d.addNeighbor(a, 1);
    d.addNeighbor(c, 1);
    d.addNeighbor(e, 1);
    
    a.log();
    b.log();
    c.log();
    e.log();
    d.log();
    
    a.makeByzantine();
    b.makeByzantine();
    
    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    d.setCommittee(1);
    e.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    a.addCommitteeMember(d);
    a.addCommitteeMember(e);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    b.addCommitteeMember(d);
    b.addCommitteeMember(e);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    c.addCommitteeMember(d);
    c.addCommitteeMember(e);
    
    d.addCommitteeMember(b);
    d.addCommitteeMember(a);
    d.addCommitteeMember(c);
    d.addCommitteeMember(e);
    
    e.addCommitteeMember(b);
    e.addCommitteeMember(a);
    e.addCommitteeMember(d);
    e.addCommitteeMember(c);
    
    a.init();
    b.init();
    c.init();
    d.init();
    e.init();
    
    a.makeRequest();
    
    for(int i = 0; i < 50; i++){
        a.log();
        b.log();
        c.log();
        d.log();
        e.log();
        
        a.receive();
        b.receive();
        c.receive();
        d.receive();
        e.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        d.preformComputation();
        e.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
        d.transmit();
        e.transmit();
    }
    
    assert(a.isPrimary()                    == false);
    assert(b.isPrimary()                    == false);
    assert(c.isPrimary()                    == true);
    assert(d.isPrimary()                    == false);
    assert(e.isPrimary()                    == false);
    
    assert(a.getLedger().size()             == 1);
    assert(b.getLedger().size()             == 1);
    assert(c.getLedger().size()             == 1);
    assert(d.getLedger().size()             == 1);
    assert(e.getLedger().size()             == 1);
    
    assert(a.getCommitteeSizes()[0]         == 5);
    assert(b.getCommitteeSizes()[0]         == 5);
    assert(c.getCommitteeSizes()[0]         == 5);
    assert(d.getCommitteeSizes()[0]         == 5);
    assert(e.getCommitteeSizes()[0]         == 5);
    
    assert(a.getCommittee()                 == -1);
    assert(b.getCommittee()                 == -1);
    assert(c.getCommittee()                 == -1);
    assert(d.getCommittee()                 == -1);
    assert(e.getCommittee()                 == -1);
    
    assert(a.getLedger()[0].defeated        == false);
    assert(b.getLedger()[0].defeated        == false);
    assert(c.getLedger()[0].defeated        == false);
    assert(d.getLedger()[0].defeated        == false);
    assert(e.getLedger()[0].defeated        == false);
    
    assert(a.getCommitLog().size()          == 0);
    assert(b.getCommitLog().size()          == 0);
    assert(c.getCommitLog().size()          == 0);
    assert(d.getCommitLog().size()          == 0);
    assert(e.getCommitLog().size()          == 0);
    
    assert(a.getPrepareLog().size()         == 0);
    assert(b.getPrepareLog().size()         == 0);
    assert(c.getPrepareLog().size()         == 0);
    assert(d.getPrepareLog().size()         == 0);
    assert(e.getPrepareLog().size()         == 0);
    
    assert(a.getPrePrepareLog().size()      == 0);
    assert(b.getPrePrepareLog().size()      == 0);
    assert(c.getPrePrepareLog().size()      == 0);
    assert(d.getPrePrepareLog().size()         == 0);
    assert(e.getPrePrepareLog().size()         == 0);
    
    assert(a.getRequestLog().size()         == 0);
    assert(b.getRequestLog().size()         == 0);
    assert(c.getRequestLog().size()         == 0);
    assert(d.getRequestLog().size()         == 0);
    assert(e.getRequestLog().size()         == 0);
    
    
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testByzantine Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}

void waitingTimeSharded(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"waitingTime"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    
    ////////////////////////////////////////////////////////
    // no view change
    
    PBFTPeer_Sharded a = PBFTPeer_Sharded("A");
    PBFTPeer_Sharded b = PBFTPeer_Sharded("B");
    PBFTPeer_Sharded c = PBFTPeer_Sharded("C");
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    
    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    
    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    
    a.setFaultTolerance(1);
    b.setFaultTolerance(1);
    c.setFaultTolerance(1);
    
    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    
    a.init();
    b.init();
    c.init();
    
    a.makeRequest();
    
    for(int i = 0; i < 5; i++){
        a.log();
        b.log();
        c.log();
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    assert(a.getLedger().size()                 == 1);
    assert(b.getLedger().size()                 == 1);
    assert(c.getLedger().size()                 == 1);
    
    assert(a.getLedger()[0].submission_round    == 0);
    assert(b.getLedger()[0].submission_round    == 0);
    assert(c.getLedger()[0].submission_round    == 0);
    
    assert(a.getLedger()[0].commit_round        == 4);
    assert(b.getLedger()[0].commit_round        == 4);
    assert(c.getLedger()[0].commit_round        == 4);
    
    ////////////////////////////////////////////////////////
    // with view change
    
    a = PBFTPeer_Sharded("A");
    b = PBFTPeer_Sharded("B");
    c = PBFTPeer_Sharded("C");
    a.setLogFile(log);
    b.setLogFile(log);
    c.setLogFile(log);
    
    a.addNeighbor(b, 1);
    a.addNeighbor(c, 1);
    
    b.addNeighbor(a, 1);
    b.addNeighbor(c, 1);
    
    c.addNeighbor(b, 1);
    c.addNeighbor(a, 1);
    
    a.setFaultTolerance(0.6);
    b.setFaultTolerance(0.6);
    c.setFaultTolerance(0.6);
    
    a.makeByzantine();
    
    a.setCommittee(1);
    b.setCommittee(1);
    c.setCommittee(1);
    
    a.addCommitteeMember(b);
    a.addCommitteeMember(c);
    
    b.addCommitteeMember(a);
    b.addCommitteeMember(c);
    
    c.addCommitteeMember(b);
    c.addCommitteeMember(a);
    
    a.init();
    b.init();
    c.init();
    
    a.makeRequest();
    
    for(int i = 0; i < 10; i++){
        a.log();
        b.log();
        c.log();
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
    }
    
    assert(a.getLedger().size()                 == 1);
    assert(b.getLedger().size()                 == 1);
    assert(c.getLedger().size()                 == 1);
    
    assert(a.getLedger()[0].submission_round    == 0);
    assert(b.getLedger()[0].submission_round    == 0);
    assert(c.getLedger()[0].submission_round    == 0);
    
    assert(a.getLedger()[0].commit_round        == 8);
    assert(b.getLedger()[0].commit_round        == 8);
    assert(c.getLedger()[0].commit_round        == 8);
    
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"waitingTime Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}


