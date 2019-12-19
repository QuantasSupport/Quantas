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

#include <iostream>
#include <fstream>
#include <set>
#include <iostream>
#include <chrono>
#include <random>
#include "./../bCoin/bCoinReferenceCommittee.hpp"
#include "./../params_Blockguard.hpp"
#include "metrics.hpp"


///////////////////////////////////////////
// MOTIVATIONAL
//
void PoWCommitteeSizeVsSecurityAndThoughput(std::ofstream &csv, std::ofstream &log);

///////////////////////////////////////////
// PARAMETER
//
void PoWThroughputTimeline(std::ofstream &csv, std::ofstream &log);

///////////////////////////////////////////
// ADAPTIVE SECURITY PERFORMACE GRAPHS
//
void PoWWaitingTimeThroughputVsDelay(std::ofstream &csv, std::ofstream &log);
void PoWWaitingTimeThroughputVsByzantine(std::ofstream &csv, std::ofstream &log);

void PoWDefeatedTransactionVsByzantine(std::ofstream &csv, std::ofstream &log);

#endif /* Sharded_POW_Experiments_hpp */
