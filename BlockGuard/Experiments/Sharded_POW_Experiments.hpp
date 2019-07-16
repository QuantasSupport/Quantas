//
//  Sharded_POW_Experiments.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 6/20/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// POW Sharded with groups
//

#ifndef Sharded_POW_Experiments_hpp
#define Sharded_POW_Experiments_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <set>
#include <iostream>
#include <chrono>
#include <random>
#include "bCoinReferenceCommittee.hpp"
#include "params.hpp"
#include "metrics.hpp"


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
void MOTIVATIONAL11_Sharded_POW(std::ofstream &csv, std::ofstream &log);
void MOTIVATIONAL12_Sharded_POW(std::ofstream &csv, std::ofstream &log);
void PARAMETER1_Sharded_POW(std::ofstream &csv, std::ofstream &log);
void PARAMETER2_Sharded_POW(std::ofstream &csv, std::ofstream &log, int delay);
void ADAPTIVE11_Sharded_POW(std::ofstream &csv, std::ofstream &log, int delay);
void ADAPTIVE12_Sharded_POW(std::ofstream &csv, std::ofstream &log, double byzantine);
void ADAPTIVE21_Sharded_POW(std::ofstream &csv, std::ofstream &log, int delay);
void ADAPTIVE22_Sharded_POW(std::ofstream &csv, std::ofstream &log, double byzantine);
void ADAPTIVE3_Sharded_POW(std::ofstream &csv, std::ofstream &log, double byzantine);

#endif /* Sharded_POW_Experiments_hpp */
