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

struct BGSMessage{
    
};

class BlockGuardSharded :Peer<BGSMessage> ,PBFT_Peer{
protected:
    
public:
    BlockGuardSharded();
    BlockGuardSharded(const BlockGuardSharded&);
    ~BlockGuardSharded();
    
    void makeRequest();
    void preformComputation();
    
};

#endif /* BlockGuardSharded_hpp */
