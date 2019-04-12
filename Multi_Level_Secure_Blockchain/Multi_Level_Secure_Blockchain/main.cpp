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
#include "PBFTPeer_Sharded.hpp"
#include "BGSReferenceCommittee.hpp"
#include <iostream>
#include <chrono>
#include <random>


const int peerCount = 10;
const int blockChainLength = 100;
Blockchain *blockchain;
int shuffleByzantineInterval = 0;
//std::ofstream progress;

// util functions
void buildInitialChain(std::vector<std::string>);
std::set<std::string> getPeersForConsensus(int);
int getNumberOfConfimedTransactionsBGS_PBFT(const std::vector<PBFTPeer_Sharded>&);
int sumMessagesSentBGS(const BGSReferenceCommittee&);

void Example();
void PBFT(std::ofstream &out,int);
void syncBFT(std::ofstream &,int );
void bitcoin(std::ofstream &,int );
void bsg(std::ofstream &out,int);

int main(int argc, const char * argv[]) {
    srand((float)time(NULL));
    
    std::string algorithm = argv[1];
    std::string filePath = argv[2];

    if(algorithm == "example"){
        Example();
    }
    else if (algorithm== "pbft"){
        for(int delay = 1; delay < 51; delay = delay + 10){
            std::cout<< "PBFT"<<std::endl;
            //std::cout<< "Delay:"+std::to_string(delay)<< std::endl;
            std::ofstream out;
            out.open(filePath + "/PBFT_Delay"+std::to_string(delay) + ".log");
            if ( out.fail() ){
                std::cerr << "Error: could not open file" << std::endl;
            }
            for(int run = 0; run < 10; run++){
                //td::cout<< "run:"<<run<<std::endl;
                PBFT(out,delay);
            }
            out.close();
            if(delay == 1){
                delay = 0;
            }
        }

    }else if (algorithm == "syncBFT") {
        std::ofstream out;
        for(int delay = 1;delay<2; delay+=10){
            std::cout<< "Start with Delay "+std::to_string(delay)<< std::endl;
            for(int run = 0; run < 2; run++){
                syncBFT(out, delay);
            }

        }
    }else if (algorithm == "bgs") {
        std::cout<< "BGS"<<std::endl;
        std::ofstream out;
        for(int delay = 1; delay < 50; delay = delay + 10){
            std::ofstream out;
            out.open(filePath + "/BGS_Delay"+std::to_string(delay) + ".log");
            if ( out.fail() ){
                std::cerr << "Error: could not open file" << std::endl;
            }
            //progress.open(filePath + "/progress.txt");
            //progress<< "Delay:"+std::to_string(delay)<< std::endl;
            for(int run = 0; run < 10; run++){
                //progress<< "run:"<<run<<std::endl;
                bsg(out,delay);
            }
            out.close();
            if(delay == 1){
                delay = 0;
            }
        }
        out.close();
    }else if (algorithm == "bitcoin") {
        std::ofstream out;
        bitcoin(out, 1);
    }
    
    return 0;
}

void PBFT(std::ofstream &out,int delay){
    Network<PBFT_Message, PBFT_Peer> system;
    system.setLog(out);
    system.setToRandom();
    system.setMaxDelay(delay);
    system.initNetwork(512);
    for(int i = 0; i < system.size(); i++){
        system[i]->setFaultTolerance(0.3);
        system[i]->init();
    }
    
    int numberOfRequests = 0;
    for(int i =-1; i < 1000; i++){
//        if(i%100 == 0 && i != 0){
//            progress<< std::endl;
//        }
//        progress<< "."<< std::flush;
        
        int randIndex = rand()%system.size();
        while(system[randIndex]->isPrimary()){
            randIndex = rand()%system.size();
        }
        system.makeRequest(randIndex);
        numberOfRequests++;
        
        system.receive();
        system.preformComputation();
        system.transmit();
        system.log();
    }

    int min = (int)system[0]->getLedger().size();
    int max = (int)system[0]->getLedger().size();
    int totalMessages = system[0]->getMessageCount();
    for(int i = 0; i < system.size(); i++){
        //out<< "Peer ID:"<< system[i].id() << " Ledger Size:"<< system[i].getLedger().size()<< std::endl;
        if(system[i]->getLedger().size() < min){
            min = (int)system[i]->getLedger().size();
        }
        if(system[i]->getLedger().size() > max){
            max = (int)system[i]->getLedger().size();
        }
        totalMessages += system[i]->getMessageCount();
    }
    out<< "Min Ledger:,"<< min<< std::endl;
    out<< "Max Ledger:,"<< max<< std::endl;
    out<< "Total Messages:,"<< totalMessages<< std::endl;
    out<< "Total Request:,"<< numberOfRequests<<std::endl;
    //std::cout<< std::endl;
}

void syncBFT(std::ofstream &out,int maxDelay){
    srand(time(nullptr));
    int shuffleByzantineInterval = 50;
    int shuffleByzantineCount = 1;
    int numberOfPeers = 10;

    Network<syncBFTmessage, syncBFT_Peer> n;

    n.setMaxDelay(maxDelay);
    n.setToRandom();
    n.initNetwork(numberOfPeers);

    int consensusSize = 0;

    vector<string> byzantinePeers;
    while(byzantinePeers.size()<(numberOfPeers-1)/2) {
        int index = rand()%numberOfPeers;
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

    for(int i =1; i<1000; i++){

        if(i%shuffleByzantineInterval == 0){
            std::cerr<<"Iteration "<<i<<std::endl;
            shuffleByzantinePeers = true;
        }

        if( i%5 == 0 ){
            n.makeRequest(rand()%n.size());
        }
        n.receive();


        n.preformComputation();

        int leadeIndex = 0;
        for(int i = 0; i < n.size(); i++){
            if (n[i]->isLeader()){
                leadeIndex = i;
            }
        }
        //syncBFT_Peer::syncBFTsystemState
        if(n[leadeIndex]->getStatus() == 3){
            if(shuffleByzantinePeers){
                n.shuffleByzantines (shuffleByzantineCount);
                shuffleByzantinePeers = false;
            }
        }
        //check if all peers terminated
        bool consensus = true;
        for(int peerId = 0; peerId<n.size(); peerId++ ) {
            if (!n[peerId]->getTerminationFlag()) {
                //the leader does not need to terminate
                if (n[peerId]->id() == n[peerId]->getLeaderId()) {
                    continue;
                }
                consensus = false;
                break;
            }
        }
        if (consensus){
            consensusSize++;
            out<<"++++++++++++++++++++++++++++++++++++++++++Consensus reached at iteration "<<i<<std::endl;
            //refresh the peers
            for(int i = 0;i<n.size();i++){
                n[i]->refreshSyncBFT();

            }
            //reset system-wide sync state
            for(int i = 0; i<n.size();i++){
                n[i]->setSyncBFTState(0);
            }

            lastConsensusAt = i;
            continue;
        }

        if((i-lastConsensusAt)%n.maxDelay() ==0){
            // find leader
            int leadeIndex = 0;
            for(int i = 0; i < n.size(); i++){
                if (n[i]->isLeader()){
                    leadeIndex = i;
                }
            }

            //reset sync status of the peers after a notify step is done.
            if(n[leadeIndex]->getStatus() == 3){
                for(int i = 0; i<n.size();i++){
                    n[i]->setSyncBFTState(0);
                    n[i]->iter++;
                    
                }
            }
            for(int i = 0; i<n.size();i++){
                n[i]->incrementSyncBFTsystemState();
            }
            n.transmit();
        }

    }
    out<<"Shuffled byzantine peers every "<<shuffleByzantineInterval<<"\tMax delay: "<<maxDelay<<"\t Consensus count: "<<n[0]->getBlockchain()->getChainSize()-1<<std::endl;
}

void bitcoin(std::ofstream &out, int avgDelay){
    Network<bCoinMessage, bCoin_Peer> n;
    n.setToPoisson();
    n.setAvgDelay(avgDelay);
    n.setLog(std::cout);
    n.initNetwork(10);

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
    n.setMaxDelay(1);
    n.initNetwork(2);

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

void bsg(std::ofstream &out,int delay){
    BGSReferenceCommittee system = BGSReferenceCommittee();
    system.setGroupSize(16);
    system.setToRandom();
    system.setMaxDelay(delay);
    system.setLog(out);
    system.initNetwork(64);
    system.setFaultTolerance(0.3);

    int numberOfRequests = 0;
    for(int i =-1; i < 1000; i++){
        system.makeRequest();
        numberOfRequests++;
        system.receive();
        system.preformComputation();
        system.transmit();
        system.log();
    }

    int max = getNumberOfConfimedTransactionsBGS_PBFT(system.getPeers());
    int totalMessages = sumMessagesSentBGS(system);
    
    out<< "Max Ledger:,"<< max<< std::endl;
    out<< "Total Messages:,"<< totalMessages<< std::endl;
    out<< "Total Request:,"<< numberOfRequests<<std::endl;
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

bool match(std::vector<PBFT_Message> ledger, PBFT_Message aMessage){
    for(int i = 0; i < ledger.size(); i++){
        if(ledger[i] == aMessage){
            return true;
        }
    }
    return false;
}

int getNumberOfConfimedTransactionsBGS_PBFT(const std::vector<PBFTPeer_Sharded> &peers){
    std::vector<PBFT_Message> globalLegder = peers[0].getLedger();
    for(int i = 0; i < peers.size(); i++){
        std::vector<PBFT_Message> localLegder = peers[i].getLedger();
        for(int j = 0; j < localLegder.size(); j++){
            if(!match(globalLegder,localLegder[j])){
                globalLegder.push_back(localLegder[j]);
            }
        }
    }
    
    return globalLegder.size();
}

int sumMessagesSentBGS(const BGSReferenceCommittee &system){
    int sum = 0;
    for(int i = 0; i < system.size(); i++){
        sum += system[i]->getMessageCount();
    }
    return sum;
}
