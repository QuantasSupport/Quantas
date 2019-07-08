//
//  syncBFTReferenceCommittee.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 7/1/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "SBFTReferenceCommittee.hpp"


void SBFTReferenceCommittee::preformComputation(){
    cleanupCommittee();
    for(auto committee = _activeCommittees.begin(); committee != _activeCommittees.end(); committee++){
        committee->preformComputation();
    }
    
    if(_status == WAITING_FOR_TX){
        //            wait for max delay until all committees receive their transactions
        if(_clock%_waitTime ==  0){
            for(auto & currentCommittee : _activeCommittees) {
                currentCommittee.receiveTx();
            }
            _status = MINING;            }
        
    }else if(_status == MINING){
        for(auto & currentCommittee : _activeCommittees){
            if(currentCommittee.getConsensusFlag()){
                continue;
            }
            currentCommittee.preformComputation();
            if(_clock%_waitTime ==  0){
                currentCommittee.nextState(0, _peers.maxDelay());
            }
        }
        
        //    don't erase but check to see if consensus reached in all
        bool consensus = true;
        auto it = _activeCommittees.begin();
        while(it != _activeCommittees.end()) {
            if(!it->getConsensusFlag()) {
                consensus = false;
                break;
            }
            ++it;
        }
        if (consensus){
            for(auto committee: _activeCommittees ){
                committee.refreshPeers();
            }
            _activeCommittees.clear();
            
            _status =COLLECTING;
        }
    }else if(_status == COLLECTING){
        //    make sure no peer is busy
        for(int a = 0; a< _peers.size();a++){
            assert(_peers[a]->isTerminated());
            assert(_peers[a]->getConsensusTx().empty());
        }
        assert (_activeCommittees.empty());
        
        if(_clock%_waitTime == 0){
            for(int index = 0; index< _peers.size();index++){
                _peers[index]->updateDAG();
            }
        }
        _status = WAITING_FOR_TX;
    }
}

void SBFTReferenceCommittee::receive(){
    _clock++;
    for(auto committee = _activeCommittees.begin(); committee != _activeCommittees.end(); committee++){
        committee->receive();
    }
}

void SBFTReferenceCommittee::transmit(){
    for(auto committee = _activeCommittees.begin(); committee != _activeCommittees.end(); committee++){
        committee->transmit();
    }
}

int SBFTReferenceCommittee::getRandomSecLevel(){
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

std::vector<SBFTGroup> SBFTReferenceCommittee::getFreeGroups(){
    std::vector<SBFTGroup> freeGroups = std::vector<SBFTGroup>();
    // look at each id,group pair
    for(auto id = _groups.begin(); id != _groups.end(); id++){
        SBFTGroup group = _groups[id->first]; // get the group by id
        bool free = true; // assume the group is free
        for(auto peer = group.begin(); peer != group.end(); peer++){
            if(!(*peer)->isTerminated()){
                free = false; // if any of the peers has not terminated then the group is not free
            }
        }
        if(free){
            freeGroups.push_back(group); // if the group is free add it
        }
    }
    return freeGroups;
}

void SBFTReferenceCommittee::cleanupCommittee(){
    auto committee = _activeCommittees.begin();
    while(committee != _activeCommittees.end()){
        bool committeeDone = true;
        for(int i = 0; i < committee->size(); i++){
            if(!committee->get(i)->isTerminated()){
                committeeDone = false;
            }
        }
        if(committeeDone){
            _activeCommittees.erase(committee);
        }else{
            committee++;
        }
    }
}

void SBFTReferenceCommittee::initNetwork(int numberOfPeers){
    assert(_groupSize > 0);
    
    // assign secLevels
    _securityLevel5 = numberOfPeers/_groupSize;
    _securityLevel4 = _securityLevel5/2;
    _securityLevel3 = _securityLevel4/2;
    _securityLevel2 = _securityLevel3/2;
    _securityLevel1 = _securityLevel2/2;
    
    _peers.initNetwork(numberOfPeers);
    _waitTime = 2*_peers.maxDelay();
    
    // form groups
    SBFTGroup aGroup = SBFTGroup();
    int groupId = 0;
    for(int i = 0; i < _peers.size(); i++){
        // if we have seen enough peers add a new group
        if(i%_groupSize == 0 && i != 0){
            _groups[groupId] = aGroup;
            aGroup = SBFTGroup(); // clear for next group
            groupId++;
        }
        aGroup.push_back(_peers[i]); // add peer to group
    }
    
    // if there was not enough peers for the last group just make a group that is slightly smaller
    if(!aGroup.empty()){
        _groups[groupId] = aGroup;
    }
}

void SBFTReferenceCommittee::makeRequest(int secLevel){
    SBFTTransactionRequest request = SBFTTransactionRequest();
    request.id = _nextId;
    _nextId++;
    request.securityLevel = secLevel == -1 ? getRandomSecLevel() : secLevel;
    _requestQueue.push_back(request);
    request = _requestQueue.front();
    
    if(getFreeGroups().size() < request.securityLevel){
        return ;
    }
    
    std::vector<syncBFT_Peer*> peersInCommittee = std::vector<syncBFT_Peer*>();
    std::vector<SBFTGroup> groups = getFreeGroups();
    while(peersInCommittee.size()/_groupSize < request.securityLevel){
        SBFTGroup nextGroup = groups.back();
        for(auto peer = nextGroup.begin(); peer != nextGroup.end(); peer++){
            peersInCommittee.push_back(*peer);
        }
        groups.pop_back();
    }
    
    syncBFT_Committee newCommittee = syncBFT_Committee(peersInCommittee,peersInCommittee[0],std::to_string(request.id),request.securityLevel);
    newCommittee.refreshPeers();
    newCommittee.initiate();
    _activeCommittees.push_back(newCommittee);
    newCommittee[0]->makeRequest(peersInCommittee,std::to_string(request.id));
}

void SBFTReferenceCommittee::shuffleByzantines(int n){
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

std::vector<DAGBlock> SBFTReferenceCommittee::getGlobalLedger()const{
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

SBFTReferenceCommittee::SBFTReferenceCommittee(){
    _securityLevel5 = -1;
    _securityLevel4 = -1;
    _securityLevel3 = -1;
    _securityLevel2 = -1;
    _securityLevel1 = -1;
    _secLevel5Defeated = 0;
    _secLevel4Defeated = 0;
    _secLevel3Defeated = 0;
    _secLevel2Defeated = 0;
    _secLevel1Defeated = 0;
    _groupSize = -1;
    _nextId = 0;
    _waitTime = -1;
    _status = COLLECTING;
    _clock = -1;
    _peers = ByzantineNetwork<syncBFTmessage, syncBFT_Peer>();
    _groups = std::map<int,SBFTGroup>();
    _requestQueue = std::deque<SBFTTransactionRequest>();
    _activeCommittees = std::vector<syncBFT_Committee>();
    
    int seed = (int)time(nullptr);
    _randomGenerator = std::default_random_engine(seed);
}

SBFTReferenceCommittee::SBFTReferenceCommittee(const SBFTReferenceCommittee &rhs){
    _securityLevel5 = rhs._securityLevel5;
    _securityLevel4 = rhs._securityLevel4;
    _securityLevel3 = rhs._securityLevel3;
    _securityLevel2 = rhs._securityLevel2;
    _securityLevel1 = rhs._securityLevel1;
    _secLevel5Defeated = rhs._secLevel5Defeated;
    _secLevel4Defeated = rhs._secLevel4Defeated;
    _secLevel3Defeated = rhs._secLevel3Defeated;
    _secLevel2Defeated = rhs._secLevel2Defeated;
    _secLevel1Defeated = rhs._secLevel1Defeated;
    _groupSize = rhs._groupSize;
    _nextId = rhs._nextId;
    _waitTime = rhs._waitTime;
    _status = rhs._status;
    _clock = rhs._clock;
    _peers = rhs._peers;
    _groups = rhs._groups;
    _requestQueue = rhs._requestQueue;
    _activeCommittees = rhs._activeCommittees;
    
    int seed = (int)time(nullptr);
    _randomGenerator = std::default_random_engine(seed);
}

SBFTReferenceCommittee& SBFTReferenceCommittee::operator=(const SBFTReferenceCommittee &rhs){
    if(this == &rhs){
        return *this;
    }
    _securityLevel5 = rhs._securityLevel5;
    _securityLevel4 = rhs._securityLevel4;
    _securityLevel3 = rhs._securityLevel3;
    _securityLevel2 = rhs._securityLevel2;
    _securityLevel1 = rhs._securityLevel1;
    _secLevel5Defeated = rhs._secLevel5Defeated;
    _secLevel4Defeated = rhs._secLevel4Defeated;
    _secLevel3Defeated = rhs._secLevel3Defeated;
    _secLevel2Defeated = rhs._secLevel2Defeated;
    _secLevel1Defeated = rhs._secLevel1Defeated;
    _groupSize = rhs._groupSize;
    _nextId = rhs._nextId;
    _waitTime = rhs._waitTime;
    _status = rhs._status;
    _clock = rhs._clock;
    _peers = rhs._peers;
    _groups = rhs._groups;
    _requestQueue = rhs._requestQueue;
    _activeCommittees = rhs._activeCommittees;
    return *this;
}
