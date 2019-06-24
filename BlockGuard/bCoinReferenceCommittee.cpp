//
//  bCoinReferenceCommittee.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 6/13/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "bCoinReferenceCommittee.hpp"

bCoinReferenceCommittee::bCoinReferenceCommittee (){
    _securityLevel5 = -1;
    _securityLevel4 = -1;
    _securityLevel3 = -1;
    _securityLevel2 = -1;
    _securityLevel1 = -1;
    
    int seed = (int)time(nullptr);
    _randomGenerator = std::default_random_engine(seed);
    _groupSize = -1;
    _nextId = -1;
    _peers = ByzantineNetwork<DS_bCoinMessage, DS_bCoin_Peer>();
    _groups = std::map<int,bCoinGroup>();
    _requestQueue = std::deque<bCoinTransactionRequest>();
    _currentCommittees = std::vector<bCoin_Committee>();
    

    _log = nullptr;
    _printNetwork = false;
    _secLevel5Defeated = -1;
    _secLevel4Defeated = -1;
    _secLevel3Defeated = -1;
    _secLevel2Defeated = -1;
    _secLevel1Defeated = -1;
    _totalSubmitoins = 0;
}

bCoinReferenceCommittee::bCoinReferenceCommittee (const bCoinReferenceCommittee &rhs){
    _securityLevel5 = rhs._securityLevel5;
    _securityLevel4 = rhs._securityLevel4;
    _securityLevel3 = rhs._securityLevel3;
    _securityLevel2 = rhs._securityLevel2;
    _securityLevel1 = rhs._securityLevel1;
    
    int seed = (int)time(nullptr);
    _randomGenerator = std::default_random_engine(seed);
    _groupSize = rhs._groupSize;
    _nextId = rhs._nextId;
    _peers = rhs._peers;
    _groups = rhs._groups;
    _requestQueue = rhs._requestQueue;
    _currentCommittees = rhs._currentCommittees;
    
    
    _log = rhs._log;
    _printNetwork = rhs._printNetwork;
    _secLevel5Defeated = rhs._secLevel5Defeated;
    _secLevel4Defeated = rhs._secLevel4Defeated;
    _secLevel3Defeated = rhs._secLevel3Defeated;
    _secLevel2Defeated = rhs._secLevel2Defeated;
    _secLevel1Defeated = rhs._secLevel1Defeated;
    _totalSubmitoins = rhs._totalSubmitoins;
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
            if((*peer)->isTerminated() || (*peer)->isBusy()){
                free = false;
            }
        }
        if(free){
            freeGroups.push_back(group);
        }
    }
    
    return freeGroups;
}

int bCoinReferenceCommittee::getRandomSecLevel(){
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

void bCoinReferenceCommittee::makeRequest(int secLevel){
    bCoinTransactionRequest req = bCoinTransactionRequest();
    req.securityLevel = secLevel == -1 ? getRandomSecLevel() : secLevel;
    req.id = _nextId;
    _nextId++;
    _requestQueue.push_back(req); // add to queue
    _totalSubmitoins++;
    
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
            group[i]->setBusy(true);
        }
        // remove groups from list of free groups
        freeGroups.pop_back();
    }
    
    // add new committee to list
    bCoin_Committee committee = bCoin_Committee(peersForCommittee,peersForCommittee[0], std::to_string(req.id), req.securityLevel);
    for(int i = 0; i<committee.size(); i++){
        committee[i]->resetMiningClock();
    }


    if(committee.getByzantineRatio() > 0.5){
                 if (req.securityLevel == _securityLevel1){_secLevel1Defeated++;}
            else if (req.securityLevel == _securityLevel2){_secLevel2Defeated++;}
            else if (req.securityLevel == _securityLevel3){_secLevel3Defeated++;}
            else if (req.securityLevel == _securityLevel4){_secLevel4Defeated++;}
            else if (req.securityLevel == _securityLevel5){_secLevel5Defeated++;}
            else                                          {assert(false);}
        }
    committee.initiate();
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
    cleanupCommittee();
    for(auto committee = _currentCommittees.begin(); committee != _currentCommittees.end(); committee++){
        committee->preformComputation();
    }
};

void bCoinReferenceCommittee::cleanupCommittee(){
    // clear old committees that have terminated and release peers from them
    auto committee = _currentCommittees.begin();
    while(committee != _currentCommittees.end()){
        if(committee->checkForConsensus() == true){
            for(int i = 0; i < committee->size(); i++){
                committee->get(i)->updateDAG();
            }
            for(int i = 0; i < committee->size(); i++){
                committee->get(i)->setTerminated(false);
                committee->get(i)->setBusy(false);
            }
            _currentCommittees.erase(committee);
        }else{
            committee++;
        }
    }
}

void bCoinReferenceCommittee::transmit(){
    for(auto committee = _currentCommittees.begin(); committee != _currentCommittees.end(); committee++){
        committee->transmit();
    }
};

std::vector<DAGBlock> bCoinReferenceCommittee::getGlobalLedger()const{
    std::vector<DAGBlock> transactions = std::vector<DAGBlock>();
    for(int i = 0; i < _peers.size(); i++){
        std::vector<DAGBlock> transactionsInPeer = _peers[i]->getDAG().getTransactions();
        for(auto tx = transactionsInPeer.begin(); tx != transactionsInPeer.end(); tx++){
            bool found = false;
            for(auto prvTx = transactions.begin(); prvTx != transactions.end(); prvTx++){
                if(prvTx->getData() == tx->getData()){
                    found = true;
                }
            }
            if(!found){
                transactions.push_back(*tx);
            }
        }
    }
    return transactions;
}

bCoinReferenceCommittee& bCoinReferenceCommittee::operator=(const bCoinReferenceCommittee &rhs){
    if(this == &rhs){
        return *this;
    }
    
    _securityLevel5 = rhs._securityLevel5;
    _securityLevel4 = rhs._securityLevel4;
    _securityLevel3 = rhs._securityLevel3;
    _securityLevel2 = rhs._securityLevel2;
    _securityLevel1 = rhs._securityLevel1;
    
    int seed = (int)time(nullptr);
    _randomGenerator = std::default_random_engine(seed);
    _groupSize = rhs._groupSize;
    _nextId = rhs._nextId;
    _peers = rhs._peers;
    _groups = rhs._groups;
    _requestQueue = rhs._requestQueue;
    _currentCommittees = rhs._currentCommittees;
    
    
    _log = rhs._log;
    _printNetwork = rhs._printNetwork;
    _secLevel5Defeated = rhs._secLevel5Defeated;
    _secLevel4Defeated = rhs._secLevel4Defeated;
    _secLevel3Defeated = rhs._secLevel3Defeated;
    _secLevel2Defeated = rhs._secLevel2Defeated;
    _secLevel1Defeated = rhs._secLevel1Defeated;
    _totalSubmitoins = rhs._totalSubmitoins;
    
    return *this;
}

std::ostream& bCoinReferenceCommittee::printTo(std::ostream &out)const{
    out<< "-- REFERENCE COMMITTEE SETUP --"<< std::endl<< std::endl;
    out<< std::left;
    out<< '\t'<< std::setw(LOG_WIDTH)<<  "Request Queue Size"   << std::setw(LOG_WIDTH)<< "Next Committee Id"   << std::endl;
    out<< '\t'<< std::setw(LOG_WIDTH)<< _requestQueue.size()    << std::setw(LOG_WIDTH)<< _nextId               << std::endl;
    out<<std::endl;
    if(_printNetwork){
        _peers.printTo(out);
    }
    
    return out;
}
