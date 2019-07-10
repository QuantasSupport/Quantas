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
    
    csv.open(filePath + "MOTIVATIONAL11_Sharded_PBFT.csv");
    if ( csv.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
    }
    MOTIVATIONAL11_Sharded_PBFT(csv,log);
    csv.close();
    
    csv.open(filePath + "MOTIVATIONAL12_Sharded_PBFT.csv");
    if ( csv.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
    }
    MOTIVATIONAL12_Sharded_PBFT(csv,log);
    csv.close();
    
    csv.open(filePath + "PARAMETER1_Sharded_PBFT.csv");
    if ( csv.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
    }
    PARAMETER1_Sharded_PBFT(csv, log);
    csv.close();
    
    for(int delay = 1; delay < 11; delay = delay + 2){
        csv.open(filePath + "PARAMETER2_Sharded_PBFT_" + std::to_string(delay) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        PARAMETER2_Sharded_PBFT(csv, log, delay);
        csv.close();
    }
    
    for(int delay = 1; delay < 11; delay = delay + 2){
        csv.open(filePath + "ADAPTIVE11_Sharded_PBFT_" + std::to_string(delay) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        ADAPTIVE11_Sharded_PBFT(csv, log, delay);
        csv.close();
        csv.open(filePath + "ADAPTIVE21_Sharded_PBFT_" + std::to_string(delay) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        ADAPTIVE21_Sharded_PBFT(csv, log, delay);
        csv.close();
    }
    
    for(double byz = 0; byz < 1; byz = byz + 0.2){
        csv.open(filePath + "ADAPTIVE12_Sharded_PBFT_" + std::to_string(byz) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        ADAPTIVE12_Sharded_PBFT(csv, log, byz);
        csv.close();
        csv.open(filePath + "ADAPTIVE22_Sharded_PBFT_" + std::to_string(byz) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        ADAPTIVE22_Sharded_PBFT(csv, log, byz);
        csv.close();
    }
    
    for(double byz = 0; byz < 1; byz = byz + 0.2){
        csv.open(filePath + "ADAPTIVE3_Sharded_PBFT_" + std::to_string(byz) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        ADAPTIVE3_Sharded_PBFT(csv,log,byz);
        csv.close();
    }
    log.close();
}

///////////////////////////////////////////////////////////////////
// POW
//
void POW_refCom(std::string filePath){
    std::cout<< "pow_s"<<std::endl;
    std::ofstream csv;
    std::ofstream log;
    log.open(filePath + "pow_s.log");
    if ( log.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + "pow_s.log" << std::endl;
    }

    csv.open(filePath + "MOTIVATIONAL11_Sharded_POW.csv");
    if ( csv.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
    }
    MOTIVATIONAL11_Sharded_POW(csv,log);
    csv.close();

    csv.open(filePath + "MOTIVATIONAL12_Sharded_POW.csv");
    if ( csv.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
    }
    MOTIVATIONAL12_Sharded_POW(csv,log);
    csv.close();

    csv.open(filePath + "PARAMETER1_Sharded_POW.csv");
    if ( csv.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
    }
    PARAMETER1_Sharded_POW(csv, log);
    csv.close();

    for(int delay = 1; delay < 11; delay = delay + 2){
        csv.open(filePath + "PARAMETER2_Sharded_pow_" + std::to_string(delay) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        PARAMETER2_Sharded_POW(csv, log, delay);
        csv.close();
    }

    for(int delay = 1; delay < 11; delay = delay + 2){
        csv.open(filePath + "ADAPTIVE11_Sharded_pow_" + std::to_string(delay) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        ADAPTIVE11_Sharded_POW(csv, log, delay);
        csv.close();
        csv.open(filePath + "ADAPTIVE21_Sharded_pow_" + std::to_string(delay) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        ADAPTIVE21_Sharded_POW(csv, log, delay);
        csv.close();
    }

    for(double byz = 0; byz < 1; byz = byz + 0.2){
        csv.open(filePath + "ADAPTIVE12_Sharded_pow_" + std::to_string(byz) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        ADAPTIVE12_Sharded_POW(csv, log, byz);
        csv.close();
        csv.open(filePath + "ADAPTIVE22_Sharded_pow_" + std::to_string(byz) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        ADAPTIVE22_Sharded_POW(csv, log, byz);
        csv.close();
    }

    for(double byz = 0; byz < 1; byz = byz + 0.2){
        csv.open(filePath + "ADAPTIVE3_Sharded_pow_" + std::to_string(byz) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        ADAPTIVE3_Sharded_POW(csv,log,byz);
        csv.close();
    }
    log.close();
}

void SBFT_refCom(std::string filePath){
    std::cout<< "sbft_s"<<std::endl;
    std::ofstream csv;
    std::ofstream log;
    log.open(filePath + "pow_s.log");
    if ( log.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + "pow_s.log" << std::endl;
    }
    
    csv.open(filePath + "MOTIVATIONAL11_Sharded_SBFT.csv");
    if ( csv.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
    }
    MOTIVATIONAL11_Sharded_SBFT(csv,log);
    csv.close();
    
    csv.open(filePath + "MOTIVATIONAL12_Sharded_SBFT.csv");
    if ( csv.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
    }
    MOTIVATIONAL12_Sharded_SBFT(csv,log);
    csv.close();
    
    csv.open(filePath + "PARAMETER1_Sharded_SBFT.csv");
    if ( csv.fail() ){
        std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
    }
    PARAMETER1_Sharded_SBFT(csv, log);
    csv.close();
    
    for(int delay = 1; delay < 11; delay = delay + 2){
        csv.open(filePath + "PARAMETER2_Sharded_SBFT_" + std::to_string(delay) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        PARAMETER2_Sharded_SBFT(csv, log, delay);
        csv.close();
    }
    
    for(int delay = 1; delay < 11; delay = delay + 2){
        csv.open(filePath + "ADAPTIVE11_Sharded_SBFT_" + std::to_string(delay) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        ADAPTIVE11_Sharded_SBFT(csv, log, delay);
        csv.close();
        csv.open(filePath + "ADAPTIVE21_Sharded_SBFT_" + std::to_string(delay) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        ADAPTIVE21_Sharded_SBFT(csv, log, delay);
        csv.close();
    }
    
    for(double byz = 0; byz < 1; byz = byz + 0.2){
        csv.open(filePath + "ADAPTIVE12_Sharded_SBFT_" + std::to_string(byz) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        ADAPTIVE12_Sharded_SBFT(csv, log, byz);
        csv.close();
        csv.open(filePath + "ADAPTIVE22_Sharded_SBFT_" + std::to_string(byz) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        ADAPTIVE22_Sharded_SBFT(csv, log, byz);
        csv.close();
    }
    
    for(double byz = 0; byz < 1; byz = byz + 0.2){
        csv.open(filePath + "ADAPTIVE3_Sharded_SBFT_" + std::to_string(byz) + ".csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        ADAPTIVE3_Sharded_SBFT(csv,log,byz);
        csv.close();
    }
    log.close();
}
