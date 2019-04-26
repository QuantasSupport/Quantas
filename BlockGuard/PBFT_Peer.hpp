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
#include <algorithm>
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
static const std::string PREPARE_WAIT  = "PREPARE_WAIT"; // waiting foir messages
static const std::string COMMIT        = "COMMIT";
static const std::string COMMIT_WAIT   = "COMMIT_WAIT"; // waiting foir messages

static const std::string NO_PRIMARY    = "NO PRIMARY";

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
    int                 result;
    int                 round; // used instead of timestamp
    
    //////////////////////////////////////////
    // phases info
    std::string         phase;
    int                 sequenceNumber;
    
    bool operator==(const PBFT_Message& rhs)
    {
        return(
                client_id == rhs.client_id &&
                creator_id == rhs.creator_id &&
                view == rhs.view &&
                type == rhs.type &&
                operation == rhs.operation &&
                operands == rhs.operands &&
                result == rhs.result &&
                round == rhs.round
               );
    }
};

//
// PBFT Peer defintion
//
class PBFT_Peer : public Peer<PBFT_Message>{
protected:
    
    // tracking varables
    std::vector<PBFT_Message>       _requestLog;
    std::vector<PBFT_Message>       _prePrepareLog;
    std::vector<PBFT_Message>       _prepareLog;
    std::vector<PBFT_Message>       _commitLog;
    std::vector<PBFT_Message>       _ledger;
    
    double                          _faultUpperBound;
    int                             _currentRound; // this is the peers clock
    
    // status varables
    Peer<PBFT_Message>*             _primary;
    std::string                     _currentPhase;
    int                             _currentView;
    PBFT_Message                    _currentRequest;
    int                             _currentRequestResult;
    
    //
    // protected methds for PBFT execution inside the peer
    //
    
    // main methods used in preformComputation
    void                        collectMessages     ();             // sorts messages from _inStream into there respective logs
    
    void                        prePrepare          ();             // phase 1 pre-prepare
    void                        prepare             ();             // phase 2 prepare
    void                        waitPrepare         ();             // wait for 1/3F + 1 prepare msgs
    void                        commit              ();             // phase 3 commit
    void                        waitCommit          ();             // wait for 1/3F + 1 commit msgs ends distributed-consensus
    
    // support methods used for the above
    virtual void                sendCommitReply     ();
    virtual Peer<PBFT_Message>* findPrimary         (const std::vector<Peer<PBFT_Message>*> peers);
    virtual int                 executeQuery        (const PBFT_Message&);
    std::string                 makePckId           ()const                                         { return "Peer ID:"+_id + " round:" + std::to_string(_currentRound);};
    virtual bool                isVailedRequest     (const PBFT_Message&)const;
    virtual void                braodcast           (const PBFT_Message&);
    void                        cleanLogs           (); // clears logs for all transactions in _ledger
    
public:
    PBFT_Peer                                       (std::string id);
    PBFT_Peer                                       (std::string id, double fault);
    PBFT_Peer                                       (const PBFT_Peer &rhs);
    ~PBFT_Peer                                      ()                                              {};
    
    // getters
    std::vector<PBFT_Message>   getRequestLog       ()const                                         {return _requestLog;};
    std::vector<PBFT_Message>   getPrePrepareLog    ()const                                         {return _prePrepareLog;};
    std::vector<PBFT_Message>   getPrepareLog       ()const                                         {return _prepareLog;};
    std::vector<PBFT_Message>   getCommitLog        ()const                                         {return _commitLog;};
    std::vector<PBFT_Message>   getLedger           ()const                                         {return _ledger;};
    std::string                 getPhase            ()const                                         {return _currentPhase;};
    bool                        isPrimary           ()const                                         {return _primary == nullptr ? false : _id == _primary->id();};
    virtual int                 faultyPeers         ()const                                         {return ceil((_neighbors.size() + 1) * _faultUpperBound);};
    int                         getRound            ()const                                         {return _currentRound;};
    std::string                 getPrimary          ()const                                         {return _primary == nullptr ? NO_PRIMARY : _primary->id();}
    
    // setters
    void                        setFaultTolerance   (double f)                                      {_faultUpperBound = f;};
 
    // mutators
    void                        clearPrimary        ()                                              {_primary = nullptr;}
    void                        init                ()                                              {initPrimary();};
    virtual void                initPrimary         ()                                              {_primary = findPrimary(_neighbors);};
    
    // debug/logging
    std::ostream&               printTo             (std::ostream&)const;
    void                        log                 ()const                                         {printTo(*_log);};
    
    // base class functions
    void                        preformComputation  ();
    void                        makeRequest         ();// starts distributed-consensus
    void                        makeRequest         (int);// starts distributed-consensus with a squence number

    // operators
    PBFT_Peer&                  operator=           (const PBFT_Peer &);
    friend std::ostream&        operator<<          (std::ostream &o, const PBFT_Peer &p)           {p.printTo(o); return o;};
};

#endif /* PBFT_Peer_hpp */
