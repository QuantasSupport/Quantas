//
//  test.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 4/15/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

// run tests over classes

#include "ByzantineNetwork_Test.hpp"
#include "NetworkTests.hpp"
#include "Peer_Test.hpp"

#include <string>

int main(int argc, const char * argv[]){
    if(argc < 2){
        std::cerr << "Error: need test to run and output path" << std::endl;
    }

    std::string testOption = argv[1];
    std::string filePath = argv[2];

    if(testOption == "all"){
        RunByzantineNetworkTest(filePath);
        runNetworkTests(filePath);
        RunPeerTests(filePath);
    }else if(testOption == "b_network"){
        RunByzantineNetworkTest(filePath);
    }else if(testOption == "network"){
        runNetworkTests(filePath);
    }else if(testOption == "peer"){
        RunPeerTests(filePath);
    }

    return 0;
}


