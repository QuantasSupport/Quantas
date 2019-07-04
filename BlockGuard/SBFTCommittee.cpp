//
//  SBFTCommittee.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 7/1/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "SBFTCommittee.hpp"

SBFTCommittee::SBFTCommittee(std::vector<syncBFT_Peer*> peers){
    _consensusReached = false;
    _peers = peers;
}

SBFTCommittee::SBFTCommittee(const SBFTCommittee &rhs){
    _consensusReached = rhs._consensusReached;
    _peers = rhs._peers;
    
}

SBFTCommittee& SBFTCommittee::operator=(const SBFTCommittee &rhs){
    if(&rhs == this){
        return *this;
    }
    
    _consensusReached = rhs._consensusReached;
    _peers = rhs._peers;
    return *this;
}

bool SBFTCommittee::checkForConsensus()const{
    bool consensusReached = true;
    for(auto peer = _peers.begin(); peer != _peers.end(); peer++){
        if(!(*peer)->isTerminated()){
            consensusReached = false;
        }
    }
    return consensusReached;
}

void SBFTCommittee::preformComputation(){
    for(auto peer = _peers.begin(); peer != _peers.end(); peer++){
        (*peer)->preformComputation();
    }
}

void SBFTCommittee::receive(){
    for(auto peer = _peers.begin(); peer != _peers.end(); peer++){
        (*peer)->receive();
    }
}

void SBFTCommittee::transmit(){
    for(auto peer = _peers.begin(); peer != _peers.end(); peer++){
        (*peer)->transmit();
    }
}


