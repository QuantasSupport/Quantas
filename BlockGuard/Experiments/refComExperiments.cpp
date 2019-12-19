//
//  refComExperiments.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 7/9/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "refComExperiments.hpp"

///////////////////////////////////////////////////////////////////
// PBFT
//
void PBFT_refCom(std::string filePath){
    std::cout<< "pbft_s"<<std::endl;
    std::ofstream csv;
    std::ofstream log;
    log.open(filePath + "pbft_s.log");
    if ( log.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + "pbft_s.log" << std::endl;
    }
    
    csv.open(filePath + "PBFTCommitteeSizeVsSecurityAndThoughput.csv");
    if ( log.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + "PBFTCommitteeSizeVsSecurityAndThoughput.csv" << std::endl;
    }
    PBFTCommitteeSizeVsSecurityAndThoughput(csv,log);
    csv.close();
    
    csv.open(filePath + "PBFTWaitingTimeThroughputVsDelay.csv");
    if ( log.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + "PBFTWaitingTimeThroughputVsDelay.csv" << std::endl;
    }
    PBFTWaitingTimeThroughputVsDelay(csv,log);
    csv.close();
    
    csv.open(filePath + "PBFTWaitingTimeThroughputVsByzantine.csv");
    if ( log.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + "PBFTWaitingTimeThroughputVsByzantine.csv" << std::endl;
    }
    PBFTWaitingTimeThroughputVsByzantine(csv,log);
    csv.close();
    
    log.close();
}

void POW_refCom(std::string filePath){
    std::cout<< "pow_s"<<std::endl;
    std::ofstream csv;
    std::ofstream log;
    log.open(filePath + "pow_s.log");
    if ( log.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + "pbft_s.log" << std::endl;
    }
    
    csv.open(filePath + "POWCommitteeSizeVsSecurityAndThoughput.csv");
    if ( log.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + "POWCommitteeSizeVsSecurityAndThoughput.csv" << std::endl;
    }
//    PoWCommitteeSizeVsSecurityAndThoughput(csv,log);
    csv.close();
    
    csv.open(filePath + "POWWaitingTimeThroughputVsDelay.csv");
    if ( log.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + "POWWaitingTimeThroughputVsDelay.csv" << std::endl;
    }
//    PoWWaitingTimeThroughputVsDelay(csv,log);
    csv.close();
    
    csv.open(filePath + "POWWaitingTimeThroughputVsByzantine.csv");
    if ( log.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + "POWWaitingTimeThroughputVsByzantine.csv" << std::endl;
    }
//    PoWWaitingTimeThroughputVsByzantine(csv,log);
    csv.close();
    
    
    log.close();
}

void SBFT_refCom(std::string filePath){
    std::cout<< "sbft_s"<<std::endl;
    std::ofstream csv;
    std::ofstream log;
    log.open(filePath + "sbft_s.log");
    if ( log.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + "pow_s.log" << std::endl;
    }
    
    csv.open(filePath + "SBFTCommitteeSizeVsSecurityAndThoughput.csv");
    if ( log.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + "SBFTCommitteeSizeVsSecurityAndThoughput.csv" << std::endl;
    }
//    SBFTCommitteeSizeVsSecurityAndThoughput(csv,log);
    csv.close();
    
    csv.open(filePath + "SBFTWaitingTimeThroughputVsDelay.csv");
    if ( log.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + "SBFTWaitingTimeThroughputVsDelay.csv" << std::endl;
    }
//    SBFTWaitingTimeThroughputVsDelay(csv,log);
    csv.close();
    
    csv.open(filePath + "SBFTWaitingTimeThroughputVsByzantine.csv");
    if ( log.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + "SBFTWaitingTimeThroughputVsByzantine.csv" << std::endl;
    }
//    SBFTWaitingTimeThroughputVsByzantine(csv,log);
    csv.close();
    
    
    log.close();
}
