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
#include "./../PBFT/PBFTReferenceCommittee.hpp"
#include "params_Blockguard.hpp"
#include "metrics.hpp"


///////////////////////////////////////////
// MOTIVATIONAL
//
void PBFTCommitteeSizeVsSecurityAndThoughput(std::ofstream &csv, std::ofstream &log);

///////////////////////////////////////////
// PARAMETER
// 
void PBFTThroughputTimeline(std::ofstream &csv, std::ofstream &log);

///////////////////////////////////////////
// ADAPTIVE SECURITY PERFORMACE GRAPHS
//
void PBFTWaitingTimeThroughputVsDelay(std::ofstream &csv, std::ofstream &log);
void PBFTWaitingTimeThroughputVsByzantine(std::ofstream &csv, std::ofstream &log);

void PBFTDefeatedTransactionVsByzantine(std::ofstream &csv, std::ofstream &log);

///////////////////////////////////////////
// util
//
std::vector<DAGBlock> PBFTLedgerToDag(std::vector<ledgerEntery>);

#endif /* Sharded_PBFT_Experiments_hpp */
