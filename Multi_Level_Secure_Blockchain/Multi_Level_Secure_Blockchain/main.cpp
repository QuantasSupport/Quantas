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
    
    ExamplePeer a("A");
    ExamplePeer b("B");
    
    a.addGroupMembers(b);
    b.addGroupMembers(a);
    
    for(int i =0; i < 25; i++){
        std::cout<< "-- STARTING ROUND "<< i<< " --"<<  std::endl;
        
        a.receive();
        b.receive();
        
        a.preformComputation();
        b.preformComputation();
        
        a.transmit();
        b.transmit();
        
        std::cout<< "-- ENDING ROUND "<< i<< " --"<<  std::endl;
    }
    
    return 0;
}
