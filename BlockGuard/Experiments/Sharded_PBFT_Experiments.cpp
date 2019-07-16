//
//  Sharded_PBFT_Experiments.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 6/20/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "Sharded_PBFT_Experiments.hpp"

void PBFTCommitteeSizeVsSecurityAndThoughput(std::ofstream &csv, std::ofstream &log){
    std::string header = "Committee Size, Ratio Defeated Committees, Confirmed/Submitted";
    csv<< header<< std::endl;
    
    // sec lvl 1
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.initNetwork(PEER_COUNT);
        int secLvel = system.securityLevel1();
        
        system.makeByzantines(NUMBER_OF_BYZ);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest(secLvel);
            system.makeRequest(secLvel);
            system.makeRequest(secLvel);
            system.shuffleByzantines(NUMBER_OF_BYZ);
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
        }
        double totalDef = totalNumberOfDefeatedCommittees(PBFTLedgerToDag(system.getGlobalLedger()),secLvel);
        double totalHonest = PBFTLedgerToDag(system.getGlobalLedger()).size() - totalNumberOfDefeatedCommittees(PBFTLedgerToDag(system.getGlobalLedger()),secLvel);
        double ratioOfDefToHonest = totalDef / totalHonest;
        csv<< system.securityLevel1()*GROUP_SIZE<< ","<< ratioOfDefToHonest << ","<< double(PBFTLedgerToDag(system.getGlobalLedger()).size()) / totalSub<<std::endl;
    } // end loop runs
    
    // sec lvl 2
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.initNetwork(PEER_COUNT);
        int secLvel = system.securityLevel2();
        
        system.makeByzantines(NUMBER_OF_BYZ);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest(secLvel);
            system.makeRequest(secLvel);
            system.makeRequest(secLvel);
            system.shuffleByzantines(NUMBER_OF_BYZ);
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
        }
        double totalDef = totalNumberOfDefeatedCommittees(PBFTLedgerToDag(system.getGlobalLedger()),secLvel);
        double totalHonest = PBFTLedgerToDag(system.getGlobalLedger()).size() - totalNumberOfDefeatedCommittees(PBFTLedgerToDag(system.getGlobalLedger()),secLvel);
        double ratioOfDefToHonest = totalDef / totalHonest;
        csv<< system.securityLevel1()*GROUP_SIZE<< ","<< ratioOfDefToHonest << ","<< double(PBFTLedgerToDag(system.getGlobalLedger()).size()) / totalSub<<std::endl;
    } // end loop runs
    
    // sec lvl 3
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.initNetwork(PEER_COUNT);
        int secLvel = system.securityLevel3();
        
        system.makeByzantines(NUMBER_OF_BYZ);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest(secLvel);
            system.makeRequest(secLvel);
            system.makeRequest(secLvel);
            system.shuffleByzantines(NUMBER_OF_BYZ);
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
        }
        double totalDef = totalNumberOfDefeatedCommittees(PBFTLedgerToDag(system.getGlobalLedger()),secLvel);
        double totalHonest = PBFTLedgerToDag(system.getGlobalLedger()).size() - totalNumberOfDefeatedCommittees(PBFTLedgerToDag(system.getGlobalLedger()),secLvel);
        double ratioOfDefToHonest = totalDef / totalHonest;
        csv<< system.securityLevel1()*GROUP_SIZE<< ","<< ratioOfDefToHonest << ","<< double(PBFTLedgerToDag(system.getGlobalLedger()).size()) / totalSub<<std::endl;
    } // end loop runs
    
    // sec lvl 4
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.initNetwork(PEER_COUNT);
        int secLvel = system.securityLevel4();
        
        system.makeByzantines(NUMBER_OF_BYZ);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest(secLvel);
            system.makeRequest(secLvel);
            system.makeRequest(secLvel);
            system.shuffleByzantines(NUMBER_OF_BYZ);
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
        }
        double totalDef = totalNumberOfDefeatedCommittees(PBFTLedgerToDag(system.getGlobalLedger()),secLvel);
        double totalHonest = PBFTLedgerToDag(system.getGlobalLedger()).size() - totalNumberOfDefeatedCommittees(PBFTLedgerToDag(system.getGlobalLedger()),secLvel);
        double ratioOfDefToHonest = totalDef / totalHonest;
        csv<< system.securityLevel1()*GROUP_SIZE<< ","<< ratioOfDefToHonest << ","<< double(PBFTLedgerToDag(system.getGlobalLedger()).size()) / totalSub<<std::endl;
    } // end loop runs
    
    // sec lvl 5
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.initNetwork(PEER_COUNT);
        int secLvel = system.securityLevel5();
        
        system.makeByzantines(NUMBER_OF_BYZ);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest(secLvel);
            system.makeRequest(secLvel);
            system.makeRequest(secLvel);
            system.shuffleByzantines(NUMBER_OF_BYZ);
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
        }
        double totalDef = totalNumberOfDefeatedCommittees(PBFTLedgerToDag(system.getGlobalLedger()),secLvel);
        double totalHonest = PBFTLedgerToDag(system.getGlobalLedger()).size() - totalNumberOfDefeatedCommittees(PBFTLedgerToDag(system.getGlobalLedger()),secLvel);
        double ratioOfDefToHonest = totalDef / totalHonest;
        csv<< system.securityLevel1()*GROUP_SIZE<< ","<< ratioOfDefToHonest << ","<< double(PBFTLedgerToDag(system.getGlobalLedger()).size()) / totalSub<<std::endl;
    } // end loop runs
}

///////////////////////////////////////////////////////////////////////////////////////////
//
//
void PBFTWaitingTimeThroughputVsDelay(std::ofstream &csv, std::ofstream &log){
    int delay = 0;
    std::string header = "Round, Confirmed/Submitted, Average Waiting Time,  delay";
    csv<< header<< std::endl;
    
    // delay 1
    delay = 1;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setMaxDelay(delay);
        system.setLog(log);
        system.makeByzantines(NUMBER_OF_BYZ);
        system.initNetwork(PEER_COUNT);
        
        int totalSub = 0;
        int prvConfirmed = 0;
        int prvSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.shuffleByzantines(NUMBER_OF_BYZ);
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                double last100RoundCon = PBFTLedgerToDag(system.getGlobalLedger()).size() - prvConfirmed;
                double last100RoundSub = totalSub - prvSub;
                double waitingTime = waitTimeRolling(PBFTLedgerToDag(system.getGlobalLedger()),i-100);
                csv<< i<< ","<< last100RoundCon / last100RoundSub<< ","<< waitingTime<< ","<<delay<< std::endl;
                prvConfirmed = PBFTLedgerToDag(system.getGlobalLedger()).size();
                prvSub = totalSub;
            }
        }
        double last100RoundCon = PBFTLedgerToDag(system.getGlobalLedger()).size() - prvConfirmed;
        double last100RoundSub = totalSub - prvSub;
        double waitingTime = waitTimeRolling(PBFTLedgerToDag(system.getGlobalLedger()),NUMBER_OF_ROUNDS-100);
        csv<< NUMBER_OF_ROUNDS<< ","<< last100RoundCon / last100RoundSub<< ","<< waitingTime<< ","<<delay<< std::endl;
    }// end loop runs
    
    // delay 3
    delay = 3;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setMaxDelay(delay);
        system.setLog(log);
        system.makeByzantines(NUMBER_OF_BYZ);
        system.initNetwork(PEER_COUNT);
        
        int totalSub = 0;
        int prvConfirmed = 0;
        int prvSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.shuffleByzantines(NUMBER_OF_BYZ);
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                double last100RoundCon = PBFTLedgerToDag(system.getGlobalLedger()).size() - prvConfirmed;
                double last100RoundSub = totalSub - prvSub;
                double waitingTime = waitTimeRolling(PBFTLedgerToDag(system.getGlobalLedger()),i-100);
                csv<< i<< ","<< last100RoundCon / last100RoundSub<< ","<< waitingTime<< ","<<delay<< std::endl;
                prvConfirmed = PBFTLedgerToDag(system.getGlobalLedger()).size();
                prvSub = totalSub;
            }
        }
        double last100RoundCon = PBFTLedgerToDag(system.getGlobalLedger()).size() - prvConfirmed;
        double last100RoundSub = totalSub - prvSub;
        double waitingTime = waitTimeRolling(PBFTLedgerToDag(system.getGlobalLedger()),NUMBER_OF_ROUNDS-100);
        csv<< NUMBER_OF_ROUNDS<< ","<< last100RoundCon / last100RoundSub<< ","<< waitingTime<< ","<<delay<< std::endl;
    }// end loop runs
    
    // delay 5
    delay = 5;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setMaxDelay(delay);
        system.setLog(log);
        system.makeByzantines(NUMBER_OF_BYZ);
        system.initNetwork(PEER_COUNT);
        
        int totalSub = 0;
        int prvConfirmed = 0;
        int prvSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.shuffleByzantines(NUMBER_OF_BYZ);
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                double last100RoundCon = PBFTLedgerToDag(system.getGlobalLedger()).size() - prvConfirmed;
                double last100RoundSub = totalSub - prvSub;
                double waitingTime = waitTimeRolling(PBFTLedgerToDag(system.getGlobalLedger()),i-100);
                csv<< i<< ","<< last100RoundCon / last100RoundSub<< ","<< waitingTime<< ","<<delay<< std::endl;
                prvConfirmed = PBFTLedgerToDag(system.getGlobalLedger()).size();
                prvSub = totalSub;
            }
        }
        double last100RoundCon = PBFTLedgerToDag(system.getGlobalLedger()).size() - prvConfirmed;
        double last100RoundSub = totalSub - prvSub;
        double waitingTime = waitTimeRolling(PBFTLedgerToDag(system.getGlobalLedger()),NUMBER_OF_ROUNDS-100);
        csv<< NUMBER_OF_ROUNDS<< ","<< last100RoundCon / last100RoundSub<< ","<< waitingTime<< ","<<delay<< std::endl;
    }// end loop runs
    
    // delay 10
    delay = 10;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setMaxDelay(delay);
        system.setLog(log);
        system.makeByzantines(NUMBER_OF_BYZ);
        system.initNetwork(PEER_COUNT);
        
        int totalSub = 0;
        int prvConfirmed = 0;
        int prvSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.shuffleByzantines(NUMBER_OF_BYZ);
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                double last100RoundCon = PBFTLedgerToDag(system.getGlobalLedger()).size() - prvConfirmed;
                double last100RoundSub = totalSub - prvSub;
                double waitingTime = waitTimeRolling(PBFTLedgerToDag(system.getGlobalLedger()),i-100);
                csv<< i<< ","<< last100RoundCon / last100RoundSub<< ","<< waitingTime<< ","<<delay<< std::endl;
                prvConfirmed = PBFTLedgerToDag(system.getGlobalLedger()).size();
                prvSub = totalSub;
            }
        }
        double last100RoundCon = PBFTLedgerToDag(system.getGlobalLedger()).size() - prvConfirmed;
        double last100RoundSub = totalSub - prvSub;
        double waitingTime = waitTimeRolling(PBFTLedgerToDag(system.getGlobalLedger()),NUMBER_OF_ROUNDS-100);
        csv<< NUMBER_OF_ROUNDS<< ","<< last100RoundCon / last100RoundSub<< ","<< waitingTime<< ","<<delay<< std::endl;
    }// end loop runs
}

///////////////////////////////////////////////////////////////////////////////////////////
//
//
void PBFTWaitingTimeThroughputVsByzantine(std::ofstream &csv, std::ofstream &log){
    double byzantine = 0.0;
    std::string header = "Round, Confirmed/Submitted, Byzantine";
    csv<< header<< std::endl;
    
    // byzantine 1/10
    byzantine = 0.09;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.makeByzantines(PEER_COUNT*byzantine);
        system.initNetwork(PEER_COUNT);
        
        int totalSub = 0;
        int prvConfirmed = 0;
        int prvSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.shuffleByzantines(PEER_COUNT*byzantine);
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                double last100RoundCon = PBFTLedgerToDag(system.getGlobalLedger()).size() - prvConfirmed;
                double last100RoundSub = totalSub - prvSub;
                double waitingTime = waitTimeRolling(PBFTLedgerToDag(system.getGlobalLedger()),i-100);
                csv<< i<< ","<< last100RoundCon / last100RoundSub<< ","<< waitingTime<< ","<<byzantine<< std::endl;
                prvConfirmed = PBFTLedgerToDag(system.getGlobalLedger()).size();
                prvSub = totalSub;
            }
        }
        double last100RoundCon = PBFTLedgerToDag(system.getGlobalLedger()).size() - prvConfirmed;
        double last100RoundSub = totalSub - prvSub;
        double waitingTime = waitTimeRolling(PBFTLedgerToDag(system.getGlobalLedger()),NUMBER_OF_ROUNDS-100);
        csv<< NUMBER_OF_ROUNDS<< ","<< last100RoundCon / last100RoundSub<< ","<< waitingTime<< ","<<byzantine<< std::endl;
    }// end loop runs
    
    // byzantine 1/5
    byzantine = 0.19;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.makeByzantines(PEER_COUNT*byzantine);
        system.initNetwork(PEER_COUNT);
        
        int totalSub = 0;
        int prvConfirmed = 0;
        int prvSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.shuffleByzantines(PEER_COUNT*byzantine);
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                double last100RoundCon = PBFTLedgerToDag(system.getGlobalLedger()).size() - prvConfirmed;
                double last100RoundSub = totalSub - prvSub;
                double waitingTime = waitTimeRolling(PBFTLedgerToDag(system.getGlobalLedger()),i-100);
                csv<< i<< ","<< last100RoundCon / last100RoundSub<< ","<< waitingTime<< ","<<byzantine<< std::endl;
                prvConfirmed = PBFTLedgerToDag(system.getGlobalLedger()).size();
                prvSub = totalSub;
            }
        }
        double last100RoundCon = PBFTLedgerToDag(system.getGlobalLedger()).size() - prvConfirmed;
        double last100RoundSub = totalSub - prvSub;
        double waitingTime = waitTimeRolling(PBFTLedgerToDag(system.getGlobalLedger()),NUMBER_OF_ROUNDS-100);
        csv<< NUMBER_OF_ROUNDS<< ","<< last100RoundCon / last100RoundSub<< ","<< waitingTime<< ","<<byzantine<< std::endl;
    }// end loop runs
    
    // delay 1/3
    byzantine = 0.29;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.makeByzantines(PEER_COUNT*byzantine);
        system.initNetwork(PEER_COUNT);
        
        int totalSub = 0;
        int prvConfirmed = 0;
        int prvSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.shuffleByzantines(PEER_COUNT*byzantine);
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                double last100RoundCon = PBFTLedgerToDag(system.getGlobalLedger()).size() - prvConfirmed;
                double last100RoundSub = totalSub - prvSub;
                double waitingTime = waitTimeRolling(PBFTLedgerToDag(system.getGlobalLedger()),i-100);
                csv<< i<< ","<< last100RoundCon / last100RoundSub<< ","<< waitingTime<< ","<<byzantine<< std::endl;
                prvConfirmed = PBFTLedgerToDag(system.getGlobalLedger()).size();
                prvSub = totalSub;
            }
        }
        double last100RoundCon = PBFTLedgerToDag(system.getGlobalLedger()).size() - prvConfirmed;
        double last100RoundSub = totalSub - prvSub;
        double waitingTime = waitTimeRolling(PBFTLedgerToDag(system.getGlobalLedger()),NUMBER_OF_ROUNDS-100);
        csv<< NUMBER_OF_ROUNDS<< ","<< last100RoundCon / last100RoundSub<< ","<< waitingTime<< ","<<byzantine<< std::endl;
    }// end loop runs
    
    // byzantine 1/2
    byzantine = 0.49;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.makeByzantines(PEER_COUNT*byzantine);
        system.initNetwork(PEER_COUNT);
        
        int totalSub = 0;
        int prvConfirmed = 0;
        int prvSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.shuffleByzantines(PEER_COUNT*byzantine);
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                double last100RoundCon = PBFTLedgerToDag(system.getGlobalLedger()).size() - prvConfirmed;
                double last100RoundSub = totalSub - prvSub;
                double waitingTime = waitTimeRolling(PBFTLedgerToDag(system.getGlobalLedger()),i-100);
                csv<< i<< ","<< last100RoundCon / last100RoundSub<< ","<< waitingTime<< ","<<byzantine<< std::endl;
                prvConfirmed = PBFTLedgerToDag(system.getGlobalLedger()).size();
                prvSub = totalSub;
            }
        }
        double last100RoundCon = PBFTLedgerToDag(system.getGlobalLedger()).size() - prvConfirmed;
        double last100RoundSub = totalSub - prvSub;
        double waitingTime = waitTimeRolling(PBFTLedgerToDag(system.getGlobalLedger()),NUMBER_OF_ROUNDS-100);
        csv<< NUMBER_OF_ROUNDS<< ","<< last100RoundCon / last100RoundSub<< ","<< waitingTime<< ","<<byzantine<< std::endl;
    }// end loop runs
}

////////////////////////////////////////////////////////////
// util
//
std::vector<DAGBlock> PBFTLedgerToDag(std::vector<ledgerEntery> PBFTLedger){
    std::vector<DAGBlock> newDag;
    for(int i = 0; i < PBFTLedger.size(); i++){
        newDag.push_back(PBFTLedger[i].first.toDAGBlock());
    }
    return newDag;
}
