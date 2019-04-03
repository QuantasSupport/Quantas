//
//  main.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <set>
#include "Blockchain.hpp"
#include "Peer.hpp"
#include "ExamplePeer.hpp"
#include "PBFT_Peer.hpp"
#include "syncBFT_Peer.hpp"
#include "Network.hpp"
#include "bCoin_Peer.hpp"
#include <iostream>
#include <chrono>
#include <random>

#include "BlockGuardPeer_Sharded.hpp"

const int peerCount = 10;
const int blockChainLength = 100;
Blockchain *blockchain;

void buildInitialChain(std::vector<std::string>);
std::set<std::string> getPeersForConsensus(int);
int syncBFT_Peer::peerCount = 3;
int shuffleByzantineInterval = 0;

void Example();
void PBFT(std::ofstream &out,int);
void syncBFT(std::ofstream &out,int avgDelay);
void bitcoin(std::ofstream &,int );
void bsg(std::ofstream &out,int);

int main(int argc, const char * argv[]) {
    srand((float)time(NULL));
    
    std::string algorithm = argv[1];
    std::string filePath = argv[2];

    /*
    buildInitialChain({"P0","P1","P2","P3","P4","P5","P6","P7","P8","P9"});
    for(const auto &s: getPeersForConsensus(2)){
        std::cerr<<s<<std::endl;
    }
    */

    if(algorithm == "example"){
        Example();
    }
    else if (algorithm== "pbft"){
        for(int delay = 2; delay < 51; delay = delay + 10){
            std::cout<< "Delay:"+std::to_string(delay)<< std::endl;
            std::ofstream out;
            out.open(filePath + "/PBFT_Delay"+std::to_string(delay) + ".log");
            for(int run = 0; run < 5; run++){
                std::cout<< "run:"<<run<<std::endl;
                PBFT(out,delay);
            }
            out.close();
        }

    }else if (algorithm == "syncBFT") {
        std::ofstream out;
        for(int delay = 1;delay<2; delay+=10){
            std::cout<< "Start with Delay "+std::to_string(delay)<< std::endl;
            for(int run = 0; run < 2; run++){
                syncBFT(out, delay);
            }

        }
    }else if (algorithm == "bitcoin") {
    }else if (algorithm == "bgs") {
        std::ofstream out;
        bitcoin(out, 1);
        out.open(filePath + "/BGS_Delay"+std::to_string(1) + ".log");
        bsg(out,1);
        out.close();
    }
    
    return 0;
}

void PBFT(std::ofstream &out,int avgDelay){
    
    Network<PBFT_Message, PBFT_Peer> system;
    system.setToPoisson();
    system.setLog(std::cout);
    system.initNetwork(100,avgDelay);
    for(int i = 0; i < system.size(); i++){
        system[i]->setFaultTolerance(0.3);
        system[i]->init();
    }
    
    int numberOfRequests = 0;
    for(int i =-1; i < 250; i++){
        if(i%100 == 0){
            //std::cout<< std::endl;
        }
        //std::cout<< "."<< std::flush;
        
        if(i%5 == 0){
            int randIndex = rand()%system.size();
            while(system[randIndex]->isPrimary()){
                randIndex = rand()%system.size();
            }
            system.makeRequest(randIndex);
            numberOfRequests++;
        }
        
        system.receive();
        system.preformComputation();
        system.transmit();
        //system.log();
    }

    int min = (int)system[0]->getLedger().size();
    int max = (int)system[0]->getLedger().size();
    for(int i = 0; i < system.size(); i++){
        //out<< "Peer ID:"<< system[i].id() << " Ledger Size:"<< system[i].getLedger().size()<< std::endl;
        if(system[i]->getLedger().size() < min){
            min = (int)system[i]->getLedger().size();
        }
        if(system[i]->getLedger().size() > max){
            max = (int)system[i]->getLedger().size();
        }
    }
    out<< "Min Ledger:,"<< min<< std::endl;
    out<< "Max Ledger:,"<< max<< std::endl;
    out<< "Total Request:,"<< numberOfRequests<<std::endl;
}

void syncBFT(std::ofstream &out,int maxDelay){
//    srand(1);
    srand(time(nullptr));

    syncBFT_Peer::changeLeader = true;
    syncBFT_Peer::leaderIdCandidates = {};
    syncBFT_Peer::leaderId = "";
    syncBFT_Peer::syncBFTsystemState = 0;

    Network<syncBFTmessage, syncBFT_Peer> n;
    Network<PBFT_Message, PBFT_Peer> system;

    n.setMaxDelay(maxDelay);
    n.setToRandom();
    n.initNetwork(syncBFT_Peer::peerCount,maxDelay);

    int consensusSize = 0;

    vector<string> byzantinePeers;
    while(byzantinePeers.size()<(syncBFT_Peer::peerCount-1)/2) {
        int index = rand()%syncBFT_Peer::peerCount;
        if (std::find(byzantinePeers.begin(),byzantinePeers.end(),n[index]->id())!= byzantinePeers.end()){
        }else{
            n[index]->setByzantineFlag(true);
            byzantinePeers.push_back(n[index]->id());
        }
    }

    std::cerr<<"Byzantine Peers are ";
    for(auto &peer: byzantinePeers){
        std::cerr<<peer<<" ";
    }

    int lastConsensusAt = 0;

    //start with the first peer as leader

    bool shuffleByzantinePeers = false;
    int shuffleByzantineInterval = 20;
    for(int i =1; i<100; i++){

        if(i%shuffleByzantineInterval == 0){
            std::cerr<<"Iteration "<<i<<std::endl;
            shuffleByzantinePeers = true;
        }

        n.receive();
        n.preformComputation();
        n.transmit();

        if(syncBFT_Peer::syncBFTsystemState == 3){
            if(shuffleByzantinePeers){
                n.shuffleByzantines ((syncBFT_Peer::peerCount-1)/2);
                shuffleByzantinePeers = false;
            }
            //check if all peers terminated
            bool consensus = true;
            for(int peerId = 0; peerId<n.size(); peerId++ ) {
                if (!n[peerId]->getTerminationFlag()) {
                    //the leader does not need to terminate
                    if (n[peerId]->id() == syncBFT_Peer::getLeaderId()) {
                        continue;
                    }
                    consensus = false;
                    break;
                }
            }
            if (consensus){
                consensusSize++;
                std::cerr<<"++++++++++++++++++++++++++++++++++++++++++Consensus reached at iteration "<<i<<std::endl;
                //refresh the peers
                for(int i = 0;i<n.size();i++){
                    n[i]->refreshSyncBFT();

                }
                //reset system-wide sync state
                syncBFT_Peer::syncBFTsystemState = 0;

                lastConsensusAt = i;
                continue;
            }
        }

        if((i-lastConsensusAt)%n.maxDelay() ==0){
            //reset sync status of the peers after a notify step is done.
            if(syncBFT_Peer::syncBFTsystemState==3){
                for(int i = 0; i<n.size();i++){
                    n[i]->setSyncBFTState(0);
                    n[i]->iter++;
                }
            }
            syncBFT_Peer::incrementSyncBFTsystemState();
        }
    }

    std::cerr<<"Shuffled byzantine peers every "<<shuffleByzantineInterval<<"\tMax delay: "<<maxDelay<<"\t Consensus count: "<<n[0]->getBlockchain()->getChainSize()-1<<std::endl;

}

void bitcoin(std::ofstream &out, int avgDelay){
    Network<bCoinMessage, bCoin_Peer> n;
    n.setToPoisson();
    n.setLog(std::cout);
    n.initNetwork(10,avgDelay);

    //mining delays at the beginning
    for(int i = 0; i<n.size(); i++){
        n[i]->setMineNextAt( bCoin_Peer::distribution(bCoin_Peer::generator) );
    }

    for(int i = 1; i<100; i++){
        std::cerr<<"Iteration "<<i<<std::endl;
        n.receive();
        n.preformComputation();
        n.transmit();
    }
    int maxChain = 0;
    for(int i =0;i<n.size();i++){
        if(n[i]->getBlockchain()->getChainSize()>maxChain)
            maxChain = i;
    }
    std::cerr<<"Number of confirmations = "<<n[maxChain]->getBlockchain()->getChainSize()<<std::endl;
}

void Example(){
    Network<ExampleMessage,ExamplePeer> n;
    n.initNetwork(2,1);

    for(int i =0; i < 3; i++){
        std::cout<< "-- STARTING ROUND "<< i<< " --"<<  std::endl;

        n.receive();
        n.preformComputation();
        n.transmit();

        std::cout<< "-- ENDING ROUND "<< i<< " --"<<  std::endl;
    }
    
    ExamplePeer A("A");
    ExamplePeer B("B");

    A.addNeighbor(B, 10);
    B.addNeighbor(A, 5);
    
    std::cout << A<< std::endl;
    std::cout << B<< std::endl;
    
    std::cout<< n<< std::endl;
}

void bsg(std::ofstream &out,int){
    BlockGuardPeer_Sharded a("A");
    BlockGuardPeer_Sharded b("B");
    BlockGuardPeer_Sharded c("C");
    BlockGuardPeer_Sharded d("D");
    BlockGuardPeer_Sharded e("E");
    BlockGuardPeer_Sharded f("F");
    
    a.setFaultTolerance(0.5);
    b.setFaultTolerance(0.5);
    c.setFaultTolerance(0.5);
    d.setFaultTolerance(0.5);
    e.setFaultTolerance(0.5);
    f.setFaultTolerance(0.5);
    
    
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
    
    // group 1
    a.setGroup(1);
    b.setGroup(1);
    a.addGroupMember(b);
    b.addGroupMember(a);
    
    // group 2
    c.setGroup(2);
    d.setGroup(2);
    c.addGroupMember(d);
    d.addGroupMember(c);
    
    // group 3
    e.setGroup(3);
    f.setGroup(3);
    e.addGroupMember(f);
    f.addGroupMember(e);
    
    // form commmit one from group 1 and 3
    a.setCommittee(1);
    b.setCommittee(1);
    e.setCommittee(1);
    f.setCommittee(1);
    
    a.addcommitteeMember(b);
    a.addcommitteeMember(e);
    a.addcommitteeMember(f);
    
    b.addcommitteeMember(a);
    b.addcommitteeMember(e);
    b.addcommitteeMember(f);
    
    e.addcommitteeMember(b);
    e.addcommitteeMember(a);
    e.addcommitteeMember(f);
    
    f.addcommitteeMember(b);
    f.addcommitteeMember(e);
    f.addcommitteeMember(a);
    
    // for commit two from group 2
    c.setCommittee(2);
    d.setCommittee(2);
    
    c.addcommitteeMember(d);
    d.addcommitteeMember(c);
    
    for(int i = 0; i < 10; i++){
        a.printTo(out);
        b.printTo(out);
        c.printTo(out);
        d.printTo(out);
        e.printTo(out);
        f.printTo(out);
        
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
        
        if(i == 0){
            b.makeRequest();
        }
    }
    
    out<< "A:"<< a.getLedger().size();
    out<< "B:"<< b.getLedger().size();
    out<< "C:"<< c.getLedger().size();
    out<< "D:"<< d.getLedger().size();
    out<< "E:"<< e.getLedger().size();
    out<< "F:"<< f.getLedger().size();

}

//
// util functions
//

void buildInitialChain(std::vector<std::string> peerIds) {
    std::cerr << "Building initial chain" << std::endl;
    srand((time(nullptr)));
    Blockchain *preBuiltChain = new Blockchain(true);
    int index = 1;                  //blockchain index starts from 1;
    int blockCount = 1;
    std::string prevHash = "genesisHash";
    
    while (blockCount < blockChainLength) {
        std::set<std::string> publishers;
        
        std::string peerId = peerIds[rand()%peerIds.size()];
        std::string blockHash = std::to_string(index) + "_" + peerId;
        publishers.insert(peerId);
        preBuiltChain->createBlock(index, prevHash, blockHash, publishers);
        prevHash = blockHash;
        index++;
        blockCount++;
    }
    
    std::cerr << "Initial chain build complete." << std::endl;
    std::cerr << "Prebuilt chain: " << std::endl;
    std::cerr << *preBuiltChain << std::endl;
    std::cerr << preBuiltChain->getChainSize() << std::endl;
    blockchain = preBuiltChain;
    
    
}

std::set<std::string> getPeersForConsensus(int securityLevel) {
    int numOfPeers = peerCount / securityLevel;
    std::set<std::string> peersForConsensus;
    int chainSize = blockchain->getChainSize ();
    int i = blockchain->getChainSize() - 1;             //i keeps track of index in the chain
    while (peersForConsensus.size() < numOfPeers) {
        std::string peerId = *(blockchain->getBlockAt(i).getPublishers()).begin();
        peersForConsensus.insert(peerId);
        //random value
        int randVal = rand()%peerCount;
        
        int skip = randVal;
        if ((i - skip) <= 0) {
            i = chainSize - i - skip;
            
        } else
            i = i - skip;
    }
    
    return peersForConsensus;
}
