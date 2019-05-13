//
//  PBFTReferenceCommittee.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 4/3/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

////////////////////////////////
// NetworkBGS takes on the role of the refrence/utility commitee
//

#ifndef PBFTReferenceCommittee_hpp
#define PBFTReferenceCommittee_hpp

#include "Network.hpp"
#include "PBFTPeer_Sharded.hpp"
#include <iostream>
#include <random>
#include <stdio.h>
#include <assert.h>
#include <cassert>

// secrity level is the number of groups needed for a committee
// 5 is high (all groups )1 is low (4 groups for 1024 peers)
// initialization in initNetwork
static double SECURITY_LEVEL_5;
static double SECURITY_LEVEL_4;
static double SECURITY_LEVEL_3;
static double SECURITY_LEVEL_2;
static double SECURITY_LEVEL_1;

struct transactionRequest{
    double securityLevel;
    int id;
};

////////////////////////
// typedef fore group
typedef std::vector<PBFTPeer_Sharded*> aGroup;

class PBFTReferenceCommittee{
protected:
    int                                                             _currentRound;
    int                                                             _groupSize;
    int                                                             _nextCommitteeId;
    int                                                             _nextSquenceNumber;
    Network<PBFT_Message, PBFTPeer_Sharded>                         _peers;
    std::vector<int>                                                _groupIds;
    std::vector<int>                                                _busyGroups;
    std::vector<int>                                                _freeGroups;
    std::vector<transactionRequest>                                 _requestQueue;
    
    // logging, metrics and untils
    std::ostream                                                    *_log;
    std::default_random_engine                                      _randomGenerator;
    std::vector<int>                                                _currentCommittees;

    // util functions
    transactionRequest                  generateRequest         ();
    void                                makeGroup               (std::vector<PBFTPeer_Sharded*>,int);
    double                              pickSecrityLevel        ();
    void                                makeCommittee           (std::vector<int>);
    void                                initCommittee           (std::vector<int>);
    void                                updateBusyGroup         ();
    
public:
PBFTReferenceCommittee                                          ();
PBFTReferenceCommittee                                          (const PBFTReferenceCommittee&);
~PBFTReferenceCommittee                                         ()                                      {};
    
    // setters
    void                                setGroupSize            (int g)                                 {_groupSize = g;};
    void                                setFaultTolerance       (double);
    void                                setLog                  (std::ostream &o)                       {_log = &o; _peers.setLog(o);}
    
    // getters
    int                                 getGroupSize            ()const                                 {return _groupSize;};
    int                                 numberOfGroups          ()const                                 {return (int)_groupIds.size();};
    int                                 size                    ()const                                 {return _peers.size();}
    int                                 getNextSquenceNumber    ()const                                 {return _nextSquenceNumber;};
    int                                 getNestCommitteeId      ()const                                 {return _nextCommitteeId;};
    aGroup                              getGroup                (int)const;
    std::vector<int>                    getGroupIds             ()const                                 {return _groupIds;};
    std::vector<PBFTPeer_Sharded>       getPeers                ()const;
    std::vector<int>                    getBusyGroups           ()const                                 {return _busyGroups;};
    std::vector<int>                    getFreeGroups           ()const                                 {return _freeGroups;};
    std::vector<int>                    getCurrentCommittees    ()const                                 {return _currentCommittees;};
    std::vector<transactionRequest>     getRequestQueue         ()const                                 {return _requestQueue;}
    std::vector<aGroup>                 getCommittee            (int committeeId)const;
    // mutators
    void                                initNetwork             (int);
    void                                makeRequest             ();
    void                                queueRequest            ()                                      { _requestQueue.push_back(generateRequest());};  
    
    // pass-through to Network class
    void                                receive                 ()                                      {_peers.receive();};
    void                                preformComputation      ()                                      {updateBusyGroup();_peers.preformComputation(); _currentRound++;};
    void                                transmit                ()                                      {_peers.transmit();};
    void                                setMaxDelay             (int d)                                 {_peers.setMaxDelay(d);};
    void                                setAvgDelay             (int d)                                 {_peers.setAvgDelay(d);};
    void                                setMinDelay             (int d)                                 {_peers.setMinDelay(d);};
    void                                setToPoisson            ()                                      {_peers.setToPoisson();};
    void                                setToOne                ()                                      {_peers.setToOne();};
    void                                setToRandom             ()                                      {_peers.setToRandom();};
    
    
    // logging and debugging
    std::ostream&                       printTo                 (std::ostream&)const;
    void                                log                     ()const                                 {printTo(*_log);};
    std::vector<PBFT_Message>           getGlobalLedger         ()const;
    
    // operators
    PBFTReferenceCommittee&             operator=               (const PBFTReferenceCommittee&);
    PBFTPeer_Sharded*                   operator[]              (int i)                                 {return _peers[i];};
    const PBFTPeer_Sharded*             operator[]              (int i)const                            {return _peers[i];};
    friend std::ostream&                operator<<              (std::ostream &out, // continued on next line
                                                                const PBFTReferenceCommittee&system)    {return system.printTo(out);};

};

#endif /* PBFTReferenceCommittee_hpp */
