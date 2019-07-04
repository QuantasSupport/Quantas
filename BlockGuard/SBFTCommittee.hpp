//
//  SBFTCommittee.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 7/1/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef SBFTCommittee_hpp
#define SBFTCommittee_hpp

#include <stdio.h>
#include "syncBFT_Peer.hpp"
#include "committee.hpp"

class SBFTCommittee{
protected:
    bool                        _consensusReached;
    std::vector<syncBFT_Peer*>  _peers;
    
public:
    SBFTCommittee                                               (std::vector<syncBFT_Peer*>);
    SBFTCommittee                                               (const SBFTCommittee&);
    SBFTCommittee&                           operator=          (const SBFTCommittee &rhs);
    
    // getters
    bool                                    checkForConsensus   ()const;
    int                                     size                ()const                         {return _peers.size();};
    syncBFT_Peer*                           get                 (int i)                         {return _peers[i];};
    const syncBFT_Peer*                     get                 (int i)const                    {return _peers[i];};
    syncBFT_Peer*                           operator[]          (int i)                         {return _peers[i];};
    const syncBFT_Peer*                     operator[]          (int i)const                    {return _peers[i];};
    
    // mutators
    void                                    preformComputation  ();
    void                                    receive             ();
    void                                    transmit            ();
};

#endif /* SBFTCommittee_hpp */
