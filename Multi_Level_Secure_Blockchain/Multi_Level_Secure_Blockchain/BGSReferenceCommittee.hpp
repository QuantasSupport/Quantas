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
#include "PBFTPeer_Sharded.hpp"
#include <iostream>
#include <random>
#include <stdio.h>


static double SECURITY_LEVEL_5 = 6;
static double SECURITY_LEVEL_4 = 5;
static double SECURITY_LEVEL_3 = 4;
static double SECURITY_LEVEL_2 = 3;
static double SECURITY_LEVEL_1 = 2;

struct BGSrequest{
    double securityLevel;
    int id;
};

typedef std::vector<PBFTPeer_Sharded*> aGroup;
class BGSReferenceCommittee{
protected:
    int                                                             _currentRound;
    int                                                             _groupSize;
    int                                                             _nextCommitteeId;
    Network<PBFT_Message, PBFTPeer_Sharded>                         _peers;
    std::vector<int>                                                _groupIds;
    std::vector<std::pair<int,aGroup> >                             _busyGroups;
    std::vector<std::pair<int,aGroup> >                             _freeGroups;
    std::vector<BGSrequest>                                         _requestQueue;
    
    std::ostream                                                    *_log;
    std::default_random_engine                                      _randomGenerator;
    
    // util functions
    BGSrequest                          generateRequest         ();
    void                                makeGroup               (std::vector<PBFTPeer_Sharded*>,int);
    double                              pickSecrityLevel        ();
    void                                makeCommittee           (std::vector<std::pair<int,aGroup> >);
    void                                initCommittee           (std::vector<std::pair<int,aGroup> >);
    void                                updateBusyGroup         ();
public:
    BGSReferenceCommittee                                       ();
    BGSReferenceCommittee                                       (const BGSReferenceCommittee&);
    ~BGSReferenceCommittee                                      ()                                                          {};
    
    // setters
    void                                setGroupSize            (int g)                                                     {_groupSize = g;};
    void                                setFaultTolerance       (double);
    void                                setLog                  (std::ostream &o)                                           {_log = &o; _peers.setLog(o);}
    
    // getters
    int                                 getGroupSize            ()const                                                     {return _groupSize;};
    int                                 numberOfGroups          ()const                                                     {return (int)_groupIds.size();};
    int                                 size                    ()const                                                     {return _peers.size();}
    aGroup                              getGroup                (int)const;
    std::vector<int>                    getGroupIds             ()const                                                     {return _groupIds;};
    std::vector<PBFTPeer_Sharded>       getPeers                ()const;
    
    // mutators
    void                                initNetwork             (int);
    void                                makeRequest             ();
    
    // pass-through to Network class
    void                                receive                 ()                                                          {_peers.receive();};
    void                                preformComputation      ()                                                          {_peers.preformComputation(); _currentRound++;};
    void                                transmit                ()                                                          {_peers.transmit();};
    void                                setMaxDelay             (int d)                                                     {_peers.setMaxDelay(d);};
    void                                setAvgDelay             (int d)                                                     {_peers.setAvgDelay(d);};
    void                                setMinDelay             (int d)                                                     {_peers.setMinDelay(d);};
    void                                setToPoisson            ()                                                          {_peers.setToPoisson();};
    void                                setToOne                ()                                                          {_peers.setToOne();};
    void                                setToRandom             ()                                                          {_peers.setToRandom();};
    
    
    // logging and debugging
    std::ostream&                       printTo                 (std::ostream&)const;
    void                                log                     ()const                                                     {printTo(*_log);};
    
    // operators
    BGSReferenceCommittee&              operator=               (const BGSReferenceCommittee&);
    PBFTPeer_Sharded*                   operator[]              (int i)                                                     {return _peers[i];};
    const PBFTPeer_Sharded*             operator[]              (int i)const                                                {return _peers[i];};
    friend std::ostream&                operator<<              (std::ostream &out, const BGSReferenceCommittee &system)    {return system.printTo(out);};

};

#endif /* BGSReferenceCommittee_hpp */
