/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/
#include "NetworkTests.hpp"

using quantas::LOG_WIDTH;
using quantas::Network;
using quantas::ExamplePeer;
using quantas::ExampleMessage;
using quantas::UNIFORM;

void runNetworkTests(std::string filepath){
    
    std::ofstream log;
    log.open(filepath + "/NetworkTest.log");
    if (log.fail() ){
        std::cerr << "Error: could not open file at: "<< filepath << std::endl;
    }
    
    testSize(log);
    testIndex(log);
    testOneDelay(log);
    testRandomDelay(log);
    testPoissonDelay(log);
}
void testSize(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testSize"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    
    ///////////////////////////////////////
    // check default values with var network size
    
    Network<ExampleMessage, ExamplePeer> system = Network<ExampleMessage, ExamplePeer>();
    system.initNetwork(1);
    system.setLog(log);
    system.log();
    assert(system.maxDelay()     == 1);
    assert(system.minDelay()     == 1);
    assert(system.avgDelay()     == 1);
    assert(system.distribution() == UNIFORM);
    assert(system.size()         == 1);
    
    system = Network<ExampleMessage, ExamplePeer>();
    system.initNetwork(100);
    system.log();
    assert(system.maxDelay()     == 1);
    assert(system.minDelay()     == 1);
    assert(system.avgDelay()     == 1);
    assert(system.distribution() == UNIFORM);
    assert(system.size()         == 100);
    
    system = Network<ExampleMessage, ExamplePeer>();
    system.initNetwork(1000);
    system.log();
    assert(system.maxDelay()     == 1);
    assert(system.minDelay()     == 1);
    assert(system.avgDelay()     == 1);
    assert(system.distribution() == UNIFORM);
    assert(system.size()         == 1000);
    
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testSize Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void testIndex(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testIndex"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    Network<ExampleMessage, ExamplePeer> system = Network<ExampleMessage, ExamplePeer>();
    system.initNetwork(1);
    system.setLog(log);
    system.log();
    assert(system[0]->id()         != -1);
    assert(system[0]->getCounter() == 0);

    system = Network<ExampleMessage, ExamplePeer>();
    system.initNetwork(100);
    system.setLog(log);
    system.log();
    for(int i = 0; i < 100; i++){
        assert(system[i]->id()                  != -1);
        assert(system[i]->getCounter()          == 0);
        assert(system[i]->neighbors().size()    == 99); // 100 - self
    }
    
    system = Network<ExampleMessage, ExamplePeer>();
    system.initNetwork(1000);
    system.setLog(log);
    system.log();
    for(int i = 0; i < 1000; i++){
        assert(system[i]->id()                  != -1);
        assert(system[i]->getCounter()          == 0);
        assert(system[i]->neighbors().size()    == 999); // 5000 - self
    }
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testIndex complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void testOneDelay(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testOneDelay"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    Network<ExampleMessage, ExamplePeer> system = Network<ExampleMessage, ExamplePeer>();
    system.initNetwork(100);
    system.setLog(log);
    system.log();
    for(int i = 0; i < 100; i++){
        assert(system[i]->id()                  != -1);
        assert(system[i]->getCounter()          == 0);
        assert(system[i]->neighbors().size()    == 99); // 100 - self
        for(int n = 0; n < system[i]->neighbors().size(); n++){
            assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) == 1);
        }
    }
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testOneDelay complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void testRandomDelay(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testOneDelay complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    // sense this is random we do it a bunch of times to see if it holds
    
    for(int i = 0; i < 100; i++){
        Network<ExampleMessage, ExamplePeer> system = Network<ExampleMessage, ExamplePeer>();
        system.setLog(log);
        int max = 2;
        int min = 1;
        
        system.setMaxDelay(max);
        system.setMinDelay(min);
        system.initNetwork(10);
        
        system.log();
        for(int i = 0; i < 10; i++){
            assert(system[i]->id()                  != -1);
            assert(system[i]->getCounter()          == 0);
            assert(system[i]->neighbors().size()    == 9); // 100 - self
            for(int n = 0; n < system[i]->neighbors().size(); n++){
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) >= min);
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) <= max);
            }
        }
    }
    
    for(int i = 0; i < 100; i++){
        Network<ExampleMessage, ExamplePeer> system = Network<ExampleMessage, ExamplePeer>();
        system.setLog(log);
        int max = 5;
        int min = 1;
        system.setMaxDelay(max);
        system.setMinDelay(min);
        system.initNetwork(10);
        system.log();
        for(int i = 0; i < 10; i++){
            assert(system[i]->id()                  != -1);
            assert(system[i]->getCounter()          == 0);
            assert(system[i]->neighbors().size()    == 9); // 100 - self
            for(int n = 0; n < system[i]->neighbors().size(); n++){
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) >= min);
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) <= max);
            }
        }
    }
    
    for(int i = 0; i < 100; i++){
        Network<ExampleMessage, ExamplePeer> system = Network<ExampleMessage, ExamplePeer>();
        system.setLog(log);
        int max = 100;
        int min = 1;
        system.setMaxDelay(max);
        system.setMinDelay(min);
        system.initNetwork(10);
        system.log();
        for(int i = 0; i < 10; i++){
            assert(system[i]->id()                  != -1);
            assert(system[i]->getCounter()          == 0);
            assert(system[i]->neighbors().size()    == 9); // 100 - self
            for(int n = 0; n < system[i]->neighbors().size(); n++){
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) >= min);
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) <= max);
            }
        }
    }
    
    for(int i = 0; i < 100; i++){
        Network<ExampleMessage, ExamplePeer> system = Network<ExampleMessage, ExamplePeer>();
        system.setLog(log);
        int max = 5;
        int min = 2;
        system.setMaxDelay(max);
        system.setMinDelay(min);
        system.initNetwork(10);
        system.log();
        for(int i = 0; i < 10; i++){
            assert(system[i]->id()                  != -1);
            assert(system[i]->getCounter()          == 0);
            assert(system[i]->neighbors().size()    == 9); // 100 - self
            for(int n = 0; n < system[i]->neighbors().size(); n++){
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) >= min);
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) <= max);
            }
        }
    }
    
    for(int i = 0; i < 100; i++){
        Network<ExampleMessage, ExamplePeer> system = Network<ExampleMessage, ExamplePeer>();
        system.setLog(log);
        int max = 1000;
        int min = 100;
        system.setMaxDelay(max);
        system.setMinDelay(min);
        system.initNetwork(10);
        system.log();
        for(int i = 0; i < 10; i++){
            assert(system[i]->id()                  != -1);
            assert(system[i]->getCounter()          == 0);
            assert(system[i]->neighbors().size()    == 9); // 100 - self
            for(int n = 0; n < system[i]->neighbors().size(); n++){
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) >= min);
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) <= max);
            }
        }
    }
    
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testOneDelay complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
void testPoissonDelay(std::ostream &log){
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testPoissonDelay"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
    // note: this does not test to validate that this is in fact poisson distrubtion, only that the values fall with in the range provided
    // sense this is random we do it a bunch of times to see if it holds
    for(int i = 0; i < 100; i++){
        Network<ExampleMessage, ExamplePeer> system = Network<ExampleMessage, ExamplePeer>();
        system.setLog(log);
        int max = 2;
        int min = 1;
        int avg = 1;
        system.setToPoisson();
        system.setMaxDelay(max);
        system.setAvgDelay(avg);
        system.setMinDelay(min);
        system.initNetwork(10);
        system.log();
        for(int i = 0; i < 10; i++){
            assert(system[i]->id()                  != -1);
            assert(system[i]->getCounter()          == 0);
            assert(system[i]->neighbors().size()    == 9); // 10 - self
            for(int n = 0; n < system[i]->neighbors().size(); n++){
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) >= min);
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) <= max);
            }
        }
    }
    
    for(int i = 0; i < 100; i++){
        Network<ExampleMessage, ExamplePeer> system = Network<ExampleMessage, ExamplePeer>();
        system.setLog(log);
        int max = 5;
        int min = 1;
        int avg = 2;
        system.setToPoisson();
        system.setMaxDelay(max);
        system.setAvgDelay(avg);
        system.setMinDelay(min);
        system.initNetwork(10);
        system.log();
        for(int i = 0; i < 10; i++){
            assert(system[i]->id()                  != -1);
            assert(system[i]->getCounter()          == 0);
            assert(system[i]->neighbors().size()    == 9); // 10 - self
            for(int n = 0; n < system[i]->neighbors().size(); n++){
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) >= min);
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) <= max);
            }
        }
    }
    
    for(int i = 0; i < 100; i++){
        Network<ExampleMessage, ExamplePeer> system = Network<ExampleMessage, ExamplePeer>();
        system.setLog(log);
        int max = 20;
        int min = 10;
        int avg = 15;
        system.setToPoisson();
        system.setMaxDelay(max);
        system.setAvgDelay(avg);
        system.setMinDelay(min);
        system.initNetwork(10);
        system.log();
        for(int i = 0; i < 10; i++){
            assert(system[i]->id()                  != -1);
            assert(system[i]->getCounter()          == 0);
            assert(system[i]->neighbors().size()    == 9); // 10 - self
            for(int n = 0; n < system[i]->neighbors().size(); n++){
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) >= min);
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) <= max);
            }
        }
    }
    
    for(int i = 0; i < 100; i++){
        Network<ExampleMessage, ExamplePeer> system = Network<ExampleMessage, ExamplePeer>();
        system.setLog(log);
        int max = 200;
        int min = 100;
        int avg = 90;
        system.setToPoisson();
        system.setMaxDelay(max);
        system.setAvgDelay(avg);
        system.setMinDelay(min);
        system.initNetwork(10);
        system.log();
        for(int i = 0; i < 10; i++){
            assert(system[i]->id()                  != -1);
            assert(system[i]->getCounter()          == 0);
            assert(system[i]->neighbors().size()    == 9); // 10 - self
            for(int n = 0; n < system[i]->neighbors().size(); n++){
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) >= min);
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) <= max);
            }
        }
    }
    
    for(int i = 0; i < 100; i++){
        Network<ExampleMessage, ExamplePeer> system = Network<ExampleMessage, ExamplePeer>();
        system.setLog(log);
        int max = 20;
        int min = 10;
        int avg = 25;
        system.setToPoisson();
        system.setMaxDelay(max);
        system.setAvgDelay(avg);
        system.setMinDelay(min);
        system.initNetwork(10);
        system.log();
        for(int i = 0; i < 10; i++){
            assert(system[i]->id()                  != -1);
            assert(system[i]->getCounter()          == 0);
            assert(system[i]->neighbors().size()    == 9); // 10 - self
            for(int n = 0; n < system[i]->neighbors().size(); n++){
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) >= min);
                assert(system[i]->getDelayToNeighbor(system[i]->neighbors()[n]) <= max);
            }
        }
    }
    log<< std::endl<< "###############################"<< std::setw(LOG_WIDTH)<< std::left<<"!!!"<<"testPoissonDelay Complete"<< std::setw(LOG_WIDTH)<< std::right<<"!!!"<<"###############################"<< std::endl;
}
