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
#include "./../SBFT/SBFTReferenceCommittee.hpp"
#include "metrics.hpp"
#include "./../params.hpp"

///////////////////////////////////////////
// MOTIVATIONAL
//
void SBFTCommitteeSizeVsSecurityAndThoughput(std::ofstream &csv, std::ofstream &log);

///////////////////////////////////////////
// PARAMETER
//
void SBFTThroughputTimeline(std::ofstream &csv, std::ofstream &log);

///////////////////////////////////////////
// ADAPTIVE SECURITY PERFORMACE GRAPHS
//
void SBFTWaitingTimeThroughputVsDelay(std::ofstream &csv, std::ofstream &log);
void SBFTWaitingTimeThroughputVsByzantine(std::ofstream &csv, std::ofstream &log);

void SBFTDefeatedTransactionVsByzantine(std::ofstream &csv, std::ofstream &log);

#endif /* Sharded_SBFT_Experiments_hpp */
