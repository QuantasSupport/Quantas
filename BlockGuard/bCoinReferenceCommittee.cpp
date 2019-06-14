//
//  bCoinReferenceCommittee.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 6/13/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "bCoinReferenceCommittee.hpp"

void bCoinReferenceCommittee::initNetwork(int numberOfPeers){
    _peers.initNetwork(numberOfPeers);
    
    _securityLevel5 = numberOfPeers/_groupSize;
    _securityLevel4 = _securityLevel5/2;
    _securityLevel3 = _securityLevel4/2;
    _securityLevel2 = _securityLevel3/2;
    _securityLevel1 = _securityLevel2/2;
    
}

std::vector<bCoinGroup> bCoinReferenceCommittee::getFreeGroups(){
    std::vector<bCoinGroup> freeGroups = std::vector<bCoinGroup>();
    for(auto g = _groups.begin(); g != _groups.end(); g++){
        bool free = true;
        bCoinGroup group = g->second;
        for(auto peer = group.begin(); peer != group.end(); peer++){
            if(!(*peer)->isTerminated()){
                free = false;
            }
        }
        if(free){
            freeGroups.push_back(group);
        }
    }
    
    return freeGroups;
}

void bCoinReferenceCommittee::makeRequest(transactionRequest){
    std::uniform_int_distribution<int> coin(0,1);
    int trails = 0;
    int heads = coin(_randomGenerator);
    while(!heads){
        trails++;
        heads = coin(_randomGenerator);
    }
    
    int secLevel = -1;
    int freeGroup = getFreeGroups().size();
    
    switch (trails) {
        case 0: secLevel = _securityLevel1; break;
        case 1: secLevel = _securityLevel2; break;
        case 2: secLevel = _securityLevel3; break;
        case 3: secLevel = _securityLevel4; break;
        case 4: secLevel = _securityLevel5; break;
            
        default: secLevel = _securityLevel5; break;
    }
    transactionRequest req = transactionRequest();
    

    req.securityLevel = secLevel;
    req.id = _nextId;
    _nextId++;
    _requestQueue.push_back(req);
    req = _requestQueue.front();
    _requestQueue.pop_front();
    
    if(freeGroup < req.securityLevel){
        return;
    }
    
    vector<bCoinGroup> groupsForCommittee = vector<bCoinGroup>();
    
    
    
}
