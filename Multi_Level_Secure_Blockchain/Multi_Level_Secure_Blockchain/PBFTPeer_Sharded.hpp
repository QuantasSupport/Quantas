//
//  BlockGuardSharded.hpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/28/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef PBFTPeer_Sharded_hpp
#define PBFTPeer_Sharded_hpp

#include <stdio.h>
#include "PBFT_Peer.hpp"

class PBFTPeer_Sharded : public PBFT_Peer{
protected:
    
    int                                                        _groupId;
    int                                                        _committeeId;
    
    std::vector<Peer<PBFT_Message>* >                          _groupMembers;
    std::vector<Peer<PBFT_Message>* >                          _committeeMembers;
    
    int                     faultyPeers             ()const                                         {return ceil((_committeeMembers.size() + 1) * _faultUpperBound);};
    void                    braodcast               (const PBFT_Message&);
    
public:
    PBFTPeer_Sharded                                (std::string);
    PBFTPeer_Sharded                                (const PBFTPeer_Sharded&);
    ~PBFTPeer_Sharded                               ()                                              {};
    
    //setters
    void                    setGroup                (int id)                                        {_groupId = id;};
    void                    setCommittee            (int id)                                        {_committeeId = id;};
    void                    addGroupMember          (PBFTPeer_Sharded &newMember)                   {_groupMembers.push_back(&newMember);};
    void                    addcommitteeMember      (PBFTPeer_Sharded &newMember)                   {_committeeMembers.push_back(&newMember);};
   
    // mutators
    void                    clearCommittee          ()                                              {_committeeMembers.clear(); _committeeId = -1;}
    void                    clearGroup              ()                                              {_groupMembers.clear(); _groupId = -1;}
    void                    initPrimary             ()                                              {_primary = findPrimary(_committeeMembers);};
    
    void                    preformComputation      ();
    
    std::ostream&           printTo                 (std::ostream&)const;
    void                    log                     ()const                                         {printTo(*_log);};
    
    PBFTPeer_Sharded&       operator=               (const PBFTPeer_Sharded&);
    friend std::ostream&    operator<<              (std::ostream &o, const PBFTPeer_Sharded &p)    {p.printTo(o); return o;};
};

#endif /* PBFTPeer_Sharded_hpp */
