//
//  BlockGuardSharded.hpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/28/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef BlockGuardSharded_hpp
#define BlockGuardSharded_hpp

#include <stdio.h>
#include "PBFT_Peer.hpp"

static double SECURITY_LEVEL_1 = 1;
static double SECURITY_LEVEL_2 = 0.5;
static double SECURITY_LEVEL_3 = 0.25;
static double SECURITY_LEVEL_4 = 0.125;
static double SECURITY_LEVEL_5 = 0.0625;

class BlockGuardPeer_Sharded : public PBFT_Peer{
protected:
    
    int                                                        _groupId;
    int                                                        _committeeId;
    
    std::vector<Peer<PBFT_Message>* >                          _groupMembers;
    std::vector<Peer<PBFT_Message>* >                          _committeeMembers;
    
    int                     faultyPeers                        ()const                                                  {return ceil((_committeeMembers.size() + 1) * _faultUpperBound);};
    void                    braodcast                          (const PBFT_Message&);
    
public:
    BlockGuardPeer_Sharded                                     (std::string);
    BlockGuardPeer_Sharded                                     (const BlockGuardPeer_Sharded&);
    ~BlockGuardPeer_Sharded                                    ()                                                       {};
    
    //setters
    void                    setGroup                           (int id)                                                 {_groupId = id;};
    void                    setCommittee                       (int id)                                                 {_committeeId = id;};
    void                    addGroupMember                     (BlockGuardPeer_Sharded &newMember)                      {_groupMembers.push_back(&newMember);};
    void                    addcommitteeMember                 (BlockGuardPeer_Sharded &newMember)                      {_committeeMembers.push_back(&newMember);};
   
    // mutators
    void                    clearCommittee                     ()                                                       {_committeeMembers.clear(); _committeeId = -1;}
    void                    clearGroup                         ()                                                       {_groupMembers.clear(); _groupId = -1;}
    
    void                    preformComputation                 ();
    
    std::ostream&           printTo                            (std::ostream&)const;
    void                    log                                ()const                                                  {printTo(*_log);};
    
    BlockGuardPeer_Sharded& operator=                          (const BlockGuardPeer_Sharded&);
    friend std::ostream&    operator<<                         (std::ostream &o, const BlockGuardPeer_Sharded &p)       {p.printTo(o); return o;};
};

#endif /* BlockGuardSharded_hpp */
