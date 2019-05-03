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
#include "PBFTReferenceCommittee.hpp"

PBFTReferenceCommittee::PBFTReferenceCommittee(){
    _currentRound = 0;
    _groupSize = -1;
    _peers = Network<PBFT_Message, PBFTPeer_Sharded>();
    typedef std::vector<PBFTPeer_Sharded*> aGroup;
    _busyGroups = std::vector<std::pair<int,aGroup> >();
    _freeGroups = std::vector<std::pair<int,aGroup> >();
    _groupIds = std::vector<int>();
    _nextCommitteeId = 0;
    _nextSquenceNumber = 0;
    
    int seed = (int)time(nullptr);
    _randomGenerator = std::default_random_engine(seed);
}

PBFTReferenceCommittee::PBFTReferenceCommittee(const PBFTReferenceCommittee &rhs){
    _currentRound = rhs._currentRound;
    _groupSize = rhs._groupSize;
    _peers = rhs._peers;
    _busyGroups = rhs._busyGroups;
    _freeGroups = rhs._freeGroups;
    _groupIds = rhs._groupIds;
    _nextCommitteeId = rhs._nextCommitteeId;
    _nextSquenceNumber = rhs._nextSquenceNumber;
    
    int seed = (int)time(nullptr);
    _randomGenerator = std::default_random_engine(seed);
}

void PBFTReferenceCommittee::makeGroup(std::vector<PBFTPeer_Sharded*> group, int id){
    for(int i = 0; i < group.size(); i++){
        for(int j = 0; j < group.size(); j++){
            if(group[i]->id() != group[j]->id()){
                //assert(group[i]->getPhase() == IDEAL);
                group[i]->addGroupMember(*group[j]);
                group[i]->setGroup(id);
            }
        }
    }
    typedef std::vector<PBFTPeer_Sharded*> aGroup;
    _freeGroups.push_back(std::pair<int,aGroup>(id,group));
    _groupIds.push_back(id);
}

void PBFTReferenceCommittee::initNetwork(int numberOfPeers){
    
    SECURITY_LEVEL_5 = numberOfPeers/_groupSize;
    SECURITY_LEVEL_4 = SECURITY_LEVEL_5/2;
    SECURITY_LEVEL_3 = SECURITY_LEVEL_4/2;
    SECURITY_LEVEL_2 = SECURITY_LEVEL_3/2;
    SECURITY_LEVEL_1 = SECURITY_LEVEL_2/2;
    
    _peers.initNetwork(numberOfPeers);
    
    int groupId = 0;
    std::vector<PBFTPeer_Sharded*> group = std::vector<PBFTPeer_Sharded*>();
    
    for(int i = 0; i < _peers.size(); i++){
        
        _peers[i]->setGroup(groupId);
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
        case 0: return SECURITY_LEVEL_1; break;
        case 1: return SECURITY_LEVEL_2; break;
        case 2: return SECURITY_LEVEL_3; break;
        case 3: return SECURITY_LEVEL_4; break;
        case 4: return SECURITY_LEVEL_5; break;

        default: return SECURITY_LEVEL_1; break;
    }
}

transactionRequest PBFTReferenceCommittee::generateRequest(){
    if(_requestQueue.empty()){
        transactionRequest request;
        request.securityLevel = pickSecrityLevel();
        request.id = _currentRound;
        
        return request;
    }
    return _requestQueue.front();
}

typedef std::vector<PBFTPeer_Sharded*> aGroup;
void PBFTReferenceCommittee::makeRequest(){
    _requestQueue.push_back(generateRequest());
    int groupsNeeded = std::ceil(_requestQueue.front().securityLevel);
    groupsNeeded = 1;
    updateBusyGroup();
    
    // return if there is not enough free groups to make the committee
    if(_freeGroups.size() < groupsNeeded){
        return;
    }
    _requestQueue.erase(_requestQueue.begin());
    
    std::vector<std::pair<int,aGroup> > groupsInCommittee = std::vector<std::pair<int,aGroup> >();
    while(groupsInCommittee.size() < groupsNeeded){
        std::pair<int,aGroup> group = _freeGroups.back();
        _busyGroups.push_back(group);
        groupsInCommittee.push_back(group);
        _freeGroups.pop_back();
    }
    
    makeCommittee(groupsInCommittee);
    initCommittee(groupsInCommittee);
    
    for(int i = 0; i < groupsInCommittee.size(); i++){
        for(int j = 0; j < groupsInCommittee.front().second.size(); j++){
            if(groupsInCommittee[i].second[j]->isPrimary()){
                groupsInCommittee[i].second[j]->makeRequest(_nextSquenceNumber);
                _nextSquenceNumber++;
                return;
            }
        }
    }
    
}

typedef std::vector<PBFTPeer_Sharded*> aGroup;
void PBFTReferenceCommittee::updateBusyGroup(){
    for(int id = 0; id < _groupIds.size(); id++){
        aGroup group= getGroup(id);
        bool stillBusy = false;
        for(int i = 0; i < group.size(); i++){
            if(group[i]->getPhase() != IDEAL){
                stillBusy = true;
            }
        }
        if(!stillBusy){
            for(int i =0; i < _busyGroups.size(); i++){
                if(_busyGroups[i].first == id){
                    _busyGroups.erase(_busyGroups.begin() + i);
                    _freeGroups.push_back(std::pair<int, aGroup>(id,group));
                }
            }
        }
    }
}

void PBFTReferenceCommittee::initCommittee(std::vector<std::pair<int, aGroup> > groupsInCommittee){
    for(int group = 0; group < groupsInCommittee.size(); group++){
        aGroup groupInCommittee = groupsInCommittee[group].second;
        for(int peerIndex = 0; peerIndex < groupInCommittee.size(); peerIndex++){
            groupInCommittee[peerIndex]->clearPrimary();
            groupInCommittee[peerIndex]->initPrimary();
        }
    }
}


typedef std::vector<PBFTPeer_Sharded*> aGroup;
void PBFTReferenceCommittee::makeCommittee(std::vector<std::pair<int,aGroup> >  groupsForCommittee){
    // gets list of all committee members
    std::vector<PBFTPeer_Sharded*> committeeMembers =  std::vector<PBFTPeer_Sharded*>();
    for(int groupIndex = 0; groupIndex < groupsForCommittee.size(); groupIndex++){
        aGroup groupInCommitte = groupsForCommittee[groupIndex].second;
        for(int peerIndex = 0; peerIndex < groupInCommitte.size(); peerIndex++){
            committeeMembers.push_back(groupInCommitte[peerIndex]);
        }
    }
    
    // joins them togeather
    for(int peer = 0; peer < committeeMembers.size(); peer++){
        committeeMembers[peer]->clearCommittee();
        for(int otherPeer = 0; otherPeer < committeeMembers.size(); otherPeer++){
            committeeMembers[peer]->setCommittee(_nextCommitteeId);
            if(!(committeeMembers[peer]->id() == committeeMembers[otherPeer]->id())){
                committeeMembers[peer]->addcommitteeMember(*committeeMembers[otherPeer]);
            }
        }
    }
    _nextCommitteeId++;
}

void PBFTReferenceCommittee::setFaultTolerance(double f){
    for(int i = 0; i < _peers.size(); i++){
        _peers[i]->setFaultTolerance(f);
    }
}

typedef std::vector<PBFTPeer_Sharded*> aGroup;
aGroup PBFTReferenceCommittee::getGroup(int id)const{
    for(int i = 0; i < _freeGroups.size(); i++){
        if(_freeGroups[i].first == id){
            return _freeGroups[i].second;
        }
    }
    for(int i = 0; i < _busyGroups.size(); i++){
        if(_busyGroups[i].first == id){
            return _busyGroups[i].second;
        }
    }
    return aGroup();
}

std::vector<PBFTPeer_Sharded> PBFTReferenceCommittee::getPeers()const{
    std::vector<PBFTPeer_Sharded> peers = std::vector<PBFTPeer_Sharded>();
    for(int i = 0; i < _peers.size(); i++){
        peers.push_back(*_peers[i]);
    }
    return peers;
}

PBFTReferenceCommittee& PBFTReferenceCommittee::operator=(const PBFTReferenceCommittee &rhs){
    _currentRound = rhs._currentRound;
    _groupSize = rhs._groupSize;
    _peers = rhs._peers;
    _busyGroups = rhs._busyGroups;
    _freeGroups = rhs._freeGroups;
    _groupIds = rhs._groupIds;
    _nextCommitteeId = rhs._nextCommitteeId;
    _nextSquenceNumber = rhs._nextSquenceNumber;
    
    int seed = (int)time(nullptr);
    _randomGenerator = std::default_random_engine(seed);
    
    return *this;
}

std::ostream& PBFTReferenceCommittee::printTo(std::ostream& out)const{
    out<< "-- REFERENCE COMMITTEE SETUP --"<< std::endl<< std::endl;
    out<< std::left;
    out<< '\t'<< std::setw(LOG_WIDTH)<< "Current Round"<< std::setw(LOG_WIDTH)<< "Group Size"<< std::setw(LOG_WIDTH)<< "Number Of Groups"<< std::setw(LOG_WIDTH)<< "Number Of Free Groups"<< std::setw(LOG_WIDTH)<< "Number Of Busy Groups"<< std::setw(LOG_WIDTH)<< std::endl;
    out<< '\t'<< std::setw(LOG_WIDTH)<< _currentRound<< std::setw(LOG_WIDTH)<< _groupSize<< std::setw(LOG_WIDTH)<< _groupIds.size()<< std::setw(LOG_WIDTH)<< _freeGroups.size()<< std::setw(LOG_WIDTH)<< _busyGroups.size()<< std::setw(LOG_WIDTH)<< std::endl;
    out<< '\t'<< std::setw(LOG_WIDTH)<< "Request Queue Size"<< std::setw(LOG_WIDTH)<< "Next Committee Id"<< std::endl;
    out<< '\t'<< std::setw(LOG_WIDTH)<< _requestQueue.size()<< std::setw(LOG_WIDTH)<< _nextCommitteeId<< std::endl;
    
    _peers.printTo(out);
    
    return out;
}

