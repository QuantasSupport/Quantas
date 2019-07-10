//
//  Sharded_SBFT_Experiments.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 7/9/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "Sharded_SBFT_Experiments.hpp"

//"Total Request:,Max Ledger:,Ratio Defeated To Honest 1,Ratio Defeated To Honest 2,Ratio Defeated To Honest 3,Ratio Defeated To Honest 4,Ratio Defeated To Honest 5,Average Waiting Time 1,Average Waiting Time 2,Average Waiting Time 3 ,Average Waiting Time 4,Average Waiting Time 5, total honest 1, total honest 2, total honest 3, total honest 4, total honest 5, total defeated 1, total defeated 2, total defeated 3, total defeated 4, total defeated 5\n"
void calculateResults(const SBFTReferenceCommittee system, std::ofstream &csv){
    std::vector<DAGBlock> globalLedger = system.getGlobalLedger();
    csv<<std::to_string(system.totalSubmissions());
    csv<< ","<< std::to_string(globalLedger.size());
    csv<< ","<< std::to_string(ratioOfSecLvl(globalLedger,system.securityLevel1()*system.getGroupSize()));
    csv<< ","<< std::to_string(ratioOfSecLvl(globalLedger,system.securityLevel2()*system.getGroupSize()));
    csv<< ","<< std::to_string(ratioOfSecLvl(globalLedger,system.securityLevel3()*system.getGroupSize()));
    csv<< ","<< std::to_string(ratioOfSecLvl(globalLedger,system.securityLevel4()*system.getGroupSize()));
    csv<< ","<< std::to_string(ratioOfSecLvl(globalLedger,system.securityLevel5()*system.getGroupSize()));

    csv<< ","<< std::to_string(waitTimeOfSecLvl(globalLedger,system.securityLevel1()*system.getGroupSize()));
    csv<< ","<< std::to_string(waitTimeOfSecLvl(globalLedger,system.securityLevel2()*system.getGroupSize()));
    csv<< ","<< std::to_string(waitTimeOfSecLvl(globalLedger,system.securityLevel3()*system.getGroupSize()));
    csv<< ","<< std::to_string(waitTimeOfSecLvl(globalLedger,system.securityLevel4()*system.getGroupSize()));
    csv<< ","<< std::to_string(waitTimeOfSecLvl(globalLedger,system.securityLevel5()*system.getGroupSize()));

    csv<< ","<< std::to_string(totalNumberOfCorrectCommittees(globalLedger,system.securityLevel1()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfCorrectCommittees(globalLedger,system.securityLevel2()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfCorrectCommittees(globalLedger,system.securityLevel3()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfCorrectCommittees(globalLedger,system.securityLevel4()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfCorrectCommittees(globalLedger,system.securityLevel5()*system.getGroupSize()));

    csv<< ","<< std::to_string(totalNumberOfDefeatedCommittees(globalLedger,system.securityLevel1()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfDefeatedCommittees(globalLedger,system.securityLevel2()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfDefeatedCommittees(globalLedger,system.securityLevel3()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfDefeatedCommittees(globalLedger,system.securityLevel4()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfDefeatedCommittees(globalLedger,system.securityLevel5()*system.getGroupSize()));
    csv<< std::endl;
    
}

void MOTIVATIONAL11_Sharded_SBFT(std::ofstream &csv, std::ofstream &log){
    return;
    std::string header = "Committee Size, Defeated Transactions, Confirmed/Submitted";
    
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        SBFTReferenceCommittee system = SBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        
        system.initNetwork(PEER_COUNT);
        
        system.makeByzantines(NUMBER_OF_BYZ);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
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
        csv<< header<< std::endl;
        csv<< system.securityLevel1()*GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel1())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
        csv<< system.securityLevel2()*GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel2())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
        csv<< system.securityLevel3()*GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel3())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
        csv<< system.securityLevel4()*GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel4())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
        csv<< system.securityLevel5()*GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel5())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
    }
}

void MOTIVATIONAL12_Sharded_SBFT(std::ofstream &csv, std::ofstream &log){
    return ;
    std::string header = "Committee Size, Defeated Transactions, Confirmed/Submitted";
    
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        SBFTReferenceCommittee system = SBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        
        system.initNetwork(PEER_COUNT);
        
        system.makeByzantines(NUMBER_OF_BYZ);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.shuffleByzantines(NUMBER_OF_BYZ);
            system.makeRequest(system.securityLevel1());
            system.makeRequest(system.securityLevel1());
            system.makeRequest(system.securityLevel1());
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
        }
        csv<< header<< std::endl;
        csv<< system.securityLevel1()*GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel1())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
    }
    
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        SBFTReferenceCommittee system = SBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        
        system.initNetwork(PEER_COUNT);
        
        system.makeByzantines(NUMBER_OF_BYZ);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.shuffleByzantines(NUMBER_OF_BYZ);
            system.makeRequest(system.securityLevel2());
            system.makeRequest(system.securityLevel2());
            system.makeRequest(system.securityLevel2());
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
        }
        csv<< header<< std::endl;
        csv<< system.securityLevel2()*GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel2())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
    }
    
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        SBFTReferenceCommittee system = SBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        
        system.initNetwork(PEER_COUNT);
        
        system.makeByzantines(NUMBER_OF_BYZ);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.shuffleByzantines(NUMBER_OF_BYZ);
            system.makeRequest(system.securityLevel3());
            system.makeRequest(system.securityLevel3());
            system.makeRequest(system.securityLevel3());
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
        }
        csv<< header<< std::endl;
        csv<< system.securityLevel3()*GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel3())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
        
    }
    
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        SBFTReferenceCommittee system = SBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        
        system.initNetwork(PEER_COUNT);
        
        system.makeByzantines(NUMBER_OF_BYZ);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest(system.securityLevel4());
            system.makeRequest(system.securityLevel4());
            system.makeRequest(system.securityLevel4());
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
        csv<< header<< std::endl;
        csv<< system.securityLevel4()*GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel4())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
        
    }
    
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        SBFTReferenceCommittee system = SBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        
        system.initNetwork(PEER_COUNT);
        
        system.makeByzantines(NUMBER_OF_BYZ);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.shuffleByzantines(NUMBER_OF_BYZ);
            system.makeRequest(system.securityLevel5());
            system.makeRequest(system.securityLevel5());
            system.makeRequest(system.securityLevel5());
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
        }
        csv<< header<< std::endl;
        csv<< system.securityLevel5()*GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel5())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
        
    }
    
}
void PARAMETER1_Sharded_SBFT(std::ofstream &csv, std::ofstream &log){
    return;
    std::string header = "Round, Confirmed/Submitted";
    csv<< header<< std::endl;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        SBFTReferenceCommittee system = SBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        
        system.initNetwork(PEER_COUNT);
        
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
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
                csv<< i<< ","<< double(system.getGlobalLedger().size())/totalSub<< std::endl;
            }
        }
        csv<< NUMBER_OF_ROUNDS<< ","<< double(system.getGlobalLedger().size())/totalSub<< std::endl;
    }
}

void PARAMETER2_Sharded_SBFT(std::ofstream &csv, std::ofstream &log, int delay){
    return ;
    std::string header = "Round, Confirmed/Submitted";
    csv<< header<< std::endl;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        SBFTReferenceCommittee system = SBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setMaxDelay(delay);
        system.setLog(log);
        
        system.initNetwork(PEER_COUNT);
        
        int totalSub = 0;
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
                csv<< i<< ","<< double(system.getGlobalLedger().size())/totalSub<< std::endl;
            }
        }
        csv<< NUMBER_OF_ROUNDS<< ","<< double(system.getGlobalLedger().size())/totalSub<< std::endl;
    }
}

void ADAPTIVE11_Sharded_SBFT(std::ofstream &csv, std::ofstream &log, int delay){
    return ;
    std::string header = "Round, Confirmed/Submitted";
    csv<< header<< std::endl;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        SBFTReferenceCommittee system = SBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setMaxDelay(delay);
        system.setLog(log);
        
        system.initNetwork(PEER_COUNT);
        
        system.makeByzantines(NUMBER_OF_BYZ);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            system.shuffleByzantines(NUMBER_OF_BYZ);
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                csv<< i<< ","<< double(system.getGlobalLedger().size())/totalSub<< std::endl;
            }
        }
        csv<< NUMBER_OF_ROUNDS<< ","<< double(system.getGlobalLedger().size())/totalSub<< std::endl;
    }
}
void ADAPTIVE12_Sharded_SBFT(std::ofstream &csv, std::ofstream &log, double byzantine){
    std::cout<< "ADAPTIVE12_Sharded_SBFT"<<std::endl;
    std::string header = "Round, Confirmed/Submitted";
    csv<< header<< std::endl;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        SBFTReferenceCommittee system = SBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        
        system.initNetwork(PEER_COUNT);
        
        system.makeByzantines(PEER_COUNT*byzantine);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            system.shuffleByzantines(PEER_COUNT*byzantine);
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                csv<< i<< ","<< double(system.getGlobalLedger().size())/totalSub<< std::endl;
            }
        }
        csv<< NUMBER_OF_ROUNDS<< ","<< double(system.getGlobalLedger().size())/totalSub<< std::endl;
    }
}
void ADAPTIVE21_Sharded_SBFT(std::ofstream &csv, std::ofstream &log, int delay){
    return ;
    std::string header = "Round, Confirmed/Submitted";
    csv<< header<< std::endl;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        SBFTReferenceCommittee system = SBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setMaxDelay(delay);
        system.setLog(log);
        
        system.initNetwork(PEER_COUNT);
        
        system.makeByzantines(NUMBER_OF_BYZ);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            system.shuffleByzantines(NUMBER_OF_BYZ);
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                csv<< i<< ","<< waitTime(system.getGlobalLedger())<< std::endl;
            }
        }
        csv<< NUMBER_OF_ROUNDS<< ","<< waitTime(system.getGlobalLedger())<< std::endl;
    }
}
void ADAPTIVE22_Sharded_SBFT(std::ofstream &csv, std::ofstream &log, double byzantine){
    std::string header = "Round, Confirmed/Submitted";
    csv<< header<< std::endl;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        SBFTReferenceCommittee system = SBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        
        system.initNetwork(PEER_COUNT);
        
        system.makeByzantines(PEER_COUNT*byzantine);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            system.shuffleByzantines(PEER_COUNT*byzantine);
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                csv<< i<< ","<< waitTime(system.getGlobalLedger())<< std::endl;
            }
        }
        csv<< NUMBER_OF_ROUNDS<< ","<< waitTime(system.getGlobalLedger())<< std::endl;
    }
}
void ADAPTIVE3_Sharded_SBFT(std::ofstream &csv, std::ofstream &log, double byzantine){
    std::string header = "fraction of byzantine, defeated/submitted";
    csv<< header<< std::endl;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        SBFTReferenceCommittee system = SBFTReferenceCommittee();
        system.setGroupSize(GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        
        system.initNetwork(PEER_COUNT);
        
        system.makeByzantines(PEER_COUNT*byzantine);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            system.shuffleByzantines(PEER_COUNT*byzantine);
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
        }
        csv<< byzantine<< ","<< defeatedTrnasactions(system.getGlobalLedger())<< std::endl;
    }
}
