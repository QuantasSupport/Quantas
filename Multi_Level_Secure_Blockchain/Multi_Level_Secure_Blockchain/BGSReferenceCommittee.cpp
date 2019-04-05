//
//  NetworkBGS.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 4/3/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <random>
#include <chrono>
#include <ctime>
#include "BGSReferenceCommittee.hpp"

BGSReferenceCommittee::BGSReferenceCommittee(){
    _currentRound = 0;
    _groupSize = -1;
    _peers = Network<PBFT_Message, BlockGuardPeer_Sharded>();
    typedef std::vector<BlockGuardPeer_Sharded*> aGroup;
    _busyGroups = std::vector<std::pair<int,aGroup> >();
    _freeGroups = std::vector<std::pair<int,aGroup> >();
    _groupIds = std::vector<int>();
    _nextCommitteeId = 0;
    
    int seed = (int)time(nullptr);
    _randomGenerator = std::default_random_engine(seed);
}

BGSReferenceCommittee::BGSReferenceCommittee(const BGSReferenceCommittee &rhs){
    _currentRound = rhs._currentRound;
    _groupSize = rhs._groupSize;
    _peers = rhs._peers;
    _busyGroups = rhs._busyGroups;
    _freeGroups = rhs._freeGroups;
    _groupIds = rhs._groupIds;
    _nextCommitteeId = rhs._nextCommitteeId;
    
    int seed = (int)time(nullptr);
    _randomGenerator = std::default_random_engine(seed);
}

void BGSReferenceCommittee::makeGroup(std::vector<BlockGuardPeer_Sharded*> group, int id){
    for(int i = 0; i < group.size(); i++){
        for(int j = 0; j < group.size(); j++){
            if(group[i]->id() != group[j]->id()){
                group[i]->addGroupMember(*group[j]);
                group[i]->setGroup(id);
            }
        }
    }
    typedef std::vector<BlockGuardPeer_Sharded*> aGroup;
    _freeGroups.push_back(std::pair<int,aGroup>(id,group));
    _groupIds.push_back(id);
}

void BGSReferenceCommittee::initNetwork(int numberOfPeers){
    _peers.initNetwork(numberOfPeers);
    
    int groupId = 0;
    std::vector<BlockGuardPeer_Sharded*> group = std::vector<BlockGuardPeer_Sharded*>();
    
    for(int i = 0; i < _peers.size(); i++){
        
        _peers[i]->setGroup(groupId);
        group.push_back(_peers[i]);
        
        if(group.size() == _groupSize){
            makeGroup(group,groupId);
            group = std::vector<BlockGuardPeer_Sharded*>();
            groupId++;
        }
    }

    // connects left over peers into s small group
    if(!group.empty()){
        makeGroup(group,groupId);
    }
    
    return;
}

double BGSReferenceCommittee::pickSecrityLevel(){

    std::uniform_int_distribution<int> coin(0,1);
    int trails = 0;
    int heads = coin(_randomGenerator);
    if(!heads){
        trails++;
    }
    
    switch (trails) {
        case 0: return SECURITY_LEVEL_1; break;
        case 1: return SECURITY_LEVEL_2; break;
        case 2: return SECURITY_LEVEL_3; break;
        case 3: return SECURITY_LEVEL_4; break;
        case 4: return SECURITY_LEVEL_5; break;
        case 5: return SECURITY_LEVEL_6; break;
        case 6: return SECURITY_LEVEL_7; break;

        default: return SECURITY_LEVEL_1; break;
    }
}

BGSrequest BGSReferenceCommittee::generateRequest(){
    if(_requestQueue.empty()){
        BGSrequest request;
        request.securityLevel = pickSecrityLevel();
        request.id = _currentRound;
        
        return request;
    }
    return _requestQueue.front();
}

typedef std::vector<BlockGuardPeer_Sharded*> aGroup;
void BGSReferenceCommittee::makeRequest(){
    _requestQueue.push_back(generateRequest());
    int groupsNeeded = (int)(_groupIds.size()*_requestQueue.front().securityLevel) == 0 ? 1 : _groupIds.size()*_requestQueue.front().securityLevel;
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
        if(!groupsInCommittee.front().second[i]->isPrimary()){
            groupsInCommittee.front().second[i]->makeRequest();
            return;
        }
    }
    
}

typedef std::vector<BlockGuardPeer_Sharded*> aGroup;
void BGSReferenceCommittee::updateBusyGroup(){
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

void BGSReferenceCommittee::initCommittee(std::vector<std::pair<int, aGroup> > groupsInCommittee){
    for(int group = 0; group < groupsInCommittee.size(); group++){
        aGroup groupInCommittee = groupsInCommittee[group].second;
        for(int peerIndex = 0; peerIndex < groupInCommittee.size(); peerIndex++){
            groupInCommittee[peerIndex]->clearPrimary();
            groupInCommittee[peerIndex]->initPrimary();
        }
    }
}


typedef std::vector<BlockGuardPeer_Sharded*> aGroup;
void BGSReferenceCommittee::makeCommittee(std::vector<std::pair<int,aGroup> >  groupsForCommittee){
    for(int groupIndex = 0; groupIndex < groupsForCommittee.size(); groupIndex++){
        aGroup groupInCommitte = groupsForCommittee[groupIndex].second;
        for(int peerIndex = 0; peerIndex < groupInCommitte.size(); peerIndex++){
            groupInCommitte[peerIndex]->setCommittee(_nextCommitteeId);
            for(int otherPeer = 0; otherPeer < groupInCommitte.size(); otherPeer++){
                if(groupInCommitte[peerIndex]->id() != groupInCommitte[otherPeer]->id()){
                    groupInCommitte[peerIndex]->addcommitteeMember(*groupInCommitte[otherPeer]);
                }
            }
        }
    }
    _nextCommitteeId++;
}

void BGSReferenceCommittee::setFaultTolerance(double f){
    for(int i = 0; i < _peers.size(); i++){
        _peers[i]->setFaultTolerance(f);
    }
}

typedef std::vector<BlockGuardPeer_Sharded*> aGroup;
aGroup BGSReferenceCommittee::getGroup(int id)const{
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

BGSReferenceCommittee& BGSReferenceCommittee::operator=(const BGSReferenceCommittee &rhs){
    _currentRound = rhs._currentRound;
    _groupSize = rhs._groupSize;
    _peers = rhs._peers;
    _busyGroups = rhs._busyGroups;
    _freeGroups = rhs._freeGroups;
    _groupIds = rhs._groupIds;
    _nextCommitteeId = rhs._nextCommitteeId;
    
    int seed = (int)time(nullptr);
    _randomGenerator = std::default_random_engine(seed);
    
    return *this;
}

std::ostream& BGSReferenceCommittee::printTo(std::ostream& out)const{
    out<< "-- REFERENCE COMMITTEE SETUP --"<< std::endl<< std::endl;
    out<< std::left;
    out<< '\t'<< std::setw(LOG_WIDTH)<< "Current Round"<< std::setw(LOG_WIDTH)<< "Group Size"<< std::setw(LOG_WIDTH)<< "Number Of Groups"<< std::setw(LOG_WIDTH)<< "Number Of Free Groups"<< std::setw(LOG_WIDTH)<< "Number Of Busy Groups"<< std::setw(LOG_WIDTH)<< std::endl;
    out<< '\t'<< std::setw(LOG_WIDTH)<< _currentRound<< std::setw(LOG_WIDTH)<< _groupSize<< std::setw(LOG_WIDTH)<< _groupIds.size()<< std::setw(LOG_WIDTH)<< _freeGroups.size()<< std::setw(LOG_WIDTH)<< _busyGroups.size()<< std::setw(LOG_WIDTH)<< std::endl;
    out<< '\t'<< std::setw(LOG_WIDTH)<< "Request Queue Size"<< std::setw(LOG_WIDTH)<< "Next Committee Id"<< std::endl;
    out<< '\t'<< std::setw(LOG_WIDTH)<< _requestQueue.size()<< std::setw(LOG_WIDTH)<< _nextCommitteeId<< std::endl;
    
    _peers.printTo(out);
    
    return out;
}

