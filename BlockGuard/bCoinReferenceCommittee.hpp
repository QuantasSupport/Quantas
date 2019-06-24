//
//  bCoinReferenceCommittee.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 6/13/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef bCoinReferenceCommittee_hpp
#define bCoinReferenceCommittee_hpp

#include <stdio.h>
#include <map>
#include <deque>
#include <vector>
#include <unordered_set>
#include <random>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <set>
#include "DS_bCoin_Peer.hpp"
#include "ByzantineNetwork.hpp"
#include "bCoin_Committee.hpp"

struct bCoinTransactionRequest{
    double securityLevel;
    int id;
};

typedef std::vector<DS_bCoin_Peer*> bCoinGroup;
typedef std::pair<DS_bCoinMessage,int> bCoinledgerEntery; // includes committee size as the int

class bCoinReferenceCommittee{
protected:
    
    // secrity level is the number of groups needed for a committee
    // 5 is high (all groups )1 is low (4 groups for 1024 peers)
    // initialization in initNetwork
    double _securityLevel5;
    double _securityLevel4;
    double _securityLevel3;
    double _securityLevel2;
    double _securityLevel1;
    
    std::default_random_engine                          _randomGenerator;
    int                                                 _groupSize;
    int                                                 _nextId;
    ByzantineNetwork<DS_bCoinMessage, DS_bCoin_Peer>    _peers;
    std::map<int,bCoinGroup>                            _groups;
    std::deque<bCoinTransactionRequest>                 _requestQueue;
    std::vector<bCoin_Committee>                        _currentCommittees;
    
    // logging and metrics
    std::ostream                                        *_log;
    bool                                                _printNetwork;
    int                                                 _secLevel5Defeated;
    int                                                 _secLevel4Defeated;
    int                                                 _secLevel3Defeated;
    int                                                 _secLevel2Defeated;
    int                                                 _secLevel1Defeated;
    int                                                 _totalSubmitoins;
    
    // util
    int                                 getRandomSecLevel       ();
    void                                cleanupCommittee        ();
    
public:
    bCoinReferenceCommittee                                     ();
    bCoinReferenceCommittee                                     (const bCoinReferenceCommittee&);
    ~bCoinReferenceCommittee                                    ()                                      {};
    
    // setters
    void                                setLog                  (std::ostream &o)                       {_log = &o; _peers.setLog(o);}
    void                                setGroupSize            (int s)                                 {_groupSize = s;};
    
    // getters
    std::vector<bCoinGroup>             getFreeGroups           ()const;
    std::deque<bCoinTransactionRequest> getRequestQueue         ()const;
    int                                 totalSubmissions        ()const                                 {return _totalSubmitoins;};
    double                              securityLevel5          ()const                                 {return _securityLevel5;};
    double                              securityLevel4          ()const                                 {return _securityLevel4;};
    double                              securityLevel3          ()const                                 {return _securityLevel3;};
    double                              securityLevel2          ()const                                 {return _securityLevel2;};
    double                              securityLevel1          ()const                                 {return _securityLevel1;};
    int                                 getGroupSize            ()const                                 {return _groupSize;};
    std::vector<bCoinGroup>             getFreeGroups            ();
    
    // mutators
    void                                initNetwork             (int);
    void                                makeRequest             (int secLevel = -1);
    void                                shuffleByzantines       (int n);
    
    // metrics
    std::vector<DAGBlock>               getGlobalLedger         ()const;
    void                                receive                 ();
    void                                preformComputation      ();
    void                                transmit                ();
    
    // operators
    std::ostream&                       printTo                 (std::ostream&)const;
    void                                log                     ()const                                 {printTo(*_log);};
    bCoinReferenceCommittee&            operator=               (const bCoinReferenceCommittee&);
    DS_bCoin_Peer*                      operator[]              (int i)                                 {return _peers[i];};
    const DS_bCoin_Peer*                operator[]              (int i)const                            {return _peers[i];};
    friend std::ostream&                operator<<              (std::ostream &out, // continued on next line
                                                                 const bCoinReferenceCommittee&system)  {return system.printTo(out);};
    // pass-through to ByzantineNetwork class
    int                                 size                    ()const                                 {return _peers.size();};
    void                                setMaxDelay             (int d)                                 {_peers.setMaxDelay(d);};
    void                                setAvgDelay             (int d)                                 {_peers.setAvgDelay(d);};
    void                                setMinDelay             (int d)                                 {_peers.setMinDelay(d);};
    void                                setToPoisson            ()                                      {_peers.setToPoisson();};
    void                                setToOne                ()                                      {_peers.setToOne();};
    void                                setToRandom             ()                                      {_peers.setToRandom();};
    std::vector<DS_bCoin_Peer*>         getByzantine            ()const                                 {return _peers.getByzantine();};
    std::vector<DS_bCoin_Peer*>         getCorrect              ()const                                 {return _peers.getCorrect();};
    void                                makeByzantines          (int n)                                 {_peers.makeByzantines(n);};
    void                                makeCorrect             (int n)                                 {_peers.makeCorrect(n);};
    void                                makePeerByzantines      (int i)                                 {_peers.makePeerByzantines(i);};
    void                                makePeerCorrect         (int i)                                 {_peers.makePeerByzantines(i);};
    
    
};
#endif /* bCoinReferenceCommittee_hpp */
