//
//  syncBFT_Peer_Sharded.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 4/9/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "syncBFT_Peer_Sharded.hpp"

syncBFT_Peer_Sharded::syncBFT_Peer_Sharded(std::string id) : syncBFT_Peer(id){
    _groupId = -1;
    _committeeId = -1;
    _groupMembers = std::vector<Peer<syncBFTmessage>* >();
    _committeeMembers = std::vector<Peer<syncBFTmessage>* >();
    _faultUpperBound = ((peerCount() - 1)/2) +1;
}

syncBFT_Peer_Sharded::syncBFT_Peer_Sharded(const syncBFT_Peer_Sharded &rhs) : syncBFT_Peer(rhs){
    _groupId = rhs._groupId;
    _committeeId = rhs._committeeId;
    _groupMembers = rhs._groupMembers;
    _committeeMembers = rhs._committeeMembers;
    _faultUpperBound = rhs._faultUpperBound;
}

syncBFT_Peer_Sharded& syncBFT_Peer_Sharded::operator=(const syncBFT_Peer_Sharded &rhs){
    syncBFT_Peer::operator=(rhs);
    _groupId = rhs._groupId;
    _committeeId = rhs._committeeId;
    _groupMembers = rhs._groupMembers;
    _committeeMembers = rhs._committeeMembers;
    
    return *this;
}

void syncBFT_Peer_Sharded::populateOutStream(const syncBFTmessage &msg){
    for(auto &member : _committeeMembers) {
        Packet<syncBFTmessage> newMessage(std::to_string(iter), member->id(), _id);
        newMessage.setBody(msg);
        _outStream.push_back(newMessage);
    }
}

std::vector<std::string> syncBFT_Peer_Sharded::leaderIdCandidates()const{
    vector<string> candidates = vector<string>();
    for(int i = 0; i < _committeeMembers.size(); i++){
        candidates.push_back(_committeeMembers[i]->id());
    }
    return candidates;
}

std::ostream& syncBFT_Peer_Sharded::printTo(std::ostream &out)const{
    syncBFT_Peer::printTo(out);
    
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
