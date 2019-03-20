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
    _PBFTinstance = PBFT();
}

PBFT_Peer::PBFT_Peer(const PBFT_Peer &rhs){
    _PBFTinstance = rhs._PBFTinstance;
}

PBFT_Peer& PBFT_Peer::operator=(const PBFT_Peer &rhs){
    
    Peer<PBFT_Message>::operator=(rhs);
    
    _PBFTinstance = rhs._PBFTinstance;
    
    return *this;
}

