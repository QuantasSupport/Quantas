//
//  Sharded_SBFT_Experiments.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 7/9/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef Sharded_SBFT_Experiments_hpp
#define Sharded_SBFT_Experiments_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <set>
#include <iostream>
#include <chrono>
#include <random>
#include "SBFTReferenceCommittee.hpp"
#include "metrics.hpp"
#include "params.hpp"

///////////////////////////////////////////
// MOTIVATIONAL
//
void CommitteeSizeVsSecurityAndThoughput(std::ofstream &csv, std::ofstream &log);

///////////////////////////////////////////
// PARAMETER
//
void ThroughputTimeline(std::ofstream &csv, std::ofstream &log);

///////////////////////////////////////////
// ADAPTIVE SECURITY PERFORMACE GRAPHS
//
void WaitingTimeThroughputVsDelay(std::ofstream &csv, std::ofstream &log);
void WaitingTimeThroughputVsByzantine(std::ofstream &csv, std::ofstream &log);

void DefeatedTransactionVsByzantine(std::ofstream &csv, std::ofstream &log);

///////////////////////////////////////////
// drafts
//
void MOTIVATIONAL11_Sharded_SBFT(std::ofstream &csv, std::ofstream &log);
void MOTIVATIONAL12_Sharded_SBFT(std::ofstream &csv, std::ofstream &log);
void PARAMETER1_Sharded_SBFT(std::ofstream &csv, std::ofstream &log);
void PARAMETER2_Sharded_SBFT(std::ofstream &csv, std::ofstream &log, int delay);
void ADAPTIVE11_Sharded_SBFT(std::ofstream &csv, std::ofstream &log, int delay);
void ADAPTIVE12_Sharded_SBFT(std::ofstream &csv, std::ofstream &log, double byzantine);
void ADAPTIVE21_Sharded_SBFT(std::ofstream &csv, std::ofstream &log, int delay);
void ADAPTIVE22_Sharded_SBFT(std::ofstream &csv, std::ofstream &log, double byzantine);
void ADAPTIVE3_Sharded_SBFT(std::ofstream &csv, std::ofstream &log, double byzantine);

#endif /* Sharded_SBFT_Experiments_hpp */
