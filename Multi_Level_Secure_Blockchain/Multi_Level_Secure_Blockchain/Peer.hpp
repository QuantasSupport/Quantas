//
//  Peer.hpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef Peer_hpp
#define Peer_hpp

#include <stdio.h>
#include <vector>
#include "Message.hpp"

class Peer{
protected:
    std::vector<Message> inStream;
    std::vector<Peer> neighbors;
public:
    void send(Message);
    void preformComputation();
};

#endif /* Peer_hpp */
