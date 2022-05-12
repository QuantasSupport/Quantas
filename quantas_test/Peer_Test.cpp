/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Peer_Test.hpp"

using quantas::Packet;
using quantas::LOG_WIDTH;


void TestPeer::makeRequest(){
    
    for (int i = 0; i <  neighbors().size(); i++){
        long target = neighbors()[i];
        Packet<TestMessage> newMessage(1, target, id()); 
        pushToOutSteam(newMessage);
    }

}

void TestPeer::performComputation(){
    Packet<TestMessage> msg = popInStream();
    std::cout << "Test NetworkInterface " << id() << " got a message from " << msg.sourceId() << std::endl;
}

void RunPeerTests(std::string filepath){
    std::ofstream log;
    log.open(filepath + "/BasePeerTest.log");
    if (log.fail() ){
        std::cerr << "Error: could not open file at: "<< filepath << std::endl;
    }

    testConst(log);
    testNeighbor(log);
    testRecTrans(log);

    log.close();
}

void testConst(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<__PRETTY_FUNCTION__<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    
    // // defulte 
    // TestPeer none = TestPeer();
    // assert("NO ID" == none.id());
    // assert(std::deque<Packet<TestMessage> >() == none.getInStream());
    // assert(std::deque<Packet<TestMessage> >() == none.getOutStream());
    // assert(std::vector<std::string>() == none.neighbors());
    // assert(0 == none.getMessageCount());
    // assert(0 == none.getClock());
    // assert(false == none.isByzantine());
    // assert(false == none.isBusy());
    // assert(false == none.isNeighbor(""));
    // assert(false == none.isNeighbor("NO ID"));

    // // copying
    // TestPeer a = TestPeer("a");
    // TestPeer b = TestPeer("");
    // assert("a" == a.id());
    // assert(std::deque<Packet<TestMessage> >() == a.getInStream());
    // assert(std::deque<Packet<TestMessage> >() == a.getOutStream());
    // assert(std::vector<std::string>() == a.neighbors());
    // assert(0 == a.getMessageCount());
    // assert(0 == a.getClock());
    // assert(false == a.isByzantine());
    // assert(false == a.isBusy());
    // assert(false == a.isNeighbor(""));
    // assert(false == a.isNeighbor("NO ID"));

    // assert("" == b.id());
    // assert(std::deque<Packet<TestMessage> >() == b.getInStream());
    // assert(std::deque<Packet<TestMessage> >() == b.getOutStream());
    // assert(std::vector<std::string>() == b.neighbors());
    // assert(0 == b.getMessageCount());
    // assert(0 == b.getClock());
    // assert(false == b.isByzantine());
    // assert(false == b.isBusy());
    // assert(false == b.isNeighbor(""));
    // assert(false == b.isNeighbor("NO ID"));
    

    // TestPeer aCopy = a;
    // assert("a" == aCopy.id());
    // assert(std::deque<Packet<TestMessage> >() == aCopy.getInStream());
    // assert(std::deque<Packet<TestMessage> >() == aCopy.getOutStream());
    // assert(std::vector<std::string>() == aCopy.neighbors());
    // assert(0 == aCopy.getMessageCount());
    // assert(0 == aCopy.getClock());
    // assert(false == aCopy.isByzantine());
    // assert(false == aCopy.isBusy());
    // assert(false == aCopy.isNeighbor(""));
    // assert(false == aCopy.isNeighbor("NO ID"));
    
    // TestPeer bCopy = b;
    // assert("" == bCopy.id());
    // assert(std::deque<Packet<TestMessage> >() == bCopy.getInStream());
    // assert(std::deque<Packet<TestMessage> >() == bCopy.getOutStream());
    // assert(std::vector<std::string>() == bCopy.neighbors());
    // assert(0 == bCopy.getMessageCount());
    // assert(0 == bCopy.getClock());
    // assert(false == bCopy.isByzantine());
    // assert(false == bCopy.isBusy());
    // assert(false == bCopy.isNeighbor(""));
    // assert(false == bCopy.isNeighbor("NO ID"));

    // TestPeer aCopyConst = TestPeer(a);
    // assert("a" == aCopyConst.id());
    // std::deque<Packet<TestMessage> > empy = std::deque<Packet<TestMessage> >();
    // assert( empy == aCopyConst.getInStream());
    // assert(std::deque<Packet<TestMessage> >() == aCopyConst.getOutStream());
    // assert(std::vector<std::string>() == aCopyConst.neighbors());
    // assert(0 == aCopyConst.getMessageCount());
    // assert(0 == aCopyConst.getClock());
    // assert(false == aCopyConst.isByzantine());
    // assert(false == aCopyConst.isBusy());
    // assert(false == aCopyConst.isNeighbor(""));
    // assert(false == aCopyConst.isNeighbor("NO ID"));

    // TestPeer bCopyConst = TestPeer(b);
    // assert("" == bCopyConst.id());
    // assert(std::deque<Packet<TestMessage> >() == bCopyConst.getInStream());
    // assert(std::deque<Packet<TestMessage> >() == bCopyConst.getOutStream());
    // assert(std::vector<std::string>() == bCopyConst.neighbors());
    // assert(0 == bCopyConst.getMessageCount());
    // assert(0 == bCopyConst.getClock());
    // assert(false == bCopyConst.isByzantine());
    // assert(false == bCopyConst.isBusy());
    // assert(false == bCopyConst.isNeighbor(""));
    // assert(false == bCopyConst.isNeighbor("NO ID"));

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<__PRETTY_FUNCTION__ << "PASS" << std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}

void testNeighbor(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<__PRETTY_FUNCTION__<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    
    long id_a = 1;
    long id_b = 2;

    TestPeer a = TestPeer(id_a);
    TestPeer b = TestPeer(id_b);
    a.addChannel(b, 10);

    assert(a.getDelayToNeighbor(id_b) == 10);
    assert(a.isNeighbor(id_b) == true);
    assert(a.neighbors()[0] == id_b);
    assert(a.neighbors().size() == 1);

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<__PRETTY_FUNCTION__ << "PASS" << std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}

void testRecTrans(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<__PRETTY_FUNCTION__<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    
    long id_a = 1;
    long id_b = 2;

    TestPeer a = TestPeer(id_a);
    TestPeer b = TestPeer(id_b);

    // test nothing happens on empty round (no msgs)
    // round one
    a.receive();
    b.receive();
    a.performComputation();
    b.performComputation();
    a.transmit();
    b.transmit();

    // test basic msg send and receive 
    a.addChannel(b, 1);
    b.addChannel(a, 1);

    a.makeRequest();
    b.makeRequest();

    // assert(a.getInStream().size() == 0);
    // assert(a.getOutStream().size() == 1);

    // assert(b.getInStream().size() == 0);
    // assert(b.getOutStream().size() == 1);

    a.receive();
    b.receive();

    // assert(a.getInStream().size() == 0);
    // assert(a.getOutStream().size() == 1);

    // assert(b.getInStream().size() == 0);
    // assert(b.getOutStream().size() == 1);

    a.performComputation();
    b.performComputation();

    a.transmit();
    b.transmit();

    // assert(a.getInStream().size() == 0);
    // assert(a.getOutStream().size() == 0);

    // assert(b.getInStream().size() == 0);
    // assert(b.getOutStream().size() == 0);

    a.receive();
    b.receive();

    // assert(a.getInStream().size() == 1);
    // assert(a.getOutStream().size() == 0);

    // assert(b.getInStream().size() == 1);
    // assert(b.getOutStream().size() == 0);

    a.performComputation();
    b.performComputation();

    a.transmit();
    b.transmit();

    a.receive();
    b.receive();

    // assert(a.getInStream().size() == 0);
    // assert(a.getOutStream().size() == 0);

    // assert(b.getInStream().size() == 0);
    // assert(b.getOutStream().size() == 0);

    a.performComputation();
    b.performComputation();

    a.transmit();
    b.transmit();

    /* make sure multiple messages from the same peer are not 
       impeeded by each other
    ex: A sends msgs X then Y to B
       
    case 1: both have a delay of 1 after 1 round both X and Y 
            should arrive at B
    case 2: X has a delay of 2 and Y has a delay of 1 then
            in 2 rounds both X and Y are receved by B
    case 3: X has a delay of 1 and Y has a delay of 2 then
            in 1 round X is receved in 2 rounds Y is receved
    */ 


    // case 1
    a.makeRequest();
    a.makeRequest();

    // assert(a.getInStream().size() == 0);
    // assert(a.getOutStream().size() == 2);

    // assert(b.getInStream().size() == 0);
    // assert(b.getOutStream().size() == 0);

    // round one
    a.receive();
    b.receive();
    a.performComputation();
    b.performComputation();
    a.transmit();
    b.transmit();

    // round two
    a.receive();
    b.receive();
    // assert(a.getInStream().size() == 0);
    // assert(a.getOutStream().size() == 0);

    // assert(b.getInStream().size() == 2);
    // assert(b.getOutStream().size() == 0);

    a.performComputation();
    b.performComputation();
    a.transmit();
    b.transmit();

    // case 2
    a.makeRequest();
    a.makeRequest();

    // round one
    a.receive();
    b.receive();
    
    // assert(a.getInStream().size() == 0);
    // assert(a.getOutStream().size() == 2);

    // assert(b.getInStream().size() == 0);
    // assert(b.getOutStream().size() == 0);
    
    a.performComputation();
    b.performComputation();
    a.transmit();
    b.transmit();
    // b.getChannelFrom(id_a)->front().setDelay(2,2); // force delay on packet to be 2 rounds 

    // round two
    a.receive();
    b.receive();
    a.performComputation();
    b.performComputation();
    a.transmit();
    b.transmit();

    // round three
    a.receive();
    b.receive();
    // assert(a.getInStream().size() == 0);
    // assert(a.getOutStream().size() == 0);

    // assert(b.getInStream().size() == 2);
    // assert(b.getOutStream().size() == 0);

    a.performComputation();
    b.performComputation();
    a.transmit();
    b.transmit();

    // case 3 
    a.makeRequest();
    a.makeRequest();

    // assert(a.getInStream().size() == 0);
    // assert(a.getOutStream().size() == 2);

    // assert(b.getInStream().size() == 0);
    // assert(b.getOutStream().size() == 0);

    // round one
    a.receive();
    b.receive();
    a.performComputation();
    b.performComputation();
    a.transmit();
    b.transmit();
    // b.getChannelFrom(id_a)->at(1).setDelay(2,2); // force delay on packet to be 2 rounds 

    // round two
    a.receive();
    b.receive();

    // assert(a.getInStream().size() == 0);
    // assert(a.getOutStream().size() == 0);

    // assert(b.getInStream().size() == 1);
    // assert(b.getOutStream().size() == 0);

    a.performComputation();
    b.performComputation();
    a.transmit();
    b.transmit();

    // round three
    a.receive();
    b.receive();

    // assert(a.getInStream().size() == 0);
    // assert(a.getOutStream().size() == 0);

    // assert(b.getInStream().size() == 1);
    // assert(b.getOutStream().size() == 0);

    a.performComputation();
    b.performComputation();
    a.transmit();
    b.transmit();

    // round four
    a.receive();
    b.receive();

    // assert(a.getInStream().size() == 0);
    // assert(a.getOutStream().size() == 0);

    // assert(b.getInStream().size() == 0);
    // assert(b.getOutStream().size() == 0);

    a.performComputation();
    b.performComputation();
    a.transmit();
    b.transmit();

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<__PRETTY_FUNCTION__ << "PASS" << std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
