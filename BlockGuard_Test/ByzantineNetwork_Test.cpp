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
#include "../BlockGuard/ExamplePeer.hpp"

static const int PEERS = 1000; 

void RunByzantineNetworkTest(std::string filepath){
    std::ofstream log;
    log.open(filepath + "/ByzantineNetwork.log");
    if (log.fail() ){
        std::cerr << "Error: could not open file at: "<< filepath << std::endl;
    }
}

void testMakeByzantine(std::ostream &log){
    ByzantineNetwork<ExamplePeer,ExampleMessage> bNetwork = ByzantineNetwork<ExamplePeer,ExampleMessage>();

    bNetwork.initNetwork(PEERS);

    // test that making peers Byzantine only effects the number of peers passed
    bNetwork.makeByzantines(PEERS*0.1);// 10 percent
    assert(bNetwork.getByzantine().size()   == PEERS*0.1);
    assert(bNetwork.getCorrect().size()     == PEERS*0.9);

    bNetwork.makeByzantines(PEERS*0.1);// another 10 percent
    assert(bNetwork.getByzantine().size()   == PEERS*0.2);
    assert(bNetwork.getCorrect().size()     == PEERS*0.8);

    bNetwork.makeByzantines(PEERS*0.2);// add 20 percent
    assert(bNetwork.getByzantine().size()   == PEERS*0.4);
    assert(bNetwork.getCorrect().size()     == PEERS*0.6);

    bNetwork.makeByzantines(PEERS*0.6);//the rest of the network
    assert(bNetwork.getByzantine().size()   == PEERS);
    assert(bNetwork.getCorrect().size()     == 0);

    // test that making peers Correct only effects the number of peers passed
    bNetwork = ByzantineNetwork<ExamplePeer,ExampleMessage>();
    bNetwork.makeByzantines(PEERS);

    bNetwork.makeCorrect(PEERS*0.1);// 10 percent
    assert(bNetwork.getCorrect().size()     == PEERS*0.1);
    assert(bNetwork.getByzantine().size()   == PEERS*0.9);

    bNetwork.makeCorrect(PEERS*0.1);// another 10 percent
    assert(bNetwork.getCorrect().size()     == PEERS*0.2);
    assert(bNetwork.getByzantine().size()   == PEERS*0.8);

    bNetwork.makeCorrect(PEERS*0.2);// add 20 percent
    assert(bNetwork.getCorrect().size()     == PEERS*0.4);
    assert(bNetwork.getByzantine().size()   == PEERS*0.6);

    bNetwork.makeCorrect(PEERS*0.6);//the rest of the network
    assert(bNetwork.getCorrect().size()     == PEERS);
    assert(bNetwork.getByzantine().size()   == 0);

}
void testByzantineShuffle(std::ostream &log){

     ByzantineNetwork<ExamplePeer,ExampleMessage> bNetwork = ByzantineNetwork<ExamplePeer,ExampleMessage>();

    bNetwork.initNetwork(PEERS);

    // test that peers to not change state when shuffled and only X number of peers are shuffled
    bNetwork.makeByzantines(PEERS*0.3);// 10 percent
    assert(bNetwork.getByzantine().size()   == PEERS*0.3);
    assert(bNetwork.getCorrect().size()     == PEERS*0.7);

    // set state to somthing other then the init values
    for(int i = 0; i < bNetwork.size(); i++){
        bNetwork[i]->setCounter(50);
    }

}