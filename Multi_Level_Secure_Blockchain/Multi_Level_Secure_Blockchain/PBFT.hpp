//
//  PBFT.hpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/20/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef PBFT_hpp
#define PBFT_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include "Peer.hpp"
//
// Basic Message defintion
//

// These are the type defintions for the messages
const std::string REQUEST = "REQUEST";
const std::string REPLY   = "REPLY";

// These are the phase type defintions
const std::string PRE_PREPARE   = "PRE-PREPARE";
const std::string PREPARE       = "PREPARE";
const std::string COMMIT        = "COMMIT";

struct PBFT_Message{
    //////////////////////////////////////////
    // request info
    
    // the client is the peer that submited the request
    std::string         client_id;
    // this is the peer that created the message
    std::string         peer_id;
    int                 view;
    std::string         type;
    std::string         operation;
    std::pair<int,int>  operands;
    int                 round; // used instead of timestamp
    
    //////////////////////////////////////////
    // phases info
    std::string         phase;
    int                 sequenceNumber;
};

class PBFT{
    
protected:
    
    std::vector<PBFT_Message>   _messageLog;
    int                              _faultUpperBound;
    
    Peer<PBFT_Message>*         _primary;
    std::string                      _currentPhase;
    int                              _currentView;
    
public:
    PBFT                    ();
    PBFT                    (int);
    PBFT                    (const PBFT&);
    ~PBFT                   ();
    
    PBFT&   operator=       (const PBFT&);
};

#endif /* PBFT_hpp */
