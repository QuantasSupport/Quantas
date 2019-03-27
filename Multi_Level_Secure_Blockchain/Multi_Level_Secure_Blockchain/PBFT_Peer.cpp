//
//  PBFT_Peer.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/19/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <limits>
#include "PBFT_Peer.hpp"

void PBFT_Peer::prePrepare(){
    if(_currentPhase != IDEAL){
        return;
    }
    if(_requestLog.empty()){
        return;
    }
    if(_primary->id() != _id){
        return;
    }
    PBFT_Message request = _requestLog.front();
    _requestLog.clear();
    
    request.sequenceNumber = (int)_ledger.size() + 1;
    request.phase = PRE_PREPARE;
    _currentPhase = PREPARE;
    _currentRequest = request;
    _currentRequestResult = executeQuery(request);
    braodcast(request);
}

void PBFT_Peer::prepare(){
    if(_currentPhase != IDEAL && _currentPhase != PRE_PREPARE){
        return;
    }
    
    while(!_inStream.empty()){
        if(_inStream.front().getMessage().phase == PRE_PREPARE){
            _requestLog.push_back(_inStream.front().getMessage());
        }
        _messageLog.push_back(_inStream.front().getMessage());
        _inStream.erase(_inStream.begin());
    }
    PBFT_Message prePrepareMesg = _requestLog.front();
    if(!isVailedRequest(prePrepareMesg)){
        return ;
    }
    PBFT_Message prepareMsg = prePrepareMesg;
    prepareMsg.creator_id = _id;
    prepareMsg.view = _currentView;
    prepareMsg.type = REPLY;
    prepareMsg.round = _currentRound;
    prepareMsg.phase = PREPARE;
    prepareMsg.result = executeQuery(prepareMsg);
    
    _messageLog.push_back(prepareMsg);
    
    braodcast(prepareMsg);
    _currentPhase = PREPARE;
    _currentRequest = prepareMsg;
    _currentRequestResult = executeQuery(prepareMsg);
}

void PBFT_Peer::waitPrepare(){
    if(_currentPhase != PREPARE && _currentPhase != IDEAL){
        return;
    }
    while(!_inStream.empty()){
        _messageLog.push_back(_inStream.front().getMessage());
        _inStream.erase(_inStream.begin());
    }
    
    int numberOfPrepareMsg = 0;
    for(int i = 0; i < _messageLog.size(); i++){
        if(_messageLog[i].phase == PREPARE &&
           _messageLog[i].sequenceNumber == _currentRequest.sequenceNumber &&
           _messageLog[i].client_id == _currentRequest.client_id &&
           _messageLog[i].result == _currentRequestResult){
            numberOfPrepareMsg++;
        }else if(_messageLog[i].phase == IDEAL && isVailedRequest(_messageLog[i])){
            numberOfPrepareMsg++;
        }
    }
    // _neighbors.size() + 1 is neighbors plus this peer
    if(numberOfPrepareMsg > (ceil((_neighbors.size() + 1) * _faultUpperBound) + 1)){
        _currentPhase = COMMIT;
    }
}

void PBFT_Peer::commit(){
    if(_currentPhase != COMMIT){
        return ;
    }
    
    PBFT_Message commitMsg = _currentRequest;
    commitMsg.phase = COMMIT;
    commitMsg.creator_id = _id;
    commitMsg.view = _currentView;
    commitMsg.type = REPLY;
    commitMsg.result = _currentRequestResult;
    commitMsg.round = _currentRound;
    
    _messageLog.push_back(commitMsg);
    _currentRequest = commitMsg; // update request that it is in the commit phase
    braodcast(commitMsg);
    _currentPhase = COMMIT_WAIT;
}

void PBFT_Peer::waitCommit(){
    if(_currentPhase != COMMIT_WAIT){
        return;
    }
    while(!_inStream.empty()){
        _messageLog.push_back(_inStream.front().getMessage());
        _inStream.erase(_inStream.begin());
    }
    
    int numberOfCommitMsg = 0;
    for(int i = 0; i < _messageLog.size(); i++){
        if(_messageLog[i].phase == COMMIT &&
           _messageLog[i].sequenceNumber == _currentRequest.sequenceNumber &&
           _messageLog[i].client_id == _currentRequest.client_id &&
           _messageLog[i].result == _currentRequestResult){
                numberOfCommitMsg++;
        }
    }
    if(numberOfCommitMsg > ceil((_neighbors.size() + 1) * _faultUpperBound) + 1){
        _currentRequest.round = _currentRound;
        _ledger.push_back(_currentRequest);
        Packet<PBFT_Message> reply(makePckId());
        
        reply.setSource(_id);
        reply.setTarget(_currentRequest.client_id);
        reply.setBody(_currentRequest);
        _outStream.push_back(reply);
        _currentPhase = IDEAL; // complete distributed-consensus
    }
    
}

Peer<PBFT_Message>* PBFT_Peer::findPrimary(const std::vector<Peer<PBFT_Message> *> neighbors){
    
    std::vector<std::string> peerIds = std::vector<std::string>();
    for(int i = 0; i < neighbors.size(); i++){
        peerIds.push_back(neighbors[i]->id());
    }
    peerIds.push_back(_id);
    
    std::sort(peerIds.begin(), peerIds.end());
    std::string primaryId = peerIds[_currentView%peerIds.size()];
    
    if(primaryId == _id){
        return this;
    }
    for(int i = 0; i < neighbors.size(); i++){
        if(primaryId == neighbors[i]->id()){
            return neighbors[i];
        }
    }
    return nullptr;
}

void PBFT_Peer::collectRequest(){
    if(_primary->id() != _id){
        return;
    }
    int i = 0;
    while( i < _inStream.size()){
        if(_inStream[i].getMessage().type == REQUEST){
            _requestLog.push_back(_inStream[i].getMessage());
            _messageLog.push_back(_inStream[i].getMessage());
            _inStream.erase(_inStream.begin()+i);
        }else{
            i=i+1;
        }
    }
}

int PBFT_Peer::executeQuery(const PBFT_Message query){
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

bool PBFT_Peer::isVailedRequest(const PBFT_Message query)const{
    if(query.view != _currentView){
        return false;
    }
    if(!_ledger.empty()){
        if(query.sequenceNumber > _ledger.back().sequenceNumber){
            return false;
        }
    }
    if(query.phase != PRE_PREPARE){
        return false;
    }
    return true;
}

void PBFT_Peer::braodcast(const PBFT_Message msg){
    for(int i = 0; i < _neighbors.size(); i++){
        std::string neighborId = _neighbors[i]->id();
        Packet<PBFT_Message> pck(makePckId());
        pck.setSource(_id);
        pck.setTarget(neighborId);
        pck.setBody(msg);
        _outStream.push_back(pck);
    }
}
PBFT_Peer::PBFT_Peer(std::string id) : Peer<PBFT_Message>(id){
    _faultUpperBound = 0;
    _currentRound = 0;
    _messageLog = std::vector<PBFT_Message>();
    _primary = nullptr;
    _currentPhase = IDEAL;
    _currentView = 0;
    _currentRequestResult = 0;
    _ledger = std::vector<PBFT_Message>();
    _requestLog = std::vector<PBFT_Message>();
    _currentRequest = PBFT_Message();
}

PBFT_Peer::PBFT_Peer(std::string id, double fault) : Peer<PBFT_Message>(id){
    _faultUpperBound = fault;
    _currentRound = 0;
    _messageLog = std::vector<PBFT_Message>();
    _primary = nullptr;
    _currentPhase = IDEAL;
    _currentView = 0;
    _currentRequestResult = 0;
    _ledger = std::vector<PBFT_Message>();
    _requestLog = std::vector<PBFT_Message>();
    _currentRequest = PBFT_Message();
}

PBFT_Peer::PBFT_Peer(std::string id, double fault, int round) : Peer<PBFT_Message>(id){
    _faultUpperBound = fault;
    _currentRound = round;
    _messageLog = std::vector<PBFT_Message>();
    _primary = nullptr;
    _currentPhase = IDEAL;
    _currentView = 0;
    _currentRequestResult = 0;
    _ledger = std::vector<PBFT_Message>();
    _requestLog = std::vector<PBFT_Message>();
    _currentRequest = PBFT_Message();
}

PBFT_Peer::PBFT_Peer(const PBFT_Peer &rhs) : Peer<PBFT_Message>(rhs){
    _faultUpperBound = rhs._faultUpperBound;
    _currentRound = rhs._currentRound;
    _messageLog =rhs._messageLog;
    _primary = rhs._primary;
    _currentPhase = rhs._currentPhase;
    _currentView = rhs._currentView;
    _currentRequestResult = rhs._currentRequestResult;
    _ledger = rhs._ledger;
    _requestLog = rhs._requestLog;
    _currentRequest = rhs._currentRequest;
}

PBFT_Peer& PBFT_Peer::operator=(const PBFT_Peer &rhs){
    
    Peer<PBFT_Message>::operator=(rhs);
    _faultUpperBound = rhs._faultUpperBound;
    _currentRound = rhs._currentRound;
    _messageLog =rhs._messageLog;
    _primary = rhs._primary;
    _currentPhase = rhs._currentPhase;
    _currentView = rhs._currentView;
    _currentRequestResult = rhs._currentRequestResult;
    _ledger = rhs._ledger;
    _requestLog = rhs._requestLog;
    _currentRequest = rhs._currentRequest;
    
    return *this;
}

void PBFT_Peer::preformComputation(){
    if(_primary == nullptr){
        _primary = findPrimary(_neighbors);
    }
    collectRequest(); // will only excute if this peer is primary
    prePrepare();
    prepare();
    waitPrepare();
    commit();
    waitCommit();
    _currentRound++;
}

void PBFT_Peer::makeRequest(){
    if(_currentPhase != IDEAL){
        return;
    }
    if(_primary == nullptr){
        *_log<< "ERROR: makeRequest called with no primary"<< std::endl;
        return;
    }
    
    // create request
    PBFT_Message request;
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
    
    request.round = _currentRound;
    request.phase = IDEAL;
    request.sequenceNumber = -1;
    request.result = 0;
    
    // create packet for request
    Packet<PBFT_Message> pck(makePckId());
    pck.setSource(_id);
    pck.setTarget(_primary->id());
    pck.setBody(request);
    _messageLog.push_back(request);
    _outStream.push_back(pck);
}

std::ostream& PBFT_Peer::printTo(std::ostream &out)const{
    Peer<PBFT_Message>::printTo(out);
    out<< std::left;
    
    std::string primaryId;
    if(_primary == nullptr){
        primaryId = "NO PRIMARY";
    }else{
        primaryId = _primary->id();
    }
    
    out<< "\t"<< "Settings:"<< std::endl;
    out<< "\t"<< std::setw(LOG_WIDTH)<< "Fault Upper Bound"<< std::endl;
    out<< "\t"<< std::setw(LOG_WIDTH)<< _faultUpperBound<< std::endl;
    
    out<< "\t"<< "Current State:"<< std::endl;
    out<< "\t"<< std::setw(LOG_WIDTH)<< "Round"<< std::setw(LOG_WIDTH)<< "Current Phase"<< std::setw(LOG_WIDTH)<< "Current View"<< std::setw(LOG_WIDTH)<< "Primary ID"<< std::setw(LOG_WIDTH)<< "Current Request Client ID"<< std::setw(LOG_WIDTH)<< "Current Request Result"<< std::endl;
    out<< "\t"<< std::setw(LOG_WIDTH)<< _currentRound<< std::setw(LOG_WIDTH)<< _currentPhase<< std::setw(LOG_WIDTH)<< _currentView<< std::setw(LOG_WIDTH)<< primaryId<< std::setw(LOG_WIDTH)<< _currentRequest.client_id<< std::setw(LOG_WIDTH)<< _currentRequestResult<< std::endl;
    
    out<< "\t"<< std::setw(LOG_WIDTH)<< "Message Log Size"<< std::setw(LOG_WIDTH)<< "Request Log Size"<< "Ledger Size"<<  std::endl;
    out<< "\t"<< std::setw(LOG_WIDTH)<< _messageLog.size()<< std::setw(LOG_WIDTH)<< _requestLog.size()<< _ledger.size()<< std::endl <<std::endl;
    
//    out<< "\t"<< std::setw(LOG_WIDTH)<< "Message Log:"<< std::endl;
//
//    for(int i = 0; i < _messageLog.size(); i++){
//        out<< "\t"<< std::setw(LOG_WIDTH)<< "Index:"<< i<< std::endl;
//        out<< "\t\t"<< std::setw(LOG_WIDTH)<< "client_id:"<< _messageLog[i].client_id<< std::endl;
//        out<< "\t\t"<< std::setw(LOG_WIDTH)<< "creator_id:"<< _messageLog[i].creator_id<< std::endl;
//        out<< "\t\t"<< std::setw(LOG_WIDTH)<< "view:"<< _messageLog[i].view<< std::endl;
//        out<< "\t\t"<< std::setw(LOG_WIDTH)<< "type:"<< _messageLog[i].type<< std::endl;
//        out<< "\t\t"<< std::setw(LOG_WIDTH)<< "operation:"<< _messageLog[i].operation<< std::endl;
//        out<< "\t\t"<< std::setw(LOG_WIDTH)<< "operands:"<< _messageLog[i].operands.first<< ", "<< _messageLog[i].operands.second<< std::endl;
//        out<< "\t\t"<< std::setw(LOG_WIDTH)<< "result:"<< _messageLog[i].result<< std::endl;
//        out<< "\t\t"<< std::setw(LOG_WIDTH)<< "round:"<< _messageLog[i].round<< std::endl;
//        out<< "\t\t"<< std::setw(LOG_WIDTH)<< "phase:"<< _messageLog[i].phase<< std::endl;
//        out<< "\t\t"<< std::setw(LOG_WIDTH)<< "sequenceNumber:"<< _messageLog[i].sequenceNumber<< std::endl;
//    }
    
    return out;
}
