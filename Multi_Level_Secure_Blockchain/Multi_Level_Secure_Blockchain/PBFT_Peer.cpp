//
//  PBFT_Peer.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/19/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <string>
#include <vector>
#include "PBFT_Peer.hpp"


PBFT_Peer::PBFT_Peer(std::string id, int faltBound) : Peer(id){
    _currentView = 0;
    _primary = nullptr;
    _messageLog = {};
    _faltUpperBound = faltBound;
}

PBFT_Peer::PBFT_Peer(const PBFT_Peer &rhs){
    _currentView = rhs._currentView;
    _primary = rhs._primary;
    _messageLog = rhs._messageLog;
    _faltUpperBound = rhs._faltUpperBound;
}

void PBFT_Peer::findPrimary(){
    int primaryIndex = _currentView%_neighbors.size();
    auto it = _neighbors.begin();
    for(int i = 0; i < primaryIndex; i++){
        it++;
    }
    _primary = it->first;
}

PBFT_Peer& PBFT_Peer::operator=(const PBFT_Peer &rhs){
    
    Peer<PBFT_BasicMessage>::operator=(rhs);
    
    _currentView = rhs._currentView;
    _primary = rhs._primary;
    _messageLog = rhs._messageLog;
    _faltUpperBound = rhs._faltUpperBound;
    
    return *this;
}

