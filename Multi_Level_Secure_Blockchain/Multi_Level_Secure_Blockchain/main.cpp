//
//  main.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "ExamplePeer.hpp"
#include "PBFT_Peer.hpp"
#include "Network.hpp"

void Example();
void PBFT(std::ofstream &out,int);

int main(int argc, const char * argv[]) {
    srand((float)time(NULL));
    
    std::string algorithm = argv[1];
    
    if(algorithm == "example"){
        Example();
    }
    else if (algorithm== "pbft"){
        for(int delay = 1; delay < 101; delay = delay + 10){
            std::cout<< "Start with Delay "+std::to_string(delay)<< std::endl;
            std::ofstream out;
            out.open("/Users/kendrichood/Desktop/pbft/PBFT_Delay:"+std::to_string(delay) + ".csv");
            for(int run = 0; run < 5; run++){
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
    system.initNetwork(500,1);
    for(int i = 0; i < system.size(); i++){
        system[i]->setFaultTolerance(0.3);
    }
    
    
    for(int i =-1; i < 1000; i++){
        std::cout<< ".";
       // out<< "-- STARTING ROUND "<< i<< " --"<<  std::endl;
        
        if(i%5 == 0){
            int randIndex = rand()%system.size();
            system.makeRequest(randIndex);
        }
        
        system.receive();
        system.preformComputation();
        system.transmit();
        
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
    //out<< "Min Ledger:"<< min<< std::endl;
    out<< "Max Ledger:,"<< max<< std::endl;
}

void Example(){
    Network<ExampleMessage,ExamplePeer> n;
    n.initNetwork(100,1);
    
    for(int i =0; i < 100; i++){
        std::cout<< "-- STARTING ROUND "<< i<< " --"<<  std::endl;
        
        n.receive();
        n.preformComputation();
        n.transmit();
        
        std::cout<< "-- ENDING ROUND "<< i<< " --"<<  std::endl;
    }
}
