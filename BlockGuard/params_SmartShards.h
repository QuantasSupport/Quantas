//
// Created by khood on 10/14/19.
//

#ifndef DISTRIBUTED_CONSENSUS_ABSTRACT_SIMULATOR_PARAMS_SMARTSHARDS_H
#define DISTRIBUTED_CONSENSUS_ABSTRACT_SIMULATOR_PARAMS_SMARTSHARDS_H

#include "params_common.h"

static const int MAX_DELAY = 1;
static const int MAX_NUMBER_OF_SHARDS = 5;
//static const int NUMBER_OF_HALTS = 100;
static const int RESERVE_SIZE = double(PEER_COUNT)*0.2;
static const int NUMBER_OF_REQUEST = 1;
static const int ROUNDS_TO_MAKE_REQUEST = 1; // every this many rounds make NUMBER_OF_REQUEST

#endif //DISTRIBUTED_CONSENSUS_ABSTRACT_SIMULATOR_PARAMS_SMARTSHARDS_H
