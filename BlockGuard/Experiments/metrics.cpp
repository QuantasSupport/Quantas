//
//  metrics.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 7/9/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "metrics.hpp"

double ratioOfSecLvl(std::vector<DAGBlock> globalLedger, double secLvl){
    double total = globalLedger.size();
    double defeated = 0;
    for(auto entry = globalLedger.begin(); entry != globalLedger.end(); entry++){
        if(entry->getSecruityLevel() == secLvl*GROUP_SIZE){
            if(entry->isByzantine()){
                defeated++;
            }
        }
    }
    
    return defeated/total;
}

double waitTimeOfSecLvl(std::vector<DAGBlock> globalLedger, double secLvl){
    double sumOfWaitingTime = 0;
    double totalNumberOfTrnasactions = 0;
    for(auto entry = globalLedger.begin(); entry != globalLedger.end(); entry++){
        if(entry->getSecruityLevel() == secLvl*GROUP_SIZE){
            sumOfWaitingTime += entry->getConfirmedRound() - entry->getSubmissionRound();
            totalNumberOfTrnasactions++;
        }
    }
    if(totalNumberOfTrnasactions == 0){
        return 0;
    }else{
        return sumOfWaitingTime/totalNumberOfTrnasactions;
    }
}

double waitTime(std::vector<DAGBlock> globalLedger){
    double sumOfWaitingTime = 0;
    double totalNumberOfTrnasactions = 0;
    for(auto entry = globalLedger.begin(); entry != globalLedger.end(); entry++){
        sumOfWaitingTime += entry->getConfirmedRound() - entry->getSubmissionRound();
        totalNumberOfTrnasactions++;
    }
    if(totalNumberOfTrnasactions == 0){
        return 0;
    }else{
        return sumOfWaitingTime/totalNumberOfTrnasactions;
    }
}

double waitTimeRolling(std::vector<DAGBlock> globalLedger, int fromRound){
    if(globalLedger.size() == 0){
        return -1;
    }
    
    double averageWaitTime = 0;
    double sumOfWaitingTime = 0;
    double totalNumberOfTrnasactions = 0;
    for(auto entry = globalLedger.begin(); entry != globalLedger.end(); entry++){
        if(entry->getConfirmedRound() >= fromRound){
            sumOfWaitingTime += entry->getConfirmedRound() - entry->getSubmissionRound();
            totalNumberOfTrnasactions++;
        }
    }
    averageWaitTime = sumOfWaitingTime/totalNumberOfTrnasactions;
    
    return averageWaitTime;
}

int totalNumberOfDefeatedCommittees(std::vector<DAGBlock> globalLedger, double secLvl){
    int total = 0;
    for(auto entry = globalLedger.begin(); entry != globalLedger.end(); entry++){
        if(entry->getSecruityLevel() == secLvl*GROUP_SIZE){
            if(entry->isByzantine()){
                total++;
            }
        }
    }
    return total;
}

int defeatedTrnasactions(std::vector<DAGBlock> globalLedger){
    int defeated = 0;
    
    for(auto entry = globalLedger.begin(); entry != globalLedger.end(); entry++){
        if(entry->isByzantine()){
            defeated++;
        }
    }
    
    return defeated;
}

int totalNumberOfCorrectCommittees(std::vector<DAGBlock> globalLedger, double secLvl){
    int total = 0;
    for(auto entry = globalLedger.begin(); entry != globalLedger.end(); entry++){
        if(entry->getSecruityLevel() == secLvl*GROUP_SIZE){
            if(!entry->isByzantine()){
                total++;
            }
        }
    }
    return total;
}
