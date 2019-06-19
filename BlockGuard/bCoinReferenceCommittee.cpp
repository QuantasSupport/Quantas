//
//  bCoinReferenceCommittee.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 6/13/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "bCoinReferenceCommittee.hpp"

bCoinReferenceCommittee::bCoinReferenceCommittee (){
     _securityLevel5;
     _securityLevel4;
     _securityLevel3;
     _securityLevel2;
     _securityLevel1;
    
    int seed = (int)time(nullptr);
    _randomGenerator = std::default_random_engine(seed);
                                                 _groupSize = -1;
                                               _nextId = -1;
        _peers = ByzantineNetwork<DS_bCoinMessage, DS_bCoin_Peer>();
                                _groups = std::map<int,bCoinGroup>();
    _requestQueue = std::deque<transactionRequest>();;
                            _currentCommittees = std::vector<bCoin_Committee>();
    

    std::ostream                                        *_log;
                                               _printNetwork;
                                                _secLevel5Defeated;
                                                _secLevel4Defeated;
                                                _secLevel3Defeated;
                                                _secLevel2Defeated;
                                                _secLevel1Defeated;
}

bCoinReferenceCommittee::bCoinReferenceCommittee (const bCoinReferenceCommittee&){
    
}

void bCoinReferenceCommittee::initNetwork(int numberOfPeers){
    _peers.initNetwork(numberOfPeers);
    
    _securityLevel5 = numberOfPeers/_groupSize;
    _securityLevel4 = _securityLevel5/2;
    _securityLevel3 = _securityLevel4/2;
    _securityLevel2 = _securityLevel3/2;
    _securityLevel1 = _securityLevel2/2;
    
    // form groups
    bCoinGroup group = bCoinGroup();
    int groupID = 0;
    for(int i = 0; i < _peers.size(); i++){
        // if we have seen enough peers add a new group
        if(i%_groupSize == 0 && i != 0){
            _groups[groupID] = group;
            groupID++;
            group = bCoinGroup(); // clear for next group
        }
        
        group.push_back(_peers[i]); // add peer to group
    }
    
    // if there was not enough peers for the last group just make a group that is slightly smaller
    if(!group.empty()){
        _groups[groupID] = group;
    }
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

int bCoinReferenceCommittee::getRandomSecLevel()const{
    std::uniform_int_distribution<int> coin(0,1);
    int trails = 0;
    int heads = coin(_randomGenerator);
    while(!heads){
        trails++;
        heads = coin(_randomGenerator);
    }
    
    int secLevel = -1;
    switch (trails) {
        case 0: secLevel = _securityLevel1; break;
        case 1: secLevel = _securityLevel2; break;
        case 2: secLevel = _securityLevel3; break;
        case 3: secLevel = _securityLevel4; break;
        case 4: secLevel = _securityLevel5; break;
            
        default: secLevel = _securityLevel5; break;
    }
    return secLevel;
}

void bCoinReferenceCommittee::makeRequest(transactionRequest){
    // make a new transaction
    int secLevel = getRandomSecLevel();
    transactionRequest req = transactionRequest();
    req.securityLevel = secLevel;
    req.id = _nextId;
    _nextId++;
    _requestQueue.push_back(req); // add to queue
    
    // get front of queue not necessarily the transaction we just made
    req = _requestQueue.front();
    int freeGroup = getFreeGroups().size();
    if(freeGroup < req.securityLevel){
        return; // if we do not have enough free groups level that transaction in queue and do nothing
    }
    
    _requestQueue.pop_front();
    vector<DS_bCoin_Peer*> peersForCommittee = vector<DS_bCoin_Peer*>();
    vector<bCoinGroup> freeGroups = getFreeGroups();
    while((peersForCommittee.size()/_groupSize) < req.securityLevel){
        bCoinGroup group = getFreeGroups().back();
        // add members
        for(int i = 0; i < group.size(); i++){
            peersForCommittee.push_back(group[i]);
        }
        // remove groups from list of free groups
        freeGroups.pop_back();
    }
    
    // add new committee to list
    bCoin_Committee committee = bCoin_Committee(peersForCommittee,peersForCommittee[0], std::to_string(req.id), req.securityLevel);
    committee.initiate();
    if(committee.getByzantineRatio() > 0.5){
        switch (secLevel) {
            case 1:{
                _secLevel1Defeated++;
                break;
            }
            case 2:{
                _secLevel2Defeated++;
                break;
            }
            case 3:{
                _secLevel3Defeated++;
                break;
            }
            case 4:{
                _secLevel4Defeated++;
                break;
            }
            case 5:{
                _secLevel5Defeated++;
                break;
            }
                
            default:{
                assert(false);
            }
        }
    }
    _currentCommittees.push_back(committee);
}

void bCoinReferenceCommittee::shuffleByzantines(int n){
    std::vector<int> correct = std::vector<int>();
    std::vector<int> byz = std::vector<int>();
    // count total byz and correct peers
    for(int i = 0; i < _peers.size(); i++){
        if(!_peers[i]->isTerminated()){
            if(_peers[i]->isByzantine()){
                byz.push_back(i);
            }else{
                correct.push_back(i);
            }
        }
    }
    
    if(correct.size() < n){
        n = correct.size();
    }
    if(byz.size() < n){
        n = byz.size();
    }
    
    int shuffleCount = 0;
    while(shuffleCount < n){
        int byzantineShuffleIndex = static_cast<int>(rand() % byz.size());
        int nonByzantineShuffleIndex = static_cast<int>(rand() % correct.size());
        _peers[byz[byzantineShuffleIndex]]->makeCorrect();
        _peers[correct[nonByzantineShuffleIndex]]->makeByzantine();
        shuffleCount++;
        byz.erase(byz.begin () + byzantineShuffleIndex);
        correct.erase(correct.begin () + nonByzantineShuffleIndex);
    }
}

void bCoinReferenceCommittee::receive(){
    for(auto committee = _currentCommittees.begin(); committee != _currentCommittees.end(); committee++){
        committee->receive();
    }
};
void bCoinReferenceCommittee::preformComputation(){
    for(auto committee = _currentCommittees.begin(); committee != _currentCommittees.end(); committee++){
        committee->preformComputation();
    }
};
void bCoinReferenceCommittee::transmit(){
    for(auto committee = _currentCommittees.begin(); committee != _currentCommittees.end(); committee++){
        committee->transmit();
    }
};

std::vector<DAGBlock> bCoinReferenceCommittee::getGlobalLedger(){
    std::vector<DAGBlock> transactions = std::vector<DAGBlock>();
    for(int i = 0; i < _peers.size(); i++){
        //std::vector<DAGBlock> transactionsInPeer = _peers[i]->getDAG().getTransactions();
        //transactions.insert(transactions.end(), transactionsInPeer.begin(), transactionsInPeer.end());
    }
    return transactions;
}
