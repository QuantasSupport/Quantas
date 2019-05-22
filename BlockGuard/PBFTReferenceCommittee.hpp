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

struct transactionRequest{
    double securityLevel;
    int id;
};

////////////////////////
// typedef fore group
typedef std::vector<PBFTPeer_Sharded*> aGroup;

class PBFTReferenceCommittee{
protected:

    // secrity level is the number of groups needed for a committee
    // 5 is high (all groups )1 is low (4 groups for 1024 peers)
    // initialization in initNetwork
    double _securityLevel5;
    double _securityLevel4;
    double _securityLevel3;
    double _securityLevel2;
    double _securityLevel1;

    int                                                             _currentRound;
    int                                                             _groupSize;
    int                                                             _nextCommitteeId;
    int                                                             _nextSquenceNumber;
    double                                                          _faultTolerance;
    Network<PBFT_Message, PBFTPeer_Sharded>                         _peers;
    std::vector<int>                                                _groupIds;
    std::vector<int>                                                _busyGroups;
    std::vector<int>                                                _freeGroups;
    std::vector<transactionRequest>                                 _requestQueue;
    std::map<int,aGroup>                                            _groups;

    // logging, metrics and untils
    std::ostream                                                    *_log;
    std::default_random_engine                                      _randomGenerator;
    std::vector<int>                                                _currentCommittees;

    // util functions
    transactionRequest                  generateRequest         ();
    transactionRequest                  generateRequest         (int); // generate a request of a specific security level
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
    int                                 getNextCommitteeId      ()const                                 {return _nextCommitteeId;};
    double                              securityLevel5          ()const                                 {return _securityLevel5;}
    double                              securityLevel4          ()const                                 {return _securityLevel4;}
    double                              securityLevel3          ()const                                 {return _securityLevel3;}
    double                              securityLevel2          ()const                                 {return _securityLevel2;}
    double                              securityLevel1          ()const                                 {return _securityLevel1;}

    aGroup                              getGroup                (int);
    std::vector<int>                    getGroupIds             ()const                                 {return _groupIds;};
    std::vector<PBFTPeer_Sharded>       getPeers                ()const;
    std::vector<int>                    getBusyGroups           ()const                                 {return _busyGroups;};
    std::vector<int>                    getFreeGroups           ()const                                 {return _freeGroups;};
    std::vector<int>                    getCurrentCommittees    ()const                                 {return _currentCommittees;};
    std::vector<transactionRequest>     getRequestQueue         ()const                                 {return _requestQueue;}
    std::vector<aGroup>                 getCommittee            (int);
    
    // mutators
    void                                initNetwork             (int);
    void                                serveRequest            ();
    void                                makeRequest             ()                                      {queueRequest();serveRequest();};
    void                                makeRequest             (int securityLevel)                     {queueRequest(securityLevel);serveRequest();};
    void                                queueRequest            ()                                      {_requestQueue.push_back(generateRequest());}; 
    void                                queueRequest            (int securityLevel)                     {_requestQueue.push_back(generateRequest(securityLevel));};
    void                                clearQueue              ()                                      {_requestQueue.clear();}  
    
    // pass-through to Network class
    void                                receive                 ()                                      {_peers.receive();};
    void                                preformComputation      ()                                      {_peers.preformComputation();updateBusyGroup(); _currentRound++;};
    void                                transmit                ()                                      {_peers.transmit();};
    void                                setMaxDelay             (int d)                                 {_peers.setMaxDelay(d);};
    void                                setAvgDelay             (int d)                                 {_peers.setAvgDelay(d);};
    void                                setMinDelay             (int d)                                 {_peers.setMinDelay(d);};
    void                                setToPoisson            ()                                      {_peers.setToPoisson();};
    void                                setToOne                ()                                      {_peers.setToOne();};
    void                                setToRandom             ()                                      {_peers.setToRandom();};
    void                                shuffleByzantines       (int n)                                 {_peers.shuffleByzantines(n);};
    std::vector<PBFTPeer_Sharded*>      getByzantine            ()const                                 {return _peers.getByzantine();};
    std::vector<PBFTPeer_Sharded*>      getCorrect              ()const                                 {return _peers.getCorrect();};
    void                                makeByzantines          (int n)                                 {_peers.makeByzantines(n);};
    void                                makeCorrect             (int n)                                 {_peers.makeCorrect(n);};
    
    // logging and debugging
    std::ostream&                       printTo                 (std::ostream&)const;
    void                                log                     ()const                                 {printTo(*_log);};
    std::vector<PBFT_Message>           getGlobalLedger         ()const;
    void                                setMaxSecurityLevel     (int); // used to fix max security as number of groups for debugging
    void                                setMinSecurityLevel     (int); // used to fix min security as number of groups for debugging
    
    // operators
    PBFTReferenceCommittee&             operator=               (const PBFTReferenceCommittee&);
    PBFTPeer_Sharded*                   operator[]              (int i)                                 {return _peers[i];};
    const PBFTPeer_Sharded*             operator[]              (int i)const                            {return _peers[i];};
    friend std::ostream&                operator<<              (std::ostream &out, // continued on next line
                                                                const PBFTReferenceCommittee&system)    {return system.printTo(out);};

};

#endif /* PBFTReferenceCommittee_hpp */
