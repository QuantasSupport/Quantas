//
// Created by khood on 10/14/19.
//

#ifndef DISTRIBUTED_CONSENSUS_ABSTRACT_SIMULATOR_SMARTSHARDS_EXPERIMENTS_H
#define DISTRIBUTED_CONSENSUS_ABSTRACT_SIMULATOR_SMARTSHARDS_EXPERIMENTS_H

#include <iostream>
#include <fstream>
#include <set>
#include <chrono>
#include <random>
#include <string>
#include <deque>

#include "./../MarkPBFT_peer.hpp"
#include "./../SmartShard.hpp"
#include "./../params_SmartShards.h"
#include "./../params_Blockguard.hpp"

void ChurnRateVsQuorumIntersection(std::ofstream &csv, std::ofstream &log);

// intersection runs
std::string intersectionWithHaltsAndJoins(int numberOfShards, int numberOfJoins, int numberOfHalts, std::ofstream &log);

// util
// returns a list of rounds for for some event to happen, each event gets it's own round
std::deque<int> scheduleEvents(int numberOfEvents);

#endif //DISTRIBUTED_CONSENSUS_ABSTRACT_SIMULATOR_SMARTSHARDS_EXPERIMENTS_H
