//
// Created by khood on 10/14/19.
//

#include "SmartShards_Experiments.h"

void ChurnRateVsQuorumIntersection(std::ofstream &summary, std::ofstream &log){

    int quorumIntersection = 1;
    summary << "Quorum Intersection 1"<< std::endl;
    summary << "confirmations, total dead peers, churnRate" << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (0.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (1.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (2.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (3.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (4.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (5.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (6.0 / 5), NUMBER_OF_HALTS, log) << std::endl;

    quorumIntersection = 2;
    summary << "Quorum Intersection 2"<< std::endl;
    summary << "confirmations, total dead peers, churnRate" << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (0.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (1.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (2.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (3.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (4.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (5.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (6.0 / 5), NUMBER_OF_HALTS, log) << std::endl;

    quorumIntersection = 3;
    summary << "Quorum Intersection 3"<< std::endl;
    summary << "confirmations, total dead peers, churnRate" << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (0.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (1.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (2.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (3.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (4.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (5.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (6.0 / 5), NUMBER_OF_HALTS, log) << std::endl;

    quorumIntersection = 4;
    summary << "Quorum Intersection 4"<< std::endl;
    summary << "confirmations, total dead peers, churnRate" << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (0.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (1.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (2.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (3.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (4.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (5.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (6.0 / 5), NUMBER_OF_HALTS, log) << std::endl;

    quorumIntersection = 5;
    summary << "Quorum Intersection 5"<< std::endl;
    summary << "confirmations, total dead peers, churnRate" << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (0.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (1.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (2.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (3.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (4.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (5.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (6.0 / 5), NUMBER_OF_HALTS, log) << std::endl;

    quorumIntersection = 6;
    summary << "Quorum Intersection 6"<< std::endl;
    summary << "confirmations, total dead peers, churnRate" << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (0.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (1.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (2.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (3.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (4.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (5.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (6.0 / 5), NUMBER_OF_HALTS, log) << std::endl;

    quorumIntersection = 7;
    summary << "Quorum Intersection 7"<< std::endl;
    summary << "confirmations, total dead peers, churnRate" << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (0.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (1.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (2.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (3.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (4.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (5.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, NUMBER_OF_HALTS * (6.0 / 5), NUMBER_OF_HALTS, log) << std::endl;
}

std::string intersectionWithHaltsAndJoins(int quorumIntersection, int numberOfJoins, int numberOfHalts, std::ofstream &log){

    int peersPerShard = -1; // not specified based on number of quorumIntersection
    int totalConfirmations = 0;
    int totalDeadPeers = 0;

    std::deque<int> joinRounds = scheduleEvents(numberOfJoins);
    std::deque<int> haltRounds = scheduleEvents(numberOfHalts);

    // --------------------------- number of experiments --------------------------- //
    for(int run = 0; run < NUMBER_OF_RUNS; run++) {
        SmartShard system(NUMBER_OF_SHARDS, log, MAX_DELAY, peersPerShard, RESERVE_SIZE, quorumIntersection);
        system.setFaultTolerance(FAULT);
        system.setRequestsPerRound(NUMBER_OF_REQUEST); // number of requests to make at one time
        system.setRoundsToRequest(ROUNDS_TO_MAKE_REQUEST); // number of times to make a request
        system.setMaxWait();

        // --------------------------- number of rounds ------------------------------- //
        for (int round = 0; round < NUMBER_OF_ROUNDS; round++) {
            if(joinRounds.front() == round){
                joinRounds.pop_front();
                system.revivePeer();
            }
            if(haltRounds.front() == round){
                haltRounds.pop_front();
                system.dropPeer();
            }

            for (auto e : system.getQuorums()) {
                for (int j = 0; j < system.getShardSize(); ++j)
                    (*e)[j]->makeRequest(); // does both send and preformComputation
            }
            for (auto e: system.getQuorums()) {
                for (int j = 0; j < system.getShardSize(); ++j)
                    (*e)[j]->transmit();
                e->log();
            }
            totalDeadPeers += system.getByzantine();
        }

        totalConfirmations += system.getConfirmationCount();
    }
    return std::to_string(totalConfirmations/NUMBER_OF_RUNS) + ", " + std::to_string(totalDeadPeers / (NUMBER_OF_ROUNDS * NUMBER_OF_RUNS)) + ", " + std::to_string(numberOfJoins) + ":" + std::to_string(numberOfHalts);
}

std::deque<int> scheduleEvents(int numberOfEvents){
    std::deque<int> schedule = std::deque<int>();
    std::uniform_int_distribution<int> randomDistribution(1,NUMBER_OF_ROUNDS);

    for(int i = 0; i < numberOfEvents; i++) {
        int randomRound = 0;
        do {
            randomRound = randomDistribution(RANDOM_GENERATOR);
        } while (std::find(schedule.begin(), schedule.end(), randomRound) != schedule.end());
        schedule.push_back(randomRound);
    }

    std::sort(schedule.begin(), schedule.end());

    return schedule;
}