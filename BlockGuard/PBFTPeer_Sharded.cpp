//
//  BlockGuardSharded.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/28/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//
//
#include "PBFTPeer_Sharded.hpp"

PBFTPeer_Sharded::PBFTPeer_Sharded(std::string id) : PBFT_Peer(id) {
    _groupId = -1;
    _committeeId = -1;
    _groupMembers = std::map<std::string, Peer<PBFT_Message>* >();
    _committeeMembers = std::map<std::string, Peer<PBFT_Message>* >();
    _printCommittee = false;
    _printGroup = false;
    _committeeSizes = std::vector<int>();
}

PBFTPeer_Sharded::PBFTPeer_Sharded(const PBFTPeer_Sharded &rhs) : PBFT_Peer(rhs){
    _groupId = rhs._groupId;
    _committeeId = rhs._committeeId;
    _groupMembers = rhs._groupMembers;
    _committeeMembers = rhs._committeeMembers;
    _printCommittee = rhs._printCommittee;
    _printGroup = rhs._printGroup;
    _committeeSizes = rhs._committeeSizes;
}

void PBFTPeer_Sharded::braodcast(const PBFT_Message &msg){
    for (auto it=_committeeMembers.begin(); it!=_committeeMembers.end(); ++it){
        std::string neighborId = it->first;
        Packet<PBFT_Message> pck(makePckId());
        pck.setSource(_id);
        pck.setTarget(neighborId);
        pck.setBody(msg);
        _outStream.push_back(pck);
    }
}

void PBFTPeer_Sharded::commitRequest(){
    PBFT_Message commit = _currentRequest;
    commit.result = _currentRequestResult;
    commit.commit_round = _clock;

    int numberOfByzantineCommits = 0;
    int correctCommitMsg = 0;
    for(auto commitMsg = _commitLog.begin(); commitMsg != _commitLog.end(); commitMsg++){
        if(commitMsg->sequenceNumber == _currentRequest.sequenceNumber
                && commitMsg->view == _currentView
                && commitMsg->result == _currentRequestResult){
            if(commitMsg->byzantine){
                numberOfByzantineCommits++;
            }else{
                correctCommitMsg++;
            }
        }
    }
    
    // if we dont have enough honest commits and we have not heard from everyone wait
    if(correctCommitMsg <= faultyPeers()){
        if (correctCommitMsg + numberOfByzantineCommits != _committeeMembers.size() + 1){
            return;
        }
    }
    
    if(_currentRequest.byzantine && correctCommitMsg >= faultyPeers()){
        // clear messages from old view
        int oldTransaction = _currentRequest.sequenceNumber;
        int view = _currentView;
        auto entry = _prePrepareLog.begin();
        while(entry != _prePrepareLog.end()){
            if(entry->sequenceNumber == oldTransaction && entry->view == view){
                _prePrepareLog.erase(entry++);
            }else{
                entry++;
            }
        }
        
        entry = _prepareLog.begin();
        while(entry != _prepareLog.end()){
            if(entry->sequenceNumber == oldTransaction && entry->view == view){
                _prepareLog.erase(entry++);
            }else{
                entry++;
            }
        }
        
        entry = _commitLog.begin();
        while(entry != _commitLog.end()){
            if(entry->sequenceNumber == oldTransaction && entry->view == view){
                _commitLog.erase(entry++);
            }else{
                entry++;
            }
        }
        viewChange(_committeeMembers);
    }else if(!_currentRequest.byzantine && correctCommitMsg >= faultyPeers()){
        commit.defeated = false;
        _ledger.push_back(commit);
        _committeeSizes.push_back(_committeeMembers.size()+1);// +1 for self
        _currentRequest = PBFT_Message();
        clearCommittee();
        _currentRequest = PBFT_Message();
    }else{
        commit.defeated = true;
        _ledger.push_back(commit);
        _committeeSizes.push_back(_committeeMembers.size()+1);// +1 for self
        _currentRequest = PBFT_Message();
        clearCommittee();
        _currentRequest = PBFT_Message();
    }
    
    for(auto confirmedTransaction = _ledger.begin(); confirmedTransaction != _ledger.end(); confirmedTransaction++){
        cleanLogs(confirmedTransaction->sequenceNumber);
    }
    _currentPhase = IDEAL; // complete distributed-consensus
    _currentRequestResult = 0;
}

std::vector<std::string> PBFTPeer_Sharded::getGroupMembers()const{
    std::vector<std::string> groupIds = std::vector<std::string>();
    for (auto it=_groupMembers.begin(); it!=_groupMembers.end(); ++it){
        groupIds.push_back(it->first);
    }
    return groupIds;
};

std::vector<std::string> PBFTPeer_Sharded::getCommitteeMembers()const{
    std::vector<std::string> committeeIds = std::vector<std::string>();
    for (auto it=_committeeMembers.begin(); it!=_committeeMembers.end(); ++it){
        committeeIds.push_back(it->first);
    }
    return committeeIds;
};

void PBFTPeer_Sharded::preformComputation(){
    if(_primary == nullptr){
        _primary = findPrimary(_committeeMembers);
    }
    //_committeeMembers[_primary->id()];
    collectMessages(); // sorts messages into there repective logs
    prePrepare();
    prepare();
    waitPrepare();
    commit();
    waitCommit();
    for(auto confirmedTransaction = _ledger.begin(); confirmedTransaction != _ledger.end(); confirmedTransaction++){
        cleanLogs(confirmedTransaction->sequenceNumber);
    }
}

PBFTPeer_Sharded& PBFTPeer_Sharded::operator= (const PBFTPeer_Sharded &rhs){
    PBFT_Peer::operator=(rhs);
    
    _groupId = rhs._groupId;
    _committeeId = rhs._committeeId;
    _groupMembers = rhs._groupMembers;
    _committeeMembers = rhs._committeeMembers;
    _printCommittee = rhs._printCommittee;
    _printGroup = rhs._printGroup;
    _committeeSizes = rhs._committeeSizes;
    
    return *this;
}

std::ostream& PBFTPeer_Sharded::printTo(std::ostream &out)const{
    PBFT_Peer::printTo(out);
    
    out<< "\t"<< std::setw(LOG_WIDTH)<< "Group Id"<< std::setw(LOG_WIDTH)<< "Committee Id"<< std::setw(LOG_WIDTH)<< "Group Size"<< std::setw(LOG_WIDTH)<< "Committee Size"<<  std::endl;
    out<< "\t"<< std::setw(LOG_WIDTH)<< _groupId<< std::setw(LOG_WIDTH)<< _committeeId<< std::setw(LOG_WIDTH)<< _groupMembers.size() + 1<< std::setw(LOG_WIDTH)<< _committeeMembers.size() + 1<< std::endl<< std::endl;
    
    std::vector<std::string> groupMembersIds = std::vector<std::string>();
    for (auto it=_groupMembers.begin(); it!=_groupMembers.end(); ++it){
        groupMembersIds.push_back(it->first);
    }
    
    std::vector<std::string> committeeMembersIds = std::vector<std::string>();
    for (auto it=_committeeMembers.begin(); it!=_committeeMembers.end(); ++it){
        committeeMembersIds.push_back(it->first);
    }
    
    while(committeeMembersIds.size() > groupMembersIds.size()){
        groupMembersIds.push_back("");
    }
    while(groupMembersIds.size() > committeeMembersIds.size()){
        committeeMembersIds.push_back("");
    }

    if(_printCommittee || _printGroup){
        out<< "\t"<< std::setw(LOG_WIDTH)<< "Group Members"<< std::setw(LOG_WIDTH)<< "Committee Members"<< std::endl;
    }
    for(int i = 0; i < committeeMembersIds.size(); i++){
        if(_printCommittee && _printGroup){
            out<< "\t"<< std::setw(LOG_WIDTH)<< groupMembersIds[i]<< std::setw(LOG_WIDTH)<< committeeMembersIds[i]<< std::endl;
        }else if(_printCommittee){
            out<< "\t"<< std::setw(LOG_WIDTH)<< ""<< std::setw(LOG_WIDTH)<< committeeMembersIds[i]<< std::endl;
        }else if(_printGroup){
            out<< "\t"<< std::setw(LOG_WIDTH)<< groupMembersIds[i]<< std::setw(LOG_WIDTH)<< ""<< std::endl;
        }
    }
    
    return out;
}
