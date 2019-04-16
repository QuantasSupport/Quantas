//
//  BlockGuardSharded.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/28/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//
//
#include "PBFTPeer_Sharded.hpp"

PBFTPeer_Sharded::PBFTPeer_Sharded(std::string id) : PBFT_Peer(id) {
    _groupId = -1;
    _committeeId = -1;
    _groupMembers = std::vector<Peer<PBFT_Message>* >();
    _committeeMembers = std::vector<Peer<PBFT_Message>* >();
}

PBFTPeer_Sharded::PBFTPeer_Sharded(const PBFTPeer_Sharded &rhs) : PBFT_Peer(rhs){
    _groupId = rhs._groupId;
    _committeeId = rhs._committeeId;
    _groupMembers = rhs._groupMembers;
    _committeeMembers = rhs._committeeMembers;
}

void PBFTPeer_Sharded::braodcast(const PBFT_Message &msg){
    for(int i = 0; i < _committeeMembers.size(); i++){
        std::string neighborId = _committeeMembers[i]->id();
        Packet<PBFT_Message> pck(makePckId());
        pck.setSource(_id);
        pck.setTarget(neighborId);
        pck.setBody(msg);
        _outStream.push_back(pck);
    }
}

void PBFTPeer_Sharded::preformComputation(){
    if(_primary == nullptr){
        _primary = findPrimary(_committeeMembers);
    }
    collectMessages(); // sorts messages into there repective logs
    prePrepare();
    prepare();
    waitPrepare();
    commit();
    waitCommit();
    _currentRound++;
}



PBFTPeer_Sharded& PBFTPeer_Sharded::operator= (const PBFTPeer_Sharded &rhs){
    PBFT_Peer::operator=(rhs);
    
    _groupId = rhs._groupId;
    _committeeId = rhs._committeeId;
    _groupMembers = rhs._groupMembers;
    _committeeMembers = rhs._committeeMembers;
    
    return *this;
}

std::ostream& PBFTPeer_Sharded::printTo(std::ostream &out)const{
    PBFT_Peer::printTo(out);
    
    out<< "\t"<< std::setw(LOG_WIDTH)<< "Group Id"<< std::setw(LOG_WIDTH)<< "Committee Id"<< std::setw(LOG_WIDTH)<< "Group Size"<< std::setw(LOG_WIDTH)<< "Committee Size"<<  std::endl;
    out<< "\t"<< std::setw(LOG_WIDTH)<< _groupId<< std::setw(LOG_WIDTH)<< _committeeId<< std::setw(LOG_WIDTH)<< _groupMembers.size() + 1<< std::setw(LOG_WIDTH)<< _committeeMembers.size() + 1<< std::endl<< std::endl;
    
    std::vector<std::string> groupMembersIds = std::vector<std::string>();
    for(int i = 0; i < _groupMembers.size(); i++){
        groupMembersIds.push_back(_groupMembers[i]->id());
    }
    
    std::vector<std::string> committeeMembersIds = std::vector<std::string>();
    for(int i = 0; i < _committeeMembers.size(); i++){
        committeeMembersIds.push_back(_committeeMembers[i]->id());
    }
    
    while(committeeMembersIds.size() > groupMembersIds.size()){
        groupMembersIds.push_back("");
    }
    while(groupMembersIds.size() > committeeMembersIds.size()){
        committeeMembersIds.push_back("");
    }
    
    out<< "\t"<< std::setw(LOG_WIDTH)<< "Group Members"<< std::setw(LOG_WIDTH)<< "Committee Members"<< std::endl;
    for(int i = 0; i < committeeMembersIds.size(); i++){
        out<< "\t"<< std::setw(LOG_WIDTH)<< groupMembersIds[i]<< std::setw(LOG_WIDTH)<< committeeMembersIds[i]<< std::endl;
    }
    
    return out;
}
