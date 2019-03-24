//
//  main.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <iostream>
#include "ExamplePeer.hpp"
#include "PBFT_Peer.hpp"
#include "Network.hpp"

void Example();
void PBFT();

int main(int argc, const char * argv[]) {
    srand((float)time(NULL));
    
    Network<ExampleMessage,ExamplePeer> n(50,1);
   
    for(int i =0; i < 100; i++){
        std::cout<< "-- STARTING ROUND "<< i<< " --"<<  std::endl;
        
        n.receive();
        n.preformComputation();
        n.transmit();

        std::cout<< "-- ENDING ROUND "<< i<< " --"<<  std::endl;
    }
}

void PBFT(){
    
    PBFT_Peer A("A",0.3);
    PBFT_Peer B("B",0.3);
    PBFT_Peer C("C",0.3);
    PBFT_Peer D("D",0.3);
    PBFT_Peer E("E",0.3);
    PBFT_Peer F("F",0.3);
    PBFT_Peer G("G",0.3);
    PBFT_Peer H("H",0.3);
    PBFT_Peer I("I",0.3);
    PBFT_Peer J("J",0.3);
    
    A.addNeighbor(B, 1);
    A.addNeighbor(C, 1);
    A.addNeighbor(D, 1);
    A.addNeighbor(E, 1);
    A.addNeighbor(F, 1);
    A.addNeighbor(G, 1);
    A.addNeighbor(H, 1);
    A.addNeighbor(I, 1);
    A.addNeighbor(J, 1);
    
    B.addNeighbor(A, 1);
    B.addNeighbor(C, 1);
    B.addNeighbor(D, 1);
    B.addNeighbor(E, 1);
    B.addNeighbor(F, 1);
    B.addNeighbor(G, 1);
    B.addNeighbor(H, 1);
    B.addNeighbor(I, 1);
    B.addNeighbor(J, 1);
    
    C.addNeighbor(B, 1);
    C.addNeighbor(A, 1);
    C.addNeighbor(D, 1);
    C.addNeighbor(E, 1);
    C.addNeighbor(F, 1);
    C.addNeighbor(G, 1);
    C.addNeighbor(H, 1);
    C.addNeighbor(I, 1);
    C.addNeighbor(J, 1);
    
    D.addNeighbor(B, 1);
    D.addNeighbor(C, 1);
    D.addNeighbor(A, 1);
    D.addNeighbor(E, 1);
    D.addNeighbor(F, 1);
    D.addNeighbor(G, 1);
    D.addNeighbor(H, 1);
    D.addNeighbor(I, 1);
    D.addNeighbor(J, 1);
    
    E.addNeighbor(B, 1);
    E.addNeighbor(C, 1);
    E.addNeighbor(D, 1);
    E.addNeighbor(A, 1);
    E.addNeighbor(F, 1);
    E.addNeighbor(G, 1);
    E.addNeighbor(H, 1);
    E.addNeighbor(I, 1);
    E.addNeighbor(J, 1);
    
    F.addNeighbor(B, 1);
    F.addNeighbor(C, 1);
    F.addNeighbor(D, 1);
    F.addNeighbor(E, 1);
    F.addNeighbor(A, 1);
    F.addNeighbor(G, 1);
    F.addNeighbor(H, 1);
    F.addNeighbor(I, 1);
    F.addNeighbor(J, 1);
    
    G.addNeighbor(B, 1);
    G.addNeighbor(C, 1);
    G.addNeighbor(D, 1);
    G.addNeighbor(E, 1);
    G.addNeighbor(F, 1);
    G.addNeighbor(A, 1);
    G.addNeighbor(H, 1);
    G.addNeighbor(I, 1);
    G.addNeighbor(J, 1);
    
    H.addNeighbor(B, 1);
    H.addNeighbor(C, 1);
    H.addNeighbor(D, 1);
    H.addNeighbor(E, 1);
    H.addNeighbor(F, 1);
    H.addNeighbor(G, 1);
    H.addNeighbor(A, 1);
    H.addNeighbor(I, 1);
    H.addNeighbor(J, 1);
    
    I.addNeighbor(B, 1);
    I.addNeighbor(C, 1);
    I.addNeighbor(D, 1);
    I.addNeighbor(E, 1);
    I.addNeighbor(F, 1);
    I.addNeighbor(G, 1);
    I.addNeighbor(H, 1);
    I.addNeighbor(A, 1);
    I.addNeighbor(J, 1);
    
    J.addNeighbor(B, 1);
    J.addNeighbor(C, 1);
    J.addNeighbor(D, 1);
    J.addNeighbor(E, 1);
    J.addNeighbor(F, 1);
    J.addNeighbor(G, 1);
    J.addNeighbor(H, 1);
    J.addNeighbor(I, 1);
    J.addNeighbor(A, 1);
    
    for(int i =0; i < 100; i++){
        std::cout<< "-- STARTING ROUND "<< i<< " --"<<  std::endl;
        
        A.receive();
        B.receive();
        C.receive();
        D.receive();
        E.receive();
        F.receive();
        G.receive();
        H.receive();
        I.receive();
        J.receive();

        
        A.preformComputation(5);
        B.preformComputation(5);
        C.preformComputation(5);
        D.preformComputation(5);
        E.preformComputation(5);
        F.preformComputation(5);
        G.preformComputation(5);
        H.preformComputation(5);
        I.preformComputation(5);
        J.preformComputation(5);

        
        A.transmit();
        B.transmit();
        C.transmit();
        D.transmit();
        E.transmit();
        F.transmit();
        G.transmit();
        H.transmit();
        I.transmit();
        J.transmit();
        
        std::cout<< "-- ENDING ROUND "<< i<< " --"<<  std::endl;
    }
    std::cout<< "HERE"<<std::endl;;
}
