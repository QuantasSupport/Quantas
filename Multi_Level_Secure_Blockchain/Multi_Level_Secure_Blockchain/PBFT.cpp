//
//  PBFT.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/20/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "PBFT.hpp"

PBFT::PBFT(){
        _currentView = 0;
        _primary = nullptr;
        _messageLog = {};
        _faultUpperBound = 0;
}

PBFT::PBFT(int f){
    _currentView = 0;
    _primary = nullptr;
    _messageLog = {};
    _faultUpperBound = f;
}

PBFT::PBFT(const PBFT &rhs){
    _currentView = rhs._currentView;
    _primary = rhs._primary;
    _messageLog = rhs._messageLog;
    _faultUpperBound = rhs._faultUpperBound;
}
