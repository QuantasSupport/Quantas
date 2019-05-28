//
//  ByzantineNetwork_Test.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 5/23/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "ByzantineNetwork_Test.hpp"

static const int PEERS = 10; 

void RunByzantineNetworkTest(std::string filepath){
    std::ofstream log;
    log.open(filepath + "/ByzantineNetwork.log");
    if (log.fail() ){
        std::cerr << "Error: could not open file at: "<< filepath << std::endl;
    }
    testMakeByzantine(log);
    testByzantineShuffle(log);
}

void testMakeByzantine(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testMakeByzantine"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;

    ByzantineNetwork<ExampleMessage, ExamplePeer> bNetwork = ByzantineNetwork<ExampleMessage, ExamplePeer>();
    bNetwork.initNetwork(PEERS);

    // set state to somthing other then the init values
    for(int i = 0; i < bNetwork.size(); i++){
        bNetwork[i]->setCounter(50);
    }

    // test that making peers Byzantine only effects the number of peers passed
    bNetwork.makeByzantines(PEERS*0.1);// 10 percent
    assert(bNetwork.getByzantine().size()   == PEERS*0.1);
    assert(bNetwork.getCorrect().size()     == PEERS*0.9);
    for(auto peer = bNetwork.getByzantine().begin(); peer != bNetwork.getByzantine().end(); peer++){
        assert((*peer)->isByzantine()   == true);
        assert((*peer)->getCounter()    == 50);
    }
    for(auto peer = bNetwork.getCorrect().begin(); peer != bNetwork.getCorrect().end(); peer++){
        assert((*peer)->isByzantine()   == false);
        assert((*peer)->getCounter()    == 50);
    }

    bNetwork.makeByzantines(PEERS*0.1);// another 10 percent
    assert(bNetwork.getByzantine().size()   == PEERS*0.2);
    assert(bNetwork.getCorrect().size()     == PEERS*0.8);
    for(auto peer = bNetwork.getByzantine().begin(); peer != bNetwork.getByzantine().end(); peer++){
        assert((*peer)->isByzantine()   == true);
        assert((*peer)->getCounter()    == 50);
    }
    for(auto peer = bNetwork.getCorrect().begin(); peer != bNetwork.getCorrect().end(); peer++){
        assert((*peer)->isByzantine()   == false);
        assert((*peer)->getCounter()    == 50);
    }

    bNetwork.makeByzantines(PEERS*0.2);// add 20 percent
    assert(bNetwork.getByzantine().size()   == PEERS*0.4);
    assert(bNetwork.getCorrect().size()     == PEERS*0.6);
    for(auto peer = bNetwork.getByzantine().begin(); peer != bNetwork.getByzantine().end(); peer++){
        assert((*peer)->isByzantine()   == true);
        assert((*peer)->getCounter()    == 50);
    }
    for(auto peer = bNetwork.getCorrect().begin(); peer != bNetwork.getCorrect().end(); peer++){
        assert((*peer)->isByzantine()   == false);
        assert((*peer)->getCounter()    == 50);
    }

    bNetwork.makeByzantines(PEERS*0.6);//the rest of the network
    assert(bNetwork.getByzantine().size()   == PEERS);
    assert(bNetwork.getCorrect().size()     == 0);
    for(auto peer = bNetwork.getByzantine().begin(); peer != bNetwork.getByzantine().end(); peer++){
        assert((*peer)->isByzantine()   == true);
        assert((*peer)->getCounter()    == 50);
    }
    for(auto peer = bNetwork.getCorrect().begin(); peer != bNetwork.getCorrect().end(); peer++){
        assert((*peer)->isByzantine()   == false);
        assert((*peer)->getCounter()    == 50);
    }

    // test that making peers Correct only effects the number of peers passed
    bNetwork = ByzantineNetwork<ExampleMessage, ExamplePeer>();
    bNetwork.initNetwork(PEERS);
    bNetwork.makeByzantines(PEERS);
    // set state to somthing other then the init values
    for(int i = 0; i < bNetwork.size(); i++){
        bNetwork[i]->setCounter(50);
    }

    bNetwork.makeCorrect(PEERS*0.1);// 10 percent
    assert(bNetwork.getCorrect().size()     == PEERS*0.1);
    assert(bNetwork.getByzantine().size()   == PEERS*0.9);
    for(auto peer = bNetwork.getByzantine().begin(); peer != bNetwork.getByzantine().end(); peer++){
        assert((*peer)->isByzantine()   == true);
        assert((*peer)->getCounter()    == 50);
    }
    for(auto peer = bNetwork.getCorrect().begin(); peer != bNetwork.getCorrect().end(); peer++){
        assert((*peer)->isByzantine()   == false);
        assert((*peer)->getCounter()    == 50);
    }

    bNetwork.makeCorrect(PEERS*0.1);// another 10 percent
    assert(bNetwork.getCorrect().size()     == PEERS*0.2);
    assert(bNetwork.getByzantine().size()   == PEERS*0.8);
    for(auto peer = bNetwork.getByzantine().begin(); peer != bNetwork.getByzantine().end(); peer++){
        assert((*peer)->isByzantine()   == true);
        assert((*peer)->getCounter()    == 50);
    }
    for(auto peer = bNetwork.getCorrect().begin(); peer != bNetwork.getCorrect().end(); peer++){
        assert((*peer)->isByzantine()   == false);
        assert((*peer)->getCounter()    == 50);
    }

    bNetwork.makeCorrect(PEERS*0.2);// add 20 percent
    assert(bNetwork.getCorrect().size()     == PEERS*0.4);
    assert(bNetwork.getByzantine().size()   == PEERS*0.6);
    for(auto peer = bNetwork.getByzantine().begin(); peer != bNetwork.getByzantine().end(); peer++){
        assert((*peer)->isByzantine()   == true);
        assert((*peer)->getCounter()    == 50);
    }
    for(auto peer = bNetwork.getCorrect().begin(); peer != bNetwork.getCorrect().end(); peer++){
        assert((*peer)->isByzantine()   == false);
        assert((*peer)->getCounter()    == 50);
    }

    bNetwork.makeCorrect(PEERS*0.6);//the rest of the network
    assert(bNetwork.getCorrect().size()     == PEERS);
    assert(bNetwork.getByzantine().size()   == 0);
    for(auto peer = bNetwork.getByzantine().begin(); peer != bNetwork.getByzantine().end(); peer++){
        assert((*peer)->isByzantine()   == true);
        assert((*peer)->getCounter()    == 50);
    }
    for(auto peer = bNetwork.getCorrect().begin(); peer != bNetwork.getCorrect().end(); peer++){
        assert((*peer)->isByzantine()   == false);
        assert((*peer)->getCounter()    == 50);
    }

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testMakeByzantine correct"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void testByzantineShuffle(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testByzantineShuffle"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;

    ByzantineNetwork<ExampleMessage, ExamplePeer> bNetwork = ByzantineNetwork<ExampleMessage, ExamplePeer>();
    bNetwork.initNetwork(PEERS);
    // set state to somthing other then the init values
    for(int i = 0; i < bNetwork.size(); i++){
        bNetwork[i]->setCounter(50);
    }

    // test that peers to not change state when shuffled and only X number of peers are shuffled
    bNetwork.makeByzantines(PEERS*0.3);// 10 percent
    assert(bNetwork.getByzantine().size()   == PEERS*0.3);
    assert(bNetwork.getCorrect().size()     == PEERS*0.7);

    for(auto peer = bNetwork.getByzantine().begin(); peer != bNetwork.getByzantine().end(); peer++){
        assert((*peer)->isByzantine()   == true);
        assert((*peer)->getCounter()    == 50);
    }
    for(auto peer = bNetwork.getCorrect().begin(); peer != bNetwork.getCorrect().end(); peer++){
        assert((*peer)->isByzantine()   == false);
        assert((*peer)->getCounter()    == 50);
    }

    bNetwork.shuffleByzantines(PEERS*0.3); // shuffle all Byzantine peers
    assert(bNetwork.getByzantine().size()   == PEERS*0.3);
    assert(bNetwork.getCorrect().size()     == PEERS*0.7);

    for(auto peer = bNetwork.getByzantine().begin(); peer != bNetwork.getByzantine().end(); peer++){
        assert((*peer)->isByzantine()   == true);
        assert((*peer)->getCounter()    == 50);
    }
    for(auto peer = bNetwork.getCorrect().begin(); peer != bNetwork.getCorrect().end(); peer++){
        assert((*peer)->isByzantine()   == false);
        assert((*peer)->getCounter()    == 50);
    }

    bNetwork.shuffleByzantines(PEERS*0.10); // shuffle some Byzantine peers
    assert(bNetwork.getByzantine().size()   == PEERS*0.3);
    assert(bNetwork.getCorrect().size()     == PEERS*0.7);

    for(auto peer = bNetwork.getByzantine().begin(); peer != bNetwork.getByzantine().end(); peer++){
        assert((*peer)->isByzantine()   == true);
        assert((*peer)->getCounter()    == 50);
    }
    for(auto peer = bNetwork.getCorrect().begin(); peer != bNetwork.getCorrect().end(); peer++){
        assert((*peer)->isByzantine()   == false);
        assert((*peer)->getCounter()    == 50);
    }
    // test to make sure Byzantine peers are really shuffled

    std::vector<std::string> before_correct = std::vector<std::string>();
    std::vector<std::string> before_byzantine = std::vector<std::string>();
    for(int i = 0; i < bNetwork.size(); i++){
        if(bNetwork[i]->isByzantine()){
            before_byzantine.push_back(bNetwork[i]->id());
        }else{
            before_correct.push_back(bNetwork[i]->id());
        }
    }


    bNetwork.shuffleByzantines(PEERS*0.10); // shuffle some Byzantine peers

    std::vector<std::string> after_correct = std::vector<std::string>();
    std::vector<std::string> after_byzantine = std::vector<std::string>();
    for(int i = 0; i < bNetwork.size(); i++){
        if(bNetwork[i]->isByzantine()){
            after_byzantine.push_back(bNetwork[i]->id());
        }else{
            after_correct.push_back(bNetwork[i]->id());
        }
    }
    assert(before_byzantine != after_byzantine);
    assert(before_correct != after_correct);

    // check to make sure that nothing changes over repaeating this process
    for(int i = 0; i < 100; i++){

        for(int i = 0; i < bNetwork.size(); i++){
            bNetwork[i]->setCounter(i);
        }

        std::vector<std::string> before_correct = std::vector<std::string>();
        std::vector<std::string> before_byzantine = std::vector<std::string>();
        for(int i = 0; i < bNetwork.size(); i++){
            if(bNetwork[i]->isByzantine()){
                before_byzantine.push_back(bNetwork[i]->id());
            }else{
                before_correct.push_back(bNetwork[i]->id());
            }
        }

        bNetwork.shuffleByzantines(PEERS*0.10); // shuffle some Byzantine peers

        std::vector<std::string> after_correct = std::vector<std::string>();
        std::vector<std::string> after_byzantine = std::vector<std::string>();
        for(int i = 0; i < bNetwork.size(); i++){
            if(bNetwork[i]->isByzantine()){
                after_byzantine.push_back(bNetwork[i]->id());
            }else{
                after_correct.push_back(bNetwork[i]->id());
            }
        }
        assert(before_byzantine != after_byzantine);
        assert(before_correct != after_correct);
    for(auto peer = bNetwork.getByzantine().begin(); peer != bNetwork.getByzantine().end(); peer++){
        assert((*peer)->isByzantine()   == true);
        assert((*peer)->getCounter()    == i);
    }
    for(auto peer = bNetwork.getCorrect().begin(); peer != bNetwork.getCorrect().end(); peer++){
        assert((*peer)->isByzantine()   == false);
        assert((*peer)->getCounter()    == i);
    }
    }

    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testByzantineShuffle correct"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}