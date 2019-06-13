//
//  PBFT_Peer.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/19/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <limits>
#include "PBFT_Peer.hpp"

PBFT_Peer::PBFT_Peer(std::string id) : Peer<PBFT_Message>(id){
    _requestLog = std::list<PBFT_Message>();
    _prePrepareLog = std::list<PBFT_Message>();
    _prepareLog = std::list<PBFT_Message>();
    _commitLog = std::list<PBFT_Message>();
    _ledger = std::list<PBFT_Message>();
    
    _faultUpperBound = 0;
    _currentRound = 0;
    
    _primary = nullptr;
    _currentPhase = IDEAL;
    _currentView = 0;
    _currentRequest = PBFT_Message();
    _currentRequestResult = 0;
}

PBFT_Peer::PBFT_Peer(std::string id, double fault) : Peer<PBFT_Message>(id){
    _requestLog = std::list<PBFT_Message>();
    _prePrepareLog = std::list<PBFT_Message>();
    _prepareLog = std::list<PBFT_Message>();
    _commitLog = std::list<PBFT_Message>();
    _ledger = std::list<PBFT_Message>();
    
    _faultUpperBound = fault;
    _currentRound = 0;
    
    _primary = nullptr;
    _currentPhase = IDEAL;
    _currentView = 0;
    _currentRequest = PBFT_Message();
    _currentRequestResult = 0;
}

PBFT_Peer::PBFT_Peer(const PBFT_Peer &rhs) : Peer<PBFT_Message>(rhs){
    
    if(this == &rhs){
        return;
    }
    
    _requestLog = rhs._requestLog;
    _prePrepareLog = rhs._prePrepareLog;
    _prepareLog = rhs._prepareLog;
    _commitLog = rhs._commitLog;
    _ledger = rhs._ledger;
    
    _faultUpperBound = rhs._faultUpperBound;
    _currentRound = rhs._currentRound;
    
    _primary = rhs._primary;
    _currentPhase = rhs._currentPhase;
    _currentView = rhs._currentView;
    _currentRequest = rhs._currentRequest;
    _currentRequestResult = rhs._currentRequestResult;
}

PBFT_Peer& PBFT_Peer::operator=(const PBFT_Peer &rhs){
    
    if(this == &rhs){
        return *this;
    }

    Peer<PBFT_Message>::operator=(rhs);
    
    _requestLog = rhs._requestLog;
    _prePrepareLog = rhs._prePrepareLog;
    _prepareLog = rhs._prepareLog;
    _commitLog = rhs._commitLog;
    _ledger = rhs._ledger;
    
    _faultUpperBound = rhs._faultUpperBound;
    _currentRound = rhs._currentRound;
    
    _primary = rhs._primary;
    _currentPhase = rhs._currentPhase;
    _currentView = rhs._currentView;
    _currentRequest = rhs._currentRequest;
    _currentRequestResult = rhs._currentRequestResult;
    
    return *this;
}

void PBFT_Peer::collectMessages(){
    while(!_inStream.empty()){
        if(_inStream.front().getMessage().type == REQUEST && _primary->id() == _id){
            _requestLog.push_back(_inStream.front().getMessage());
            _inStream.erase(_inStream.begin());
            
        }else if(_inStream.front().getMessage().phase == PRE_PREPARE){
            _prePrepareLog.push_back(_inStream.front().getMessage());
            _inStream.erase(_inStream.begin());
            
        }else if(_inStream.front().getMessage().phase == PREPARE){
            _prepareLog.push_back(_inStream.front().getMessage());
            _inStream.erase(_inStream.begin());
            
        }else if(_inStream.front().getMessage().phase == COMMIT){
            _commitLog.push_back(_inStream.front().getMessage());
            _inStream.erase(_inStream.begin());
            
        }else{
            _inStream.erase(_inStream.begin());
        }
    }
}

void PBFT_Peer::cleanLogs(int sequenceNumber){

    int oldTransaction = sequenceNumber;
    auto entry = _prePrepareLog.begin();
    while(entry != _prePrepareLog.end()){
        if(entry->sequenceNumber == oldTransaction){
            _prePrepareLog.erase(entry++);
        }else{
            entry++;
        }
    }
    
    entry = _prepareLog.begin();
    while(entry != _prepareLog.end()){
        if(entry->sequenceNumber == oldTransaction){
            _prepareLog.erase(entry++);
        }else{
            entry++;
        }
    }
    
    entry = _commitLog.begin();
    while(entry != _commitLog.end()){
        if(entry->sequenceNumber == oldTransaction){
            _commitLog.erase(entry++);
        }else{
            entry++;
        }
    }
}

void PBFT_Peer::prePrepare(){
    if(_currentPhase != IDEAL ||
       _requestLog.empty() ||
       _primary->id() != _id){
        return;
    }

    PBFT_Message request = _requestLog.front();
    _requestLog.erase(_requestLog.begin());
    
    if(request.sequenceNumber == -1){
        request.sequenceNumber = (int)_ledger.size() + 1;
    }else{
        // request came with a sequenceNumber number nothing to do
    }
    
    request.phase = PRE_PREPARE;
    request.type = REPLY;
    request.creator_id = _id;
    request.byzantine = _byzantine;
    _currentPhase = PREPARE_WAIT;
    _currentRequest = request;
    _currentRequestResult = executeQuery(request);
    PBFT_Message myPrepareMsg = request;
    myPrepareMsg.phase = PREPARE;
    _prepareLog.push_back(myPrepareMsg);
    braodcast(request);
}

void PBFT_Peer::prepare(){
    if(_currentPhase != IDEAL){
        return;
    }
    PBFT_Message prePrepareMesg;
    if(!_prePrepareLog.empty()){
        prePrepareMesg = _prePrepareLog.front();
        _prePrepareLog.erase(_prePrepareLog.begin());
    }else{
        return;
    }
    if(!isVailedRequest(prePrepareMesg)){
        return;
    }
    prePrepareMesg.phase = PREPARE;
    _prepareLog.push_back(prePrepareMesg);
    PBFT_Message prepareMsg = prePrepareMesg;
    prepareMsg.creator_id = _id;
    prepareMsg.view = _currentView;
    prepareMsg.type = REPLY;
    prepareMsg.commit_round = _currentRound;
    prepareMsg.phase = PREPARE;
    prepareMsg.byzantine = _byzantine;
    _prepareLog.push_back(prepareMsg);
    braodcast(prepareMsg);
    _currentPhase = PREPARE_WAIT;
    _currentRequest = prePrepareMesg;
}

void PBFT_Peer::waitPrepare(){
    if(_currentPhase != PREPARE_WAIT){
        return;
    }
    int numberOfPrepareMsg = 0;
    for(auto entry = _prepareLog.begin(); entry != _prepareLog.end(); entry++){
        if(entry->sequenceNumber == _currentRequest.sequenceNumber
           && entry->view == _currentView){
            numberOfPrepareMsg++;
        }
    }
    if(numberOfPrepareMsg >= (faultyPeers())){
        _currentPhase = COMMIT;
    }
}

void PBFT_Peer::commit(){
    if(_currentPhase != COMMIT){
        return;
    }
    
    PBFT_Message commitMsg = _currentRequest;
    _currentRequestResult = executeQuery(_currentRequest);
    
    commitMsg.phase = COMMIT;
    commitMsg.creator_id = _id;
    commitMsg.type = REPLY;
    commitMsg.result = _currentRequestResult;
    commitMsg.commit_round = _currentRound;
    commitMsg.byzantine = _byzantine;
    _commitLog.push_back(commitMsg);
    braodcast(commitMsg);
    _currentPhase = COMMIT_WAIT;
}

void PBFT_Peer::waitCommit(){
    if(_currentPhase != COMMIT_WAIT){
        return;
    }
    int numberOfCommitMsg = 0;
    for(auto entry = _commitLog.begin(); entry != _commitLog.end(); entry++){
        if(entry->sequenceNumber == _currentRequest.sequenceNumber
           && entry->view == _currentView){
            numberOfCommitMsg++;
        }
    }
    // if we have enough commit messages
    if(numberOfCommitMsg >= faultyPeers()){
        commitRequest();
    }
}

void PBFT_Peer::commitRequest(){
    PBFT_Message commit = _currentRequest;
    commit.result = _currentRequestResult;
    commit.commit_round = _currentRound;
    // count the number of byzantine and correct commits
    // if more then f peers match leader commit otherwise view change (byz peers force view change until leader is byz)
    
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
    if(_currentRequest.byzantine){
        if( numberOfByzantineCommits >= faultyPeers()){
            commit.defeated = true;
            _ledger.push_back(commit);
            _currentRequest = PBFT_Message();
            _currentRequest = PBFT_Message();
        }else if (numberOfByzantineCommits + correctCommitMsg != _neighbors.size() +1){
            return;
        }else{
            if(_currentView + 1 == _neighbors.size() + 1){
                commit.defeated = true;
                _ledger.push_back(commit);
                _currentRequest = PBFT_Message();
                _currentRequest = PBFT_Message();
            }
            viewChange(_neighbors);
        }
    }else if(!_currentRequest.byzantine){
        if( correctCommitMsg >= faultyPeers()){
            commit.defeated = false;
            _ledger.push_back(commit);
            _currentRequest = PBFT_Message();
            _currentRequest = PBFT_Message();
        }else if (numberOfByzantineCommits + correctCommitMsg != _neighbors.size() +1){
            return;
        }else{
            if(_currentView + 1 == _neighbors.size() + 1){
                commit.defeated = true;
                _ledger.push_back(commit);
                _currentRequest = PBFT_Message();
                _currentRequest = PBFT_Message();
            }
            viewChange(_neighbors);
        }
    }
    
    for(auto confirmedTransaction = _ledger.begin(); confirmedTransaction != _ledger.end(); confirmedTransaction++){
        cleanLogs(confirmedTransaction->sequenceNumber);
    }
    _currentPhase = IDEAL; // complete distributed-consensus
    _currentRequestResult = 0;
}

void PBFT_Peer::viewChange(std::map<std::string, Peer<PBFT_Message>* > potentialPrimarys){
    _currentView++;
    _primary = findPrimary(potentialPrimarys);
    if(_primary->id() == _id){
        PBFT_Message request;
        request = _currentRequest;
        request.view = _currentView;
        request.byzantine = _byzantine;
        _requestLog.push_back(request);
    }
}

Peer<PBFT_Message>* PBFT_Peer::findPrimary(const std::map<std::string, Peer<PBFT_Message> *> neighbors){
    
    std::vector<std::string> peerIds = std::vector<std::string>();
    for (auto it =neighbors.begin(); it != neighbors.end(); ++it){
        Peer<PBFT_Message>* neighbor = it->second;
        peerIds.push_back(neighbor->id());
    }
    peerIds.push_back(_id);
    
    std::sort(peerIds.begin(), peerIds.end());
    std::string primaryId = peerIds[_currentView%peerIds.size()];
    
    if(primaryId == _id){
        return this;
    }else{
        return _neighbors[primaryId];
    }
}

int PBFT_Peer::executeQuery(const PBFT_Message &query){
    switch (query.operation) {
        case ADD:
            return query.operands.first + query.operands.second;
            break;
            
        case SUBTRACT:
            return query.operands.first - query.operands.second;
            break;
            
        default:
            *_log<< "ERROR: invailed request excution"<< std::endl;
            return 0;
            break;
    }
}

bool PBFT_Peer::isVailedRequest(const PBFT_Message &query)const{
    if(query.view != _currentView){
        return false;
    }
    if(query.phase != PRE_PREPARE){
        return false;
    }
    return true;
}

void PBFT_Peer::braodcast(const PBFT_Message &msg){
    for (auto it =_neighbors.begin(); it != _neighbors.end(); ++it){
        std::string neighborId = it->first;
        Packet<PBFT_Message> pck(makePckId());
        pck.setSource(_id);
        pck.setTarget(neighborId);
        pck.setBody(msg);
        _outStream.push_back(pck);
    }
}

void PBFT_Peer::preformComputation(){
    if(_primary == nullptr){
        _primary = findPrimary(_neighbors);
    }
    _currentRound++;
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

void PBFT_Peer::makeRequest(){
    if(_primary == nullptr){
        *_log<< "ERROR: makeRequest called with no primary"<< std::endl;
        return;
    }
    // create request
    PBFT_Message request;
    request.submission_round = _currentRound;
    request.client_id = _id;
    request.creator_id = _id;
    request.view = _currentView;
    request.type = REQUEST;
    
    bool add = (rand()%2);
    if(add){
        request.operation = ADD;
    }else{
        request.operation = SUBTRACT;
    }
    
    request.operands = std::pair<int, int>();
    request.operands.first = (rand()%100)+1;
    request.operands.second = (rand()%100)+1;
    
    request.commit_round = _currentRound;
    request.phase = IDEAL;
    request.sequenceNumber = -1;
    request.result = 0;
    request.byzantine = _byzantine;
    
    if(_id != _primary->id()){
        sendRequest(request);
    }else{
        _requestLog.push_back(request);
    }
}

void PBFT_Peer::sendRequest(PBFT_Message request){
    if(_id != _primary->id()){
        // create packet for request
        Packet<PBFT_Message> pck(makePckId());
        pck.setSource(_id);
        pck.setTarget(_primary->id());
        pck.setBody(request);
        _outStream.push_back(pck);
    }
}

void PBFT_Peer::makeRequest(int squenceNumber){
    if(_primary == nullptr){
        *_log<< "ERROR: makeRequest called with no primary"<< std::endl;
        return;
    }
    
    // create request
    PBFT_Message request;
    request.submission_round = _currentRound;
    request.client_id = _id;
    request.creator_id = _id;
    request.view = _currentView;
    request.type = REQUEST;
    
    bool add = (rand()%2);
    if(add){
        request.operation = ADD;
    }else{
        request.operation = SUBTRACT;
    }
    
    request.operands = std::pair<int, int>();
    request.operands.first = (rand()%100)+1;
    request.operands.second = (rand()%100)+1;
    
    request.commit_round = _currentRound;
    request.phase = IDEAL;
    request.sequenceNumber = squenceNumber;
    request.result = 0;
    request.byzantine = _byzantine;
    
    // if this is primary then dont send to primary
    // if this is busy then dont send to primary
    if(_currentPhase != IDEAL && id() != _primary->id()){
        sendRequest(request);
    }else{
        _requestLog.push_back(request); // this is either the primary or is free to send a new request
    }
}

std::ostream& PBFT_Peer::printTo(std::ostream &out)const{
    Peer<PBFT_Message>::printTo(out);
    out<< std::left;
    
    std::string primaryId;
    if(_primary == nullptr){
        primaryId = NO_PRIMARY;
    }else{
        primaryId = _primary->id();
    }
    
    out<< "\t"<< "Settings:"<< std::endl;
    out<< "\t"<< std::setw(LOG_WIDTH)<< "Fault Upper Bound"<< std::endl;
    out<< "\t"<< std::setw(LOG_WIDTH)<< _faultUpperBound<< std::endl;
    
    out<< "\t"<< "Current State:"<< std::endl;
    out<< "\t"<< std::setw(LOG_WIDTH)<< "Round"<< std::setw(LOG_WIDTH)<< "Current Phase"<< std::setw(LOG_WIDTH)<< "Current View"<< std::setw(LOG_WIDTH)<< "Primary ID"<< std::setw(LOG_WIDTH)<< "Current Request Client ID"<< std::setw(LOG_WIDTH)<< "Current Request Result"<< std::endl;
    out<< "\t"<< std::setw(LOG_WIDTH)<< _currentRound<< std::setw(LOG_WIDTH)<< _currentPhase<< std::setw(LOG_WIDTH)<< _currentView<< std::setw(LOG_WIDTH)<< primaryId<< std::setw(LOG_WIDTH)<< _currentRequest.client_id<< std::setw(LOG_WIDTH)<< _currentRequestResult<< std::endl;
    
    out<< "\t"<< std::setw(LOG_WIDTH)<< "Request Log"<< std::setw(LOG_WIDTH)<< "Pre-Prepare Log Size"<< std::setw(LOG_WIDTH)<< "Prepare Log Size"<< std::setw(LOG_WIDTH)<< "Commit Log Size"<< std::setw(LOG_WIDTH)<< "Ledger Size"<<  std::endl;
    out<< "\t"<< std::setw(LOG_WIDTH)<< _requestLog.size()<< std::setw(LOG_WIDTH)<< _prePrepareLog.size()<< std::setw(LOG_WIDTH)<< _prepareLog.size()<< std::setw(LOG_WIDTH)<< _commitLog.size()<< std::setw(LOG_WIDTH)<< _ledger.size()<< std::endl;
    
    return out;
}
