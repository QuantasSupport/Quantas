//
//  main.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <iostream>
#include "ExamplePeer.hpp"


int main(int argc, const char * argv[]) {
    
    srand((float)time(NULL));
    
    ExamplePeer a("A");
    ExamplePeer b("B");
    ExamplePeer c("C");
    
    // edge between A and B wight 5
    a.addNeighbor(b,5);
    b.addNeighbor(a,5);
    
    // edge between A and C wight 1
    a.addNeighbor(c,1);
    c.addNeighbor(a,1);
    
    for(int i =0; i < 25; i++){
        std::cout<< "-- STARTING ROUND "<< i<< " --"<<  std::endl;
        
        a.receive();
        b.receive();
        c.receive();
        
        a.preformComputation();
        b.preformComputation();
        c.preformComputation();
        
        a.transmit();
        b.transmit();
        c.transmit();
        
        std::cout<< "-- ENDING ROUND "<< i<< " --"<<  std::endl;
    }
    return 0;
}
