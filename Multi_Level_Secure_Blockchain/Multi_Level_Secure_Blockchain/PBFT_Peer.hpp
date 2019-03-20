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

//
// Basic Message defintion
//

// These are the type defintions for the messages
const std::string REQUEST = "REQUEST";
const std::string REPLY   = "REPLY";

struct PBFT_BasicMessage{
    
    // the client is the peer that submited the request
    std::string         client_id;
    // this is the peer that created the message
    std::string         peer_id;
    int                 view;
    std::string         type;
    std::string         operation;
    std::pair<int,int>  operands;
    int                 round; // used instead of timestamp
};

//
// Phase Message defintion
//

// These are the phase type defintions
const std::string PRE_PREPARE   = "PRE-PREPARE";
const std::string PREPARE       = "PREPARE";
const std::string COMMIT        = "COMMIT";

struct PBFT_PhasesMessage{
    std::string         phase;
    int                 view;
    int                 sequenceNumber;
    PBFT_BasicMessage   request;
};

//
// PBFT Peer defintion
//
class PBFT_Peer : public Peer<PBFT_BasicMessage>{
protected:

    std::vector<PBFT_BasicMessage>   _messageLog;
    int                              _faltUpperBound;
    
    Peer<PBFT_BasicMessage>*         _primary;
    std::string                      _currentPhase;
    int                              _currentView;
    
public:
    PBFT_Peer                               (std::string, int);
    PBFT_Peer                               (const PBFT_Peer &rhs);
    ~PBFT_Peer                              ();
    
    void                preformComputation  ();
    
    std::string         primary             ()  {return _primary->id();};
    
    void                findPrimary         ();
    
    PBFT_Peer&          operator=           (const PBFT_Peer &);
    
};

#endif /* PBFT_Peer_hpp */
