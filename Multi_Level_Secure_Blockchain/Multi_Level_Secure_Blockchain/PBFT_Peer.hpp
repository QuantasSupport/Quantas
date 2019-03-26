//
//  PBFT_Peer.hpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/19/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef PBFT_Peer_hpp
#define PBFT_Peer_hpp

#include <math.h>
#include "Peer.hpp"

//
// PBFT Message defintion
//

// These are the type defintions for the messages
static const std::string REQUEST = "REQUEST";
static const std::string REPLY   = "REPLY";

// These are the phase type defintions (same as peer state)
static const std::string IDEAL         = "IDEAL";
static const std::string PRE_PREPARE   = "PRE-PREPARE";
static const std::string PREPARE       = "PREPARE";
static const std::string COMMIT        = "COMMIT";
static const std::string COMMIT_WAIT   = "COMMIT_WAIT";

// operation defintions
static const char ADD = '+';
static const char SUBTRACT = '-';

struct PBFT_Message{
    //////////////////////////////////////////
    // request info
    
    // the client is the peer that submited the request
    std::string         client_id;
    // this is the peer that created the message
    std::string         creator_id;
    int                 view;
    std::string         type;
    char                operation;
    std::pair<int,int>  operands;
    int                 resulte;
    int                 round; // used instead of timestamp
    
    //////////////////////////////////////////
    // phases info
    std::string         phase;
    int                 sequenceNumber;
};

//
// PBFT Peer defintion
//
class PBFT_Peer : public Peer<PBFT_Message>{
protected:
    
    // tracking varables
    std::vector<PBFT_Message>       _messageLog;
    std::vector<PBFT_Message>       _requestLog;
    double                          _faultUpperBound;
    std::vector<PBFT_Message>       _ledger;
    int                             _currentRound; // this is the peers clock
    
    // status varables
    Peer<PBFT_Message>*             _primary;
    std::string                     _currentPhase;
    int                             _currentView;
    PBFT_Message                    _currentRequest;
    int                             _currentRequestResulte;
    
    //
    // protected methds for PBFT execution inside the peer
    //
    
    // main methods used in preformComputation
    void                        collectRequest      ();             // primary collecting requests
    void                        prePrepare          ();             // phase 1 pre-prepare
    void                        prepare             ();             // phase 2 prepare
    void                        waitPrepare         ();             // wait for 1/3F + 1 prepare msgs
    void                        commit              ();             // phase 3 commit
    void                        waitCommit          ();             // wait for 1/3F + 1 commit msgs ends distributed-consensus
    
    // support methods used for the above
    Peer<PBFT_Message>*         findPrimary         (const std::vector<Peer<PBFT_Message>*> peers);
    int                         executeQuery        (const PBFT_Message);
    std::string                 makePckId           ()const                                         { return "Peer ID:"+_id + " round:" + std::to_string(_currentRound);};
    bool                        isVailedRequest     (const PBFT_Message)const;
    void                        braodcast           (const PBFT_Message);
    
public:
    PBFT_Peer                                       (std::string id);
    PBFT_Peer                                       (std::string id, double fault);
    PBFT_Peer                                       (std::string id, double fault, int round);
    PBFT_Peer                                       (const PBFT_Peer &rhs);
    ~PBFT_Peer                                      ()                                              {};
    
    std::vector<PBFT_Message>   getLedger           ()const                                         {return _ledger;};
    std::vector<PBFT_Message>   getMessageLog       ()const                                         {return _messageLog;};
    
    void                        setFaultTolerance   (double f)                                      {_faultUpperBound = f;};
    PBFT_Peer&                  operator=           (const PBFT_Peer &);
    std::ostream&               printTo             (std::ostream&)const;
    //void                        log                 ()const                                         {printTo(_log);};
    
    void                        preformComputation  ();// numberOfRoundsPerRequest as one request per X number of rounds
    void                        makeRequest         ();// start distributed-consensus

    friend std::ostream&        operator<<          (std::ostream &o, const PBFT_Peer &p)           {p.printTo(o); return o;};
};

#endif /* PBFT_Peer_hpp */
