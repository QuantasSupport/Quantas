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
#include <vector>
#include <string>
#include "Peer.hpp"
#include "PBFT.hpp"

//
// PBFT Peer defintion
//
class PBFT_Peer : public Peer<PBFT_Message>{
protected:

    PBFT _PBFTinstance;
    
public:
    PBFT_Peer                               (std::string, int);
    PBFT_Peer                               (const PBFT_Peer &rhs);
    ~PBFT_Peer                              ();
    
    void                preformComputation  ();
    
    PBFT_Peer&          operator=           (const PBFT_Peer &);
    
};

#endif /* PBFT_Peer_hpp */
