//
//  refComExperiments.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 7/9/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef refComExperiments_hpp
#define refComExperiments_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <set>
#include <iostream>
#include <chrono>
#include <random>

// UTIL
#include "Sharded_PBFT_Experiments.hpp"
#include "Sharded_POW_Experiments.hpp"
#include "Sharded_SBFT_Experiments.hpp"

void SBFT_refCom(std::string filePath);
void PBFT_refCom(std::string filePath);
void POW_refCom(std::string filePath);

#endif /* refComExperiments_hpp */
