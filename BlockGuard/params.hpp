//
//  params.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 6/20/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef params_hpp
#define params_hpp

#include <stdio.h>

static const int PEER_COUNT = 256; // need to go to 1000
static const int GROUP_SIZE = 8;   // Fixed only
static const double FAULT = 0.6;
static const int NUMBER_OF_BYZ =  PEER_COUNT * 0.29; // 1/3
static const int NUMBER_OF_ROUNDS = 1000; 
static const int NUMBER_OF_RUNS = 2;

#endif /* params_hpp */
