//
//  syncBFT_Peer_Sharded.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 4/9/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef syncBFT_Peer_Sharded_hpp
#define syncBFT_Peer_Sharded_hpp

#include <stdio.h>
#include "syncBFT_Peer.hpp"
#include <math.h>
#include <string>

class syncBFT_Peer_Sharded : syncBFT_Peer{
protected:
    int                                                         _groupId;
    int                                                         _committeeId;
    int                                                         _faultUpperBound;
    
    std::vector<Peer<syncBFTmessage>* >                         _groupMembers;
    std::vector<Peer<syncBFTmessage>* >                         _committeeMembers;
    
    int                     faultyPeers                         ()const                                             {return ceil((_committeeMembers.size() + 1) * _faultUpperBound);}; // fault bound
    void                    populateOutStream                   (const syncBFTmessage&); // braodcast
    std::vector<string>     leaderIdCandidates                  ()const;
public:
    syncBFT_Peer_Sharded                                        (std::string);
    syncBFT_Peer_Sharded                                        (const syncBFT_Peer_Sharded&);
    ~syncBFT_Peer_Sharded                                       ()                                                  {};
    
    // setters
    void                    setGroup                           (int id)                                           {_groupId = id;};
    void                    setCommittee                       (int id)                                           {_committeeId = id;};
    void                    addGroupMember                     (syncBFT_Peer_Sharded &newMember)                  {_groupMembers.push_back(&newMember);};
    void                    addcommitteeMember                 (syncBFT_Peer_Sharded &newMember)                  {_committeeMembers.push_back(&newMember);};
    
    // mutators
    void                    clearCommittee                     ()                                                 {_committeeMembers.clear(); _committeeId = -1;}
    void                    clearGroup                         ()                                                 {_groupMembers.clear(); _groupId = -1;}
    void                    initPrimary                        ()                                                 {leaderChange();};
    
    std::ostream&           printTo                            (std::ostream&)const;
    void                    log                                ()const                                            {printTo(*_log);};
    
    syncBFT_Peer_Sharded&   operator=                          (const syncBFT_Peer_Sharded&);
    friend std::ostream&    operator<<                         (std::ostream &o, const syncBFT_Peer_Sharded &p)   {p.printTo(o); return o;};
};

#endif /* syncBFT_Peer_Sharded_hpp */
