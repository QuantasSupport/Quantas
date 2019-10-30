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

    int numberOfShards = MAX_NUMBER_OF_SHARDS;
    summary << "5 Quorum, quorum Intersection " << std::endl;
    summary << "confirmations, total dead peers, churnRate, Intersection" << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (0.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (1.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (2.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (3.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (4.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (5.0 / 5), PEER_COUNT, log) << std::endl;
    


    numberOfShards++;
    summary << "6 Quorum, quorum Intersection " << std::endl;
    summary << "confirmations, total dead peers, churnRate, Intersection" << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (0.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (1.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (2.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (3.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (4.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (5.0 / 5), PEER_COUNT, log) << std::endl;
    

    numberOfShards++;
    summary << "7 Quorum, quorum Intersection " << std::endl;
    summary << "confirmations, total dead peers, churnRate, Intersection" << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (0.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (1.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (2.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (3.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (4.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (5.0 / 5), PEER_COUNT, log) << std::endl;
    

    numberOfShards++;
    summary << "8 Quorum, quorum Intersection " << std::endl;
    summary << "confirmations, total dead peers, churnRate, Intersection" << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (0.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (1.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (2.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (3.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (4.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (5.0 / 5), PEER_COUNT, log) << std::endl;
    

    numberOfShards++;
    summary << "9 Quorum, quorum Intersection " << std::endl;
    summary << "confirmations, total dead peers, churnRate, Intersection" << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (0.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (1.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (2.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (3.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (4.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (5.0 / 5), PEER_COUNT, log) << std::endl;
    

    numberOfShards++;
    summary << "10 Quorum, quorum Intersection " << std::endl;
    summary << "confirmations, total dead peers, churnRate, Intersection" << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (0.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (1.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (2.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (3.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (4.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (5.0 / 5), PEER_COUNT, log) << std::endl;
    

    numberOfShards++;
    summary << "11 Quorum, quorum Intersection " << std::endl;
    summary << "confirmations, total dead peers, churnRate, Intersection" << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (0.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (1.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (2.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (3.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (4.0 / 5), PEER_COUNT, log) << std::endl;
    summary << intersectionWithHaltsAndJoins(numberOfShards, PEER_COUNT * (5.0 / 5), PEER_COUNT, log) << std::endl;
    
}

std::string intersectionWithHaltsAndJoins(int numberOfShards, int numberOfJoins, int numberOfHalts, std::ofstream &log){
    /*
     * P = Number of Peers
     * S = Number of Shards
     * I = Number of Intersections
     * P = I*(S(S-1))/2
     * I = 2P/S(S-1)
     */
    int quorumIntersection = PEER_COUNT/((numberOfShards*(numberOfShards-1))/2); // I
    int peerPerShard = quorumIntersection*(numberOfShards-1); // -1 for self
    if(peerPerShard < 2){return "";}
    int totalConfirmations = 0;
    int totalDeadPeers = 0;

    std::deque<int> joinRounds = scheduleEvents(numberOfJoins);
    std::deque<int> haltRounds = scheduleEvents(numberOfHalts);

    // --------------------------- number of experiments --------------------------- //
    for(int run = 0; run < NUMBER_OF_RUNS; run++) {
        SmartShard system(numberOfShards, log, MAX_DELAY, peerPerShard, RESERVE_SIZE, quorumIntersection);
        system.setFaultTolerance(FAULT);
        system.setRequestsPerRound(NUMBER_OF_REQUEST); // number of requests to make at one time
        system.setRoundsToRequest(ROUNDS_TO_MAKE_REQUEST); // number of times to make a request
        system.setMaxWait();
        std::cout<< "Size:"<< system.size()<< std::endl;

        // --------------------------- number of rounds ------------------------------- //
        for (int round = 0; round < NUMBER_OF_ROUNDS; round++) {
            std::cout << "Round:" << round << std::endl;
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
                    (*e)[j]->makeRequest(); // does both receive and preformComputation
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
    return std::to_string(totalConfirmations/ (NUMBER_OF_RUNS * NUMBER_OF_ROUNDS)) +
    ", " + std::to_string(totalDeadPeers / (NUMBER_OF_ROUNDS * NUMBER_OF_RUNS)) +
    ", " + std::to_string(numberOfJoins) + ":" + std::to_string(numberOfHalts) +
    ", " + std::to_string(quorumIntersection);
}

std::deque<int> scheduleEvents(int numberOfEvents){
    assert(numberOfEvents <= NUMBER_OF_ROUNDS);
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