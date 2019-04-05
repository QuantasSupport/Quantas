//
//  NetworkBGS.hpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 4/3/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

////////////////////////////////
// NetworkBGS takes on the role of the refrence/utility commitee
//

#ifndef BGSReferenceCommittee_hpp
#define BGSReferenceCommittee_hpp

#include "Network.hpp"
#include "BlockGuardPeer_Sharded.hpp"
#include <iostream>
#include <random>
#include <stdio.h>

static double SECURITY_LEVEL_7 = 1; // whole system
static double SECURITY_LEVEL_6 = SECURITY_LEVEL_7/2; // half of system
static double SECURITY_LEVEL_5 = SECURITY_LEVEL_6/2; // quarter of system
static double SECURITY_LEVEL_4 = SECURITY_LEVEL_5/2; // so on ...
static double SECURITY_LEVEL_3 = SECURITY_LEVEL_4/2;
static double SECURITY_LEVEL_2 = SECURITY_LEVEL_3/2;
static double SECURITY_LEVEL_1 = SECURITY_LEVEL_2/2; // for 1000 peers and group size of 10 this will be ~1.5625 round down for one committee

struct BGSrequest{
    double securityLevel;
    int id;
};

typedef std::vector<BlockGuardPeer_Sharded*> aGroup;
class BGSReferenceCommittee{
protected:
    int                                                             _currentRound;
    int                                                             _groupSize;
    int                                                             _nextCommitteeId;
    Network<PBFT_Message, BlockGuardPeer_Sharded>                   _peers;
    std::vector<int>                                                _groupIds;
    std::vector<std::pair<int,aGroup> >                             _busyGroups;
    std::vector<std::pair<int,aGroup> >                             _freeGroups;
    std::vector<BGSrequest>                                         _requestQueue;
    
    std::ostream                                                    *_log;
    std::default_random_engine                                      _randomGenerator;
    
    // util functions
    BGSrequest              generateRequest         ();
    void                    makeGroup               (std::vector<BlockGuardPeer_Sharded*>,int);
    double                  pickSecrityLevel        ();
    void                    makeCommittee           (std::vector<std::pair<int,aGroup> >);
    void                    initCommittee           (std::vector<std::pair<int,aGroup> >);
    
public:
    BGSReferenceCommittee                           ();
    BGSReferenceCommittee                           (const BGSReferenceCommittee&);
    ~BGSReferenceCommittee                          ()                                                          {};
    
    // setters
    void                    setGroupSize            (int g)                                                     {_groupSize = g;};
    void                    setFaultTolerance       (double);
    void                    setLog                  (std::ostream &o)                                           {_log = &o; _peers.setLog(o);}
    
    // getters
    int                     getGroupSize            ()const                                                     {return _groupSize;};
    int                     numberOfGroups          ()const                                                     {return _groupIds.size();};
    int                     size                    ()const                                                     {return _peers.size();}
    aGroup                  getGroup                (int)const;
    std::vector<int>        getGroupIds             ()const                                                     {return _groupIds;};
    
    // mutators
    void                    initNetwork             (int);
    void                    makeRequest             ();
    
    // pass-through to Network class
    void                    receive                 ()                                                          {_peers.receive();};
    void                    preformComputation      ()                                                          {_peers.preformComputation(); _currentRound++;};
    void                    transmit                ()                                                          {_peers.transmit();};
    void                    setMaxDelay             (int d)                                                     {_peers.setMaxDelay(d);};
    void                    setAvgDelay             (int d)                                                     {_peers.setAvgDelay(d);};
    void                    setMinDelay             (int d)                                                     {_peers.setMinDelay(d);};
    void                    setToPoisson            ()                                                          {_peers.setToPoisson();};
    void                    setToOne                ()                                                          {_peers.setToOne();};
    void                    setToRandom             ()                                                          {_peers.setToRandom();};
    
    
    // logging and debugging
    std::ostream&           printTo                 (std::ostream&)const;
    void                    log                     ()const                                                     {printTo(*_log);};
    
    // operators
    BGSReferenceCommittee&  operator=               (const BGSReferenceCommittee&);
    BlockGuardPeer_Sharded* operator[]              (int i)                                                     {return _peers[i];};
    friend std::ostream&    operator<<              (std::ostream &out, const BGSReferenceCommittee &system)    {return system.printTo(out);};

};

#endif /* BGSReferenceCommittee_hpp */
