//
//  PBFT_Peer.hpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/19/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef PBFT_Peer_hpp
#define PBFT_Peer_hpp

#include <stdio.h>
#include "Peer.hpp"

struct PBFTMessage{
    
    std::string aPeerId;
    std::string message;
    
};

class PBFT_Peer : public Peer<PBFTMessage>{
protected:
    int         _viewNumber;
    std::string _primary;
public:
    PBFT_Peer(std::string);
    PBFT_Peer(const PBFT_Peer &rhs);
    ~PBFT_Peer();
    
    void                preformComputation();
    
    std::string         primary           ()  {return _primary;};
    
    void                findPrimary       ();
};

#endif /* PBFT_Peer_hpp */
