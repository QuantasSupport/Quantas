//
// Created by khood on 10/14/19.
//

#include "SmartShards_Experiments.h"

void ChurnRateVsQuorumIntersection(std::ofstream &summary, std::ofstream &log){

    /* P = Number of Peers
     * S = Number of Shards
     * I = Number of Intersections
     * P = I*(S(S-1))/2
     * I = 2P/S(S-1)
     */

    int quorumIntersection = 1;
    /* P = I*(S(S-1))/2
     * P = 1*(5(4))/2
     * P = 20/2
     * P = 10
     */
    int numberOfPeers = 10;

    summary << "Quorum Intersection 1"<< std::endl;
    summary << "confirmations, total dead peers, churnRate" << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (0.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (1.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (2.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (3.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (4.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (5.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (6.0 / 5), numberOfPeers, log) << std::endl;

    quorumIntersection = 2;
    /* P = I*(S(S-1))/2
     * P = 2*(5(4))/2
     * P = 40/2
     * P = 20
     */
    numberOfPeers = 20;

    summary << "Quorum Intersection 2"<< std::endl;
    summary << "confirmations, total dead peers, churnRate" << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (0.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (1.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (2.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (3.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (4.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (5.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (6.0 / 5), numberOfPeers, log) << std::endl;

    quorumIntersection = 3;
    /* P = I*(S(S-1))/2
     * P = 3*(5(4))/2
     * P = 60/2
     * P = 30
     */
    numberOfPeers = 30;

    summary << "Quorum Intersection 3"<< std::endl;
    summary << "confirmations, total dead peers, churnRate" << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (0.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (1.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (2.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (3.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (4.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (5.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (6.0 / 5), numberOfPeers, log) << std::endl;

    quorumIntersection = 4;
    /* P = I*(S(S-1))/2
     * P = 4*(5(4))/2
     * P = 80/2
     * P = 40
     */
    numberOfPeers = 40;

    summary << "Quorum Intersection 4"<< std::endl;
    summary << "confirmations, total dead peers, churnRate" << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (0.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (1.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (2.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (3.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (4.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (5.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (6.0 / 5), numberOfPeers, log) << std::endl;

    quorumIntersection = 5;
    /* P = I*(S(S-1))/2
     * P = 5*(5(4))/2
     * P = 100/2
     * P = 50
     */
    numberOfPeers = 50;

    summary << "Quorum Intersection 5"<< std::endl;
    summary << "confirmations, total dead peers, churnRate" << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (0.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (1.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (2.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (3.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (4.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (5.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (6.0 / 5), numberOfPeers, log) << std::endl;

    quorumIntersection = 6;
    /* P = I*(S(S-1))/2
     * P = 6*(5(4))/2
     * P = 120/2
     * P = 60
     */
    numberOfPeers = 60;

    summary << "Quorum Intersection 6"<< std::endl;
    summary << "confirmations, total dead peers, churnRate" << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (0.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (1.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (2.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (3.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (4.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (5.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (6.0 / 5), numberOfPeers, log) << std::endl;

    quorumIntersection = 7;
    /* P = I*(S(S-1))/2
     * P = 7*(5(4))/2
     * P = 140/2
     * P = 70
     */
    numberOfPeers = 70;

    summary << "Quorum Intersection 7"<< std::endl;
    summary << "confirmations, total dead peers, churnRate" << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (0.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (1.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (2.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (3.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (4.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (5.0 / 5), numberOfPeers, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(quorumIntersection, numberOfPeers * (6.0 / 5), numberOfPeers, log) << std::endl;
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
        std::cout<< "Size:"<< system.size()<< std::endl;

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