//
//  Sharded_PBFT_Experiments.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 6/20/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PBFT Sharded with groups
//

#ifndef Sharded_PBFT_Experiments_hpp
#define Sharded_PBFT_Experiments_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <set>
#include <iostream>
#include <chrono>
#include <random>
#include "PBFTReferenceCommittee.hpp"
#include "params.hpp"

const static std::string s_pbft_header = "Total Request:,Max Ledger:,Ratio Defeated To Honest 1,Ratio Defeated To Honest 2,Ratio Defeated To Honest 3,Ratio Defeated To Honest 4,Ratio Defeated To Honest 5,Average Waiting Time 1,Average Waiting Time 2,Average Waiting Time 3 ,Average Waiting Time 4,Average Waiting Time 5, total honest 1, total honest 2, total honest 3, total honest 4, total honest 5, total defeated 1, total defeated 2, total defeated 3, total defeated 4, total defeated 5\n";

void Sharded_PBFT(std::ofstream &csv, std::ofstream &log,int,double);

// experiments
void MOTIVATIONAL11_Sharded_PBFT(std::ofstream &csv, std::ofstream &log);
void MOTIVATIONAL12_Sharded_PBFT(std::ofstream &csv, std::ofstream &log);
void PARAMETER1_Sharded_PBFT(std::ofstream &csv, std::ofstream &log);
void PARAMETER2_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, int delay);
void ADAPTIVE11_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, int delay);
void ADAPTIVE12_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, double byzantine);
void ADAPTIVE21_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, int delay);
void ADAPTIVE22_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, double byzantine);
void ADAPTIVE3_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, double byzantine);

#endif /* Sharded_PBFT_Experiments_hpp */
