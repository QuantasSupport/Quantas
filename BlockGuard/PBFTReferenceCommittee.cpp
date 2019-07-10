//
//  PBFTReferenceCommittee.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 4/3/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <random>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <set>
#include "PBFTReferenceCommittee.hpp"

PBFTReferenceCommittee::PBFTReferenceCommittee(){
    _securityLevel5 = -1;
    _securityLevel4 = -1;
    _securityLevel3 = -1;
    _securityLevel2 = -1;
    _securityLevel1 = -1;
    _totalTransactionsSubmitted = 0;
    _currentRound = 0;
    _groupSize = -1;
    _peers = ByzantineNetwork<PBFT_Message, PBFTPeer_Sharded>();
    _busyGroups = std::vector<int>();
    _freeGroups = std::vector<int>();
    _groupIds = std::vector<int>();
    _nextCommitteeId = 0;
    _nextSquenceNumber = 0;
    _requestQueue = std::vector<transactionRequest>();
    _groups = std::map<int,aGroup>();
    _faultTolerance = 1;
    _currentCommittees = std::vector<int>();
    _log = nullptr;
    _printNetwork = false;

    int seed = (int)time(nullptr);
    _randomGenerator = std::default_random_engine(seed);
}

PBFTReferenceCommittee::PBFTReferenceCommittee(const PBFTReferenceCommittee &rhs){
    _securityLevel5 = rhs._securityLevel5;
    _securityLevel4 = rhs._securityLevel4;
    _securityLevel3 = rhs._securityLevel3;
    _securityLevel2 = rhs._securityLevel2;
    _securityLevel1 = rhs._securityLevel1;
    _currentRound = rhs._currentRound;
    _groupSize = rhs._groupSize;
    _peers = rhs._peers;
    _busyGroups = rhs._busyGroups;
    _freeGroups = rhs._freeGroups;
    _groupIds = rhs._groupIds;
    _nextCommitteeId = rhs._nextCommitteeId;
    _nextSquenceNumber = rhs._nextSquenceNumber;
    _requestQueue = rhs._requestQueue;
    _groups = rhs._groups;
    _faultTolerance = rhs._faultTolerance;
    _currentCommittees = rhs._currentCommittees;
    _log = rhs._log;
    _totalTransactionsSubmitted = rhs._totalTransactionsSubmitted;
    _printNetwork = rhs._printNetwork;
    
    int seed = (int)time(nullptr);
    _randomGenerator = std::default_random_engine(seed);
}

void PBFTReferenceCommittee::makeGroup(std::vector<PBFTPeer_Sharded*> group, int id){
    for(int i = 0; i < group.size(); i++){
        group[i]->setGroup(id);
        for(int j = 0; j < group.size(); j++){
            if(group[i]->id() != group[j]->id()){
                group[i]->addGroupMember(*group[j]);
            }
        }
    }
    _freeGroups.push_back(id);
    _groupIds.push_back(id);
    _groups[id] = group;
}

void PBFTReferenceCommittee::initNetwork(int numberOfPeers){
    
    // 5 is high (all groups )1 is low (4 groups for 1024 peers)
    _securityLevel5 = numberOfPeers/_groupSize;
    _securityLevel4 = _securityLevel5/2;
    _securityLevel3 = _securityLevel4/2;
    _securityLevel2 = _securityLevel3/2;
    _securityLevel1 = _securityLevel2/2;
    
    _peers.initNetwork(numberOfPeers);
    
    int groupId = 0;
    std::vector<PBFTPeer_Sharded*> group = std::vector<PBFTPeer_Sharded*>();
    
    for(int i = 0; i < _peers.size(); i++){
        group.push_back(_peers[i]);
        
        if(group.size() == _groupSize){
            makeGroup(group,groupId);
            group = std::vector<PBFTPeer_Sharded*>();
            groupId++;
        }
    }

    // connects left over peers into s small group
    if(!group.empty()){
        makeGroup(group,groupId);
    }
    setFaultTolerance(_faultTolerance);
    return;
}

double PBFTReferenceCommittee::pickSecrityLevel(){

    std::uniform_int_distribution<int> coin(0,1);
    int trails = 0;
    int heads = coin(_randomGenerator);
    while(!heads){
        trails++;
        heads = coin(_randomGenerator);
    }
    
    switch (trails) {
        case 0: return _securityLevel1; break;
        case 1: return _securityLevel2; break;
        case 2: return _securityLevel3; break;
        case 3: return _securityLevel4; break;
        case 4: return _securityLevel5; break;

        default: return _securityLevel5; break;
    }
}

transactionRequest PBFTReferenceCommittee::generateRequest(){
    transactionRequest request;
    request.securityLevel = pickSecrityLevel();
    request.submissionRound = _currentRound;
    
    return request;
}

transactionRequest PBFTReferenceCommittee::generateRequest(int securityLevel){
    transactionRequest request;
    request.securityLevel = securityLevel;
    request.submissionRound = _currentRound;
    
    return request;
}

void PBFTReferenceCommittee::serveRequest(){
    if(_requestQueue.empty()){
        return;
    }
    int groupsNeeded = std::ceil(_requestQueue.front().securityLevel);
    int submissionRound = _requestQueue.front().submissionRound;
    updateBusyGroup();
    
    // return if there is not enough free groups to make the committee
    if(_freeGroups.size() < groupsNeeded){
        return;
    }
    _requestQueue.erase(_requestQueue.begin());
    
    std::vector<int> groupsInCommittee = std::vector<int>();
    while(groupsInCommittee.size() < groupsNeeded){
        int groupId = _freeGroups.back();
        _busyGroups.push_back(groupId);
        groupsInCommittee.push_back(groupId);
        _freeGroups.pop_back();
    }
    
    makeCommittee(groupsInCommittee);
    initCommittee(groupsInCommittee);
    
    for(int i = 0; i < groupsInCommittee.size(); i++){
        aGroup group = getGroup(groupsInCommittee[i]);
        for(int j = 0; j < group.size(); j++){
            if(group[j]->isPrimary()){
                group[j]->makeRequest(_nextSquenceNumber,submissionRound);
                _nextSquenceNumber++;
                return;
            }
        }
    }
    
}

void PBFTReferenceCommittee::updateBusyGroup(){
    std::vector<int> aliveCommittees = std::vector<int>(); // if a group is still bust we need the committee it belongs to
    // check each busy group to see if they are still busy
    auto id = _busyGroups.begin(); 
    while(id != _busyGroups.end()){
        aGroup group= getGroup(*id);
        bool stillBusy = false;
        for(int i = 0; i < group.size(); i++){
            // if the group is still busy it will still have a committee id and thus we need to track it
            if(group[i]->getCommittee() != -1){
                stillBusy = true;
                // find works in n^2 time and lower bound is log(n) if sorted
                auto found = std::lower_bound(aliveCommittees.begin(),aliveCommittees.end(),group[i]->getCommittee());
                if( found == aliveCommittees.end()){
                    aliveCommittees.push_back(group[i]->getCommittee());
                    std::sort(aliveCommittees.begin(),aliveCommittees.end()); // sort for next check
                }
                break; // no need to check rest of group
            }
        }
        // if the group is not still alive then we need to move its group id to free and remove from busy
        if(!stillBusy){
            _freeGroups.push_back(*id);
            _busyGroups.erase(id);
        // else we just continue
        }else{
            id++;
        }
    }

    _currentCommittees = aliveCommittees; // update list of currently active committees
}

std::vector<aGroup> PBFTReferenceCommittee::getCommittee(int committeeId)const{
    std::vector<aGroup> committee = std::vector<aGroup>();

    // need to find every group that belongs to committee id
    for(auto id = _groupIds.begin(); id != _groupIds.end(); id++){ 
        aGroup group = getGroup(*id);
        // check each group member to see if they belong to that committee still
        bool inCommittee = false;
        for(auto peer = group.begin(); peer != group.end(); peer++){
            if((*peer)->getCommittee() == committeeId){
                inCommittee = true;
            }
        }
        if(inCommittee){
            committee.push_back(group);
        }
    }

    return committee;
}

void PBFTReferenceCommittee::initCommittee(std::vector<int> groupsInCommittee){
    for(auto groupId = groupsInCommittee.begin(); groupId != groupsInCommittee.end(); groupId++){
        aGroup groupInCommittee = getGroup(*groupId);
        for(int peerIndex = 0; peerIndex < groupInCommittee.size(); peerIndex++){
            groupInCommittee[peerIndex]->clearPrimary();
            groupInCommittee[peerIndex]->initPrimary();
        }
    }
}


void PBFTReferenceCommittee::makeCommittee(std::vector<int> groupsForCommittee){
    // gets list of all committee members
    std::vector<PBFTPeer_Sharded*> committeeMembers =  std::vector<PBFTPeer_Sharded*>();
    for(auto groupId = groupsForCommittee.begin(); groupId != groupsForCommittee.end(); groupId++){
        aGroup groupInCommitte = getGroup(*groupId);
        for(int peerIndex = 0; peerIndex < groupInCommitte.size(); peerIndex++){
            committeeMembers.push_back(groupInCommitte[peerIndex]);
        }
    }

    // clear old committee
    for(int peer = 0; peer < committeeMembers.size(); peer++){
            committeeMembers[peer]->clearCommittee();
    }

    // joins them togeather
    for(int peer = 0; peer < committeeMembers.size(); peer++){
        committeeMembers[peer]->setCommittee(_nextCommitteeId);
        for(int otherPeer = 0; otherPeer < committeeMembers.size(); otherPeer++){
            if(!(committeeMembers[peer]->id() == committeeMembers[otherPeer]->id())){
                committeeMembers[peer]->addCommitteeMember(*committeeMembers[otherPeer]);
            }
        }
    }
    _currentCommittees.push_back(_nextCommitteeId);
    _nextCommitteeId++;
}

void PBFTReferenceCommittee::setFaultTolerance(double f){
    _faultTolerance = f;
    for(int i = 0; i < _peers.size(); i++){
        _peers[i]->setFaultTolerance(_faultTolerance);
    }
}

aGroup PBFTReferenceCommittee::getGroup(int id)const{
    return _groups.at(id);
}

std::vector<PBFTPeer_Sharded> PBFTReferenceCommittee::getPeers()const{
    std::vector<PBFTPeer_Sharded> peers = std::vector<PBFTPeer_Sharded>();
    for(int i = 0; i < _peers.size(); i++){
        peers.push_back(*_peers[i]);
    }
    return peers;
}

std::vector<ledgerEntery> PBFTReferenceCommittee::getGlobalLedger()const{
    std::vector<ledgerEntery> globalLegder;
    for(int i = 0; i < _peers.size(); i++){
        auto localLedger = _peers[i]->getLedger();
        for(auto transaction = localLedger.begin(); transaction != localLedger.end(); transaction++){
            bool found = false;
            for(auto global = globalLegder.begin(); global != globalLegder.end(); global++){
                if(*transaction == global->first){
                    found = true;
                }
            }
            if(!found){
                int sizeIndez = std::distance(localLedger.begin(),transaction);
                globalLegder.push_back(ledgerEntery(*transaction,_peers[i]->getCommitteeSizes()[sizeIndez]));
            }
        }
    }
    return globalLegder;
}

void PBFTReferenceCommittee::setMaxSecurityLevel(int max){
    if(_securityLevel5 > max){
        _securityLevel5 = max;
    }
    if(_securityLevel4 > max){
        _securityLevel4 = max;
    }
    if(_securityLevel3 > max){
        _securityLevel3 = max;
    }
    if(_securityLevel2 > max){
        _securityLevel2 = max;
    }
    if(_securityLevel1 > max){
        _securityLevel1 = max;
    }
}

void PBFTReferenceCommittee::setMinSecurityLevel(int min){
    if(_securityLevel5 < min){
        _securityLevel5 = min;
    }
    if(_securityLevel4 < min){
        _securityLevel4 = min;
    }
    if(_securityLevel3 < min){
        _securityLevel3 = min;
    }
    if(_securityLevel2 < min){
        _securityLevel2 = min;
    }
    if(_securityLevel1 < min){
        _securityLevel1 = min;
    }
}

void PBFTReferenceCommittee::shuffleByzantines(int n){
    std::vector<int> correct = std::vector<int>();
    std::vector<int> byz = std::vector<int>();
    for(int i = 0; i < _peers.size(); i++){
        if(_peers[i]->getCommittee() == -1){
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

PBFTReferenceCommittee& PBFTReferenceCommittee::operator=(const PBFTReferenceCommittee &rhs){
    _securityLevel5 = rhs._securityLevel5;
    _securityLevel4 = rhs._securityLevel4;
    _securityLevel3 = rhs._securityLevel3;
    _securityLevel2 = rhs._securityLevel2;
    _securityLevel1 = rhs._securityLevel1;
    _currentRound = rhs._currentRound;
    _groupSize = rhs._groupSize;
    _peers = rhs._peers;
    _busyGroups = rhs._busyGroups;
    _freeGroups = rhs._freeGroups;
    _groupIds = rhs._groupIds;
    _nextCommitteeId = rhs._nextCommitteeId;
    _nextSquenceNumber = rhs._nextSquenceNumber;
    _requestQueue = rhs._requestQueue;
    _groups = rhs._groups;
    _faultTolerance = rhs._faultTolerance;
    _currentCommittees = rhs._currentCommittees;
    _log = rhs._log;
    _totalTransactionsSubmitted = rhs._totalTransactionsSubmitted;
    _printNetwork = rhs._printNetwork;

    int seed = (int)time(nullptr);
    _randomGenerator = std::default_random_engine(seed);
    
    return *this;
}

std::ostream& PBFTReferenceCommittee::printTo(std::ostream& out)const{
    out<< "-- REFERENCE COMMITTEE SETUP --"<< std::endl<< std::endl;
    out<< std::left;
    out<< '\t'<< std::setw(LOG_WIDTH)<< "Current Round" << std::setw(LOG_WIDTH) << "Group Size" << std::setw(LOG_WIDTH) << "Number Of Groups"   << std::setw(LOG_WIDTH) << "Number Of Free Groups"  << std::setw(LOG_WIDTH) << "Number Of Busy Groups" << std::endl;
    out<< '\t'<< std::setw(LOG_WIDTH)<< _currentRound   << std::setw(LOG_WIDTH) << _groupSize   << std::setw(LOG_WIDTH) << _groupIds.size()     << std::setw(LOG_WIDTH) << _freeGroups.size()       << std::setw(LOG_WIDTH) << _busyGroups.size()<< std::endl;
    out<< '\t'<< std::setw(LOG_WIDTH)<<  "Request Queue Size"   << std::setw(LOG_WIDTH)<< "Next Committee Id"   << std::endl;
    out<< '\t'<< std::setw(LOG_WIDTH)<< _requestQueue.size()    << std::setw(LOG_WIDTH)<< _nextCommitteeId      << std::endl;
    out<<std::endl;
    out<< '\t'<<"Current Committees:"<< std::endl;
    for(auto id = _currentCommittees.begin(); id != _currentCommittees.end(); id++){
        std::vector<aGroup> committee = getCommittee(*id);
        out<< '\t'<< std::setw(LOG_WIDTH)<< "Committee ID:" + std::to_string(*id) << std::setw(LOG_WIDTH)<< "Current Committees Sizes:" + std::to_string(committee.size())<< std::endl;
        out<< '\t'<< std::setw(LOG_WIDTH)<< "Peer:"<< std::setw(LOG_WIDTH)<< "Current Phase:"<< std::setw(LOG_WIDTH)<< "Last Commit:"<< std::setw(LOG_WIDTH)<< "Current Committee"<< std::setw(LOG_WIDTH)<< "Current Primary"<< std::endl;
        for(auto group = committee.begin(); group != committee.end(); group++){
            for(auto peer = group->begin(); peer != group->end(); peer++ ){
                if((*peer)->getLedger().size() != 0){
                    out<< '\t'<< std::setw(LOG_WIDTH)<< (*peer)->id()<< std::setw(LOG_WIDTH)<< (*peer)->getPhase()<< std::setw(LOG_WIDTH)<< (*peer)->getLedger().end()->commit_round<< std::setw(LOG_WIDTH)<< (*peer)->getCommittee()<< std::setw(LOG_WIDTH)<< (*peer)->getPrimary()<<std::endl;
                }else{
                    out<< '\t'<< std::setw(LOG_WIDTH)<< (*peer)->id()<< std::setw(LOG_WIDTH)<< (*peer)->getPhase()<< std::setw(LOG_WIDTH)<< "None"<< std::setw(LOG_WIDTH)<< (*peer)->getCommittee()<<std::endl;
                }
            }
        }
    }

    if(_printNetwork){
        _peers.printTo(out);
    }

    return out;
}

// "Total Request:,Max Ledger:,Ratio Honest To Defeated,Average Waiting Time 1,Average Waiting Time 2,Average Waiting Time 3 ,Average Waiting Time 4,Average Waiting Time 5\n"
void calculateResults(const std::vector<std::pair<PBFT_Message,int> > globalLedger){
    
}
