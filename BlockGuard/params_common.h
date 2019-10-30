//
// Created by khood on 10/14/19.
//

#ifndef DISTRIBUTED_CONSENSUS_ABSTRACT_SIMULATOR_PARAMS_COMMON_H
#define DISTRIBUTED_CONSENSUS_ABSTRACT_SIMULATOR_PARAMS_COMMON_H

static const int PEER_COUNT = 100;  // 1024
static const double FAULT = 0.333334;   // number of peers that need to be honest for PBFT to work (i.e. 2/3s)
static const int NUMBER_OF_ROUNDS = 1000;
static const int NUMBER_OF_RUNS = 10;

#endif //DISTRIBUTED_CONSENSUS_ABSTRACT_SIMULATOR_PARAMS_COMMON_H
