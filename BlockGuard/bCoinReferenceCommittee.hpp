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
#include "DS_bCoin_Peer.hpp"
#include "ByzantineNetwork.hpp"

struct transactionRequest{
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
    std::deque<transactionRequest>                      _requestQueue;
    
    // logging and metrics
    std::ostream                                        *_log;
    bool                                                _printNetwork;
    
    // util
    std::vector<bCoinGroup>            getFreeGroups            ();
    
public:
    bCoinReferenceCommittee                                     ();
    bCoinReferenceCommittee                                     (const bCoinReferenceCommittee&);
    ~bCoinReferenceCommittee                                    ()                                      {};
    
    // setters
    void                                setLog                  (std::ostream &o)                       {_log = &o; _peers.setLog(o);}
    void                                setGroupSize            (int s)                                 {_groupSize = s;};
    
    // getters
    std::vector<bCoinGroup>             getFreeGroups           ()const;
    std::deque<transactionRequest>      getRequestQueue         ()const;
    
    // mutators
    void                                initNetwork             (int);
    void                                makeRequest             (transactionRequest);
    
    // metrics
    std::vector<bCoinledgerEntery>      getGlobalLedger         ()const;
    
    // operators
    std::ostream&                       printTo                 (std::ostream&)const;
    bCoinReferenceCommittee&            operator=               (const bCoinReferenceCommittee&);
    DS_bCoin_Peer*                      operator[]              (int i)                                 {return _peers[i];};
    const DS_bCoin_Peer*                operator[]              (int i)const                            {return _peers[i];};
    friend std::ostream&                operator<<              (std::ostream &out, // continued on next line
                                                                 const bCoinReferenceCommittee&system)  {return system.printTo(out);};
    // pass-through to ByzantineNetwork class
    void                                receive                 ()                                      {_peers.receive();};
    void                                preformComputation      ()                                      {_peers.preformComputation();};
    void                                transmit                ()                                      {_peers.transmit();};
    void                                setMaxDelay             (int d)                                 {_peers.setMaxDelay(d);};
    void                                setAvgDelay             (int d)                                 {_peers.setAvgDelay(d);};
    void                                setMinDelay             (int d)                                 {_peers.setMinDelay(d);};
    void                                setToPoisson            ()                                      {_peers.setToPoisson();};
    void                                setToOne                ()                                      {_peers.setToOne();};
    void                                setToRandom             ()                                      {_peers.setToRandom();};
    void                                shuffleByzantines       (int n);
    std::vector<DS_bCoin_Peer*>         getByzantine            ()const                                 {return _peers.getByzantine();};
    std::vector<DS_bCoin_Peer*>         getCorrect              ()const                                 {return _peers.getCorrect();};
    void                                makeByzantines          (int n)                                 {_peers.makeByzantines(n);};
    void                                makeCorrect             (int n)                                 {_peers.makeCorrect(n);};
    void                                makePeerByzantines      (int i)                                 {_peers.makePeerByzantines(i);};
    void                                makePeerCorrect         (int i)                                 {_peers.makePeerByzantines(i);};
    
    
};
#endif /* bCoinReferenceCommittee_hpp */
