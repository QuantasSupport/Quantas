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
        for(int delay = 1; delay < 2; delay = delay + 10){
            std::cout<< "Start with Delay "+std::to_string(delay)<< std::endl;
            std::ofstream out;
            out.open(filePath + "/PBFT_Delay:"+std::to_string(delay) + ".log");
            for(int run = 0; run < 1; run++){
                PBFT(out,delay);
            }
            out.close();
            std::cout<< "End with Delay "+std::to_string(delay)<< std::endl;
        }
    }
    
    return 0;
}

void PBFT(std::ofstream &out,int avgDelay){
    
    Network<PBFT_Message, PBFT_Peer> system;
    system.setToPoisson();
    system.initNetwork(10,avgDelay);
    for(int i = 0; i < system.size(); i++){
        system[i]->setFaultTolerance(0.3);
        system[i]->setLogFile(out);
    }
    
    //std::cout<< system<< std::endl;
    int numberOfRequests = 0;
    for(int i =-1; i < 1000; i++){
        std::cout<< "."<< std::flush;
       // out<< "-- STARTING ROUND "<< i<< " --"<<  std::endl;

        if(i%10 == 0){
            int randIndex = rand()%system.size();
            system.makeRequest(randIndex);
            numberOfRequests++;
        }

        system.receive();
        for(int i = 0; i < system.size(); i++){
            system[i]->log();
        }
        system.preformComputation();
        for(int i = 0; i < system.size(); i++){
            system[i]->log();
        }
        system.transmit();
        for(int i = 0; i < system.size(); i++){
            system[i]->log();
        }

        //out<< "-- ENDING ROUND "<< i<< " --"<<  std::endl;
    }
    std::cout<< std::endl;

    int min = system[0]->getLedger().size();
    int max = system[0]->getLedger().size();
    for(int i = 0; i < system.size(); i++){
        //out<< "Peer ID:"<< system[i].id() << " Ledger Size:"<< system[i].getLedger().size()<< std::endl;
        if(system[i]->getLedger().size() < min){
            min = system[i]->getLedger().size();
        }
        if(system[i]->getLedger().size() > max){
            max = system[i]->getLedger().size();
        }
    }
    out<< "Min Ledger:"<< min<< std::endl;
    out<< "Max Ledger:"<< max<< std::endl;
    out<< "Total Request:"<< numberOfRequests<<std::endl;
}

void Example(){
    Network<ExampleMessage,ExamplePeer> n;
    n.initNetwork(10,1);

    for(int i =0; i < 1; i++){
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
