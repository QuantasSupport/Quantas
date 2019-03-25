//
//  PBFT_Peer.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/19/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

//

#include "PBFT_Peer.hpp"

PBFT_Peer::PBFT_Peer(std::string id) : Peer<PBFT_Message>(id){
    _faultUpperBound = 0;
    _currentRound = 0;
    _messageLog = {};
    _primary = nullptr;
    _currentPhase = IDEAL;
    _currentView = 0;
    _currentRequestResulte = 0;
    _ledger = {};
    _requestLog = {};
    _currentRequest = {};
}

PBFT_Peer::PBFT_Peer(std::string id, double fault) : Peer<PBFT_Message>(id){
    _faultUpperBound = fault;
    _currentRound = 0;
    _messageLog = {};
    _primary = nullptr;
    _currentPhase = IDEAL;
    _currentView = 0;
    _currentRequestResulte = 0;
    _ledger = {};
    _requestLog = {};
    _currentRequest = {};
}

PBFT_Peer::PBFT_Peer(std::string id, double fault, int round) : Peer<PBFT_Message>(id){
    _faultUpperBound = fault;
    _currentRound = round;
    _messageLog = {};
    _primary = nullptr;
    _currentPhase = IDEAL;
    _currentView = 0;
    _currentRequestResulte = 0;
    _ledger = {};
    _requestLog = {};
    _currentRequest = {};
}

PBFT_Peer::PBFT_Peer(const PBFT_Peer &rhs) : Peer<PBFT_Message>(rhs){
    _faultUpperBound = rhs._faultUpperBound;
    _currentRound = rhs._currentRound;
    _messageLog =rhs._messageLog;
    _primary = rhs._primary;
    _currentPhase = rhs._currentPhase;
    _currentView = rhs._currentView;
    _currentRequestResulte = rhs._currentRequestResulte;
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
    _currentRequestResulte = rhs._currentRequestResulte;
    _ledger = rhs._ledger;
    _requestLog = rhs._requestLog;
    _currentRequest = rhs._currentRequest;
    
    return *this;
}

void PBFT_Peer::preformComputation(int numberOfRoundsPerRequest){
    if(numberOfRoundsPerRequest == 0){
        return;
    }
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
        std::cout<< "ERROR: makeRequest called with no primary"<< std::endl;
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
    request.resulte = 0;
    
    // create packet for request
    Packet<PBFT_Message> pck(makePckId());
    pck.setSource(_id);
    pck.setTarget(_primary->id());
    pck.setBody(request);
    _outStream.push_back(pck);
}

void PBFT_Peer::prePrepare(){
    if(_currentPhase != IDEAL){
        return;
    }
    if(_requestLog.empty()){
        return;
    }
    PBFT_Message request = _requestLog.front();
    _requestLog.erase(_requestLog.begin());
    
    request.sequenceNumber = (int)_ledger.size() + 1;
    request.phase = PRE_PREPARE;
    _currentPhase = PREPARE;
    _currentRequest = request;
    _currentRequestResulte = executeQuery(request);
    braodcast(request);
}

void PBFT_Peer::prepare(){
    if(_currentPhase != IDEAL && _currentPhase != PRE_PREPARE){
        return;
    }
    PBFT_Message prePrepareMesg = {};
    while(!_inStream.empty()){
        if(_inStream.front().getMessage().phase == PRE_PREPARE){
            prePrepareMesg = _inStream.front().getMessage();
            _inStream.clear();
        }else{
            _inStream.erase(_inStream.begin());
        }
    }
    if(!isVailedRequest(prePrepareMesg)){
        return;
    }
    PBFT_Message prepareMsg = prePrepareMesg;
    prepareMsg.creator_id = _id;
    prepareMsg.view = _currentView;
    prepareMsg.type = REPLY;
    prepareMsg.round = _currentRound;
    prepareMsg.phase = PREPARE;
    prepareMsg.resulte = executeQuery(prepareMsg);
    
    _messageLog.push_back(prePrepareMesg);
    _messageLog.push_back(prepareMsg);
    
    braodcast(prepareMsg);
    _currentPhase = PREPARE;
    _currentRequest = prepareMsg;
    _currentRequestResulte = executeQuery(prepareMsg);
}

void PBFT_Peer::waitPrepare(){
    if(_currentPhase != PREPARE){
        return;
    }
    while(!_inStream.empty()){
        if(_inStream.front().getMessage().phase != PREPARE){
            _inStream.erase(_inStream.begin());
        } else
        if(_inStream.front().getMessage().view != _currentView){
            _inStream.erase(_inStream.begin());
        }else{
            _messageLog.push_back(_inStream.front().getMessage());
            _inStream.erase(_inStream.begin());
        }
    }
    
    int numberOfPrepareMsg = 0;
    for(int i = 0; i < _messageLog.size(); i++){
        if(_messageLog[i].sequenceNumber == _currentRequest.sequenceNumber && _messageLog[i].client_id == _currentRequest.client_id){
            if(_messageLog[i].resulte == _currentRequestResulte){
                numberOfPrepareMsg++;
            }
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
    commitMsg.resulte = _currentRequestResulte;
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
        if(_inStream.front().getMessage().phase != COMMIT){
            _inStream.erase(_inStream.begin());
        }
        if(_inStream.front().getMessage().view != _currentView){
            _inStream.erase(_inStream.begin());
        }
        _messageLog.push_back(_inStream.front().getMessage());
        _inStream.erase(_inStream.begin());;
    }
    
    int numberOfCommitMsg = 0;
    for(int i = 0; i < _messageLog.size(); i++){
        if(_messageLog[i].sequenceNumber == _currentRequest.sequenceNumber && _messageLog[i].client_id == _currentRequest.client_id){
            if(_messageLog[i].resulte == _currentRequestResulte){
                numberOfCommitMsg++;
            }
        }
    }
    if(numberOfCommitMsg > (_neighbors.size() * _faultUpperBound) + 1){
        _ledger.push_back(_currentRequest);
        _currentRequest = {}; // clear old request
        _currentRequestResulte = 0; // clear old resulte
        _currentPhase = IDEAL; // complete distributed-consensus
    }
    
}

Peer<PBFT_Message>* PBFT_Peer::findPrimary(const std::map<Peer<PBFT_Message> *, int> neighbors){
    
    std::map<Peer<PBFT_Message> *, int> peers = neighbors;
    peers.insert(std::pair<Peer<PBFT_Message> *, int>(this,0));
    int peerIteration = _currentView%peers.size();
    
    auto it =  peers.begin();
    std::advance (it,peerIteration);
    Peer<PBFT_Message>* primary = it->first;
    
    return primary;
}

void PBFT_Peer::collectRequest(){
    if(_primary->id() != _id){
        return;
    }
    for(int i = 0; i < _inStream.size(); i++){
        if(_inStream[i].getMessage().type == REQUEST){
            _requestLog.push_back(_inStream[i].getMessage());
            _inStream.erase(_inStream.begin()+i);
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
            std::cout<< "ERROR: invailed request excution"<< std::endl;
            return 0;
            break;
    }
}

bool PBFT_Peer::isVailedRequest(const PBFT_Message query)const{
    if(query.view != _currentView){
        return false;
    }
    if(!_messageLog.empty()){
        if(query.sequenceNumber <= _messageLog.back().sequenceNumber){
            return false;
        }
    }
    if(query.phase != PRE_PREPARE){
        return false;
    }
    return true;
}

void PBFT_Peer::braodcast(const PBFT_Message msg){
    for(auto it = _neighbors.begin(); it != _neighbors.end(); it++){
        Peer<PBFT_Message>* neighbor = it->first;
        Packet<PBFT_Message> pck(makePckId());
        pck.setSource(_id);
        pck.setTarget(neighbor->id());
        pck.setBody(msg);
        _outStream.push_back(pck);
    }
}

