//
//  syncBFTReferenceCommittee.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 7/1/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef SBFTReferenceCommittee_hpp
#define SBFTReferenceCommittee_hpp

#include <stdio.h>
#include <map>
#include <vector>
#include <algorithm>
#include <random>
#include "syncBFT_Peer.hpp"
#include "syncBFT_Committee.hpp"
#include "ByzantineNetwork.hpp"

//phase mining or collecting
const std::string MINING             = "MINING";
const std::string COLLECTING         = "COLLECTING";
const std::string WAITING_FOR_TX     = "WAITING_FOR_TX";

struct SBFTTransactionRequest{
    double securityLevel;
    int submissionRound;
};

typedef std::vector<syncBFT_Peer*> SBFTGroup;
typedef std::pair<syncBFTmessage,int> SBFTLedgerEntery;
typedef std::pair<std::string, syncBFT_Committee> SBFTState; // committee state (collecting mining ect) and committee

class SBFTReferenceCommittee{
protected:
    // secrity level is the number of groups needed for a committee
    // 5 is high (all groups )1 is low (4 groups for 1024 peers)
    // initialization in initNetwork
    double _securityLevel5;
    double _securityLevel4;
    double _securityLevel3;
    double _securityLevel2;
    double _securityLevel1;
    
    int                                                 _groupSize;
    int                                                 _waitTime;
    int                                                 _clock;
    int                                                 _totalSubmissions;
    ByzantineNetwork<syncBFTmessage, syncBFT_Peer>      _peers;
    std::deque<SBFTGroup>                               _groups;
    std::deque<SBFTTransactionRequest>                  _requestQueue;
    std::vector<SBFTState>                              _activeCommittees;
    
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
    
public:
    SBFTReferenceCommittee                                      ();
    SBFTReferenceCommittee                                      (const SBFTReferenceCommittee&);
    ~SBFTReferenceCommittee                                     (){};
    
    // mutators
    void                                initNetwork             (int);
    void                                makeRequest             (int secLevel = -1);
    void                                shuffleByzantines       (int n);
    
    // setters
    void                                setGroupSize            (int s)                                 {_groupSize = s;};
    void                                setLog                  (std::ostream &o)                       {_log = &o; _peers.setLog(o);}
    
    // getters
    int                                 securityLevel5          ()const                                 {return _securityLevel5;};
    int                                 securityLevel4          ()const                                 {return _securityLevel4;};
    int                                 securityLevel3          ()const                                 {return _securityLevel3;};
    int                                 securityLevel2          ()const                                 {return _securityLevel2;};
    int                                 securityLevel1          ()const                                 {return _securityLevel1;};
    
    int                                 secLevel5Defeated       ()const                                 {return _secLevel5Defeated;};
    int                                 secLevel4Defeated       ()const                                 {return _secLevel4Defeated;};
    int                                 secLevel3Defeated       ()const                                 {return _secLevel3Defeated;};
    int                                 secLevel2Defeated       ()const                                 {return _secLevel2Defeated;};
    int                                 secLevel1Defeated       ()const                                 {return _secLevel1Defeated;};
    
    int                                 getGroupSize            ()const                                 {return _groupSize;};
    int                                 totalSubmissions        ()const                                 {return _totalSubmissions;};
    int                                 size                    ()const                                 {return _peers.size();}
    
    std::vector<SBFTState>              activeCommittees        ()const                                 {return _activeCommittees;};
    
    // metrics
    std::vector<DAGBlock>               getGlobalLedger         ()const;
    void                                preformComputation      ();
    void                                receive                 ();
    void                                transmit                ();
    
    // logging and debugging
    std::ostream&                       printTo                 (std::ostream &out)const                {_peers.printTo(out); return out;};
    void                                log                     ()const                                 {printTo(*_log);};
    
    // operators
    SBFTReferenceCommittee&             operator=               (const SBFTReferenceCommittee&);
    syncBFT_Peer*                       operator[]              (int i)                                 {return _peers[i];};
    const syncBFT_Peer*                 operator[]              (int i)const                            {return _peers[i];};
    friend std::ostream&                operator<<              (std::ostream &out, // continued on next line
                                                                 const SBFTReferenceCommittee &system)  {return system.printTo(out);};
    
    // pass-through to ByzantineNetwork class
    void                                setMaxDelay             (int d)                                 {_peers.setMaxDelay(d);};
    void                                setAvgDelay             (int d)                                 {_peers.setAvgDelay(d);};
    void                                setMinDelay             (int d)                                 {_peers.setMinDelay(d);};
    void                                setToPoisson            ()                                      {_peers.setToPoisson();};
    void                                setToOne                ()                                      {_peers.setToOne();};
    void                                setToRandom             ()                                      {_peers.setToRandom();};
    std::vector<syncBFT_Peer*>          getByzantine            ()const                                 {return _peers.getByzantine();};
    std::vector<syncBFT_Peer*>          getCorrect              ()const                                 {return _peers.getCorrect();};
    int                                 maxDelay                ()const                                 {return _peers.maxDelay();};
    void                                makeByzantines          (int n)                                 {_peers.makeByzantines(n);};
    void                                makeCorrect             (int n)                                 {_peers.makeCorrect(n);};
    void                                makePeerByzantines      (int i)                                 {_peers.makePeerByzantines(i);};
    void                                makePeerCorrect         (int i)                                 {_peers.makePeerByzantines(i);};
};

#endif /* SBFTReferenceCommittee_hpp */
