//
//  ByzantineNetwork_Test.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 5/23/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "ByzantineNetwork.hpp"

void RunByzantineNetworkTest(std::string filepath){
    std::ofstream log;
    log.open(filepath + "/ByzantineNetwork.log");
    if (log.fail() ){
        std::cerr << "Error: could not open file at: "<< filepath << std::endl;
    }
}

void testMakeByzantine(std::ostream &log){

}
void testByzantineShuffle(std::ostream &log){

}