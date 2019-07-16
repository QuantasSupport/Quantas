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
    
   // csv.open(filePath + "MOTIVATIONAL11_Sharded_SBFT.csv");
    
    log.close();
}
