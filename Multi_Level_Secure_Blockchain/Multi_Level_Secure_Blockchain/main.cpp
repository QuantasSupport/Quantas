//
//  main.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "Peer.hpp"
#include "ExamplePeer.hpp"
#include "PBFT_Peer.hpp"
#include "Network.hpp"

void Example();
void PBFT(std::ofstream &out,int);


int main(int argc, const char * argv[]) {
    srand((float)time(NULL));
    
    std::string algorithm = argv[1];
    std::string filePath = argv[2];
    
    if(algorithm == "example"){
        Example();
    }
    else if (algorithm== "pbft"){
        for(int delay = 1; delay < 101; delay = delay + 10){
            std::cout<< "Delay:"+std::to_string(delay)<< std::endl;
            std::ofstream out;
            out.open(filePath + "/PBFT_Delay:"+std::to_string(delay) + ".log");
            for(int run = 0; run < 5; run++){
                PBFT(out,delay);
            }
            out.close();
        }
    }
    
    return 0;
}

void PBFT(std::ofstream &out,int avgDelay){
    
    Network<PBFT_Message, PBFT_Peer> system;
    system.setToPoisson();
    system.initNetwork(100,avgDelay);
    for(int i = 0; i < system.size(); i++){
        system[i]->setFaultTolerance(0.3);
        system[i]->setLogFile(out);
        system[i]->init();
    }
    
    int numberOfRequests = 0;
    for(int i =-1; i < 1000; i++){
        if(i%100 == 0){
            std::cout<< std::endl;
        }
        std::cout<< "."<< std::flush;
        
        if(i%5 == 0){
            int randIndex = rand()%system.size();
            while(system[randIndex]->isPrimary()){
                randIndex = rand()%system.size();
            }
            system.makeRequest(randIndex);
            numberOfRequests++;
        }
        
//        for(int i = 0; i < system.size(); i++){
//            system[i]->log();
//        }
        system.receive();
//        for(int i = 0; i < system.size(); i++){
//            system[i]->log();
//        }
        system.preformComputation();
       
        system.transmit();
//        for(int i = 0; i < system.size(); i++){
//            system[i]->log();
//        }
        
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
}
