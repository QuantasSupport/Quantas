//
//  main.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <iostream>
#include "ExamplePeer.hpp"
#include "Network.hpp"


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
    
    return 0;
}
