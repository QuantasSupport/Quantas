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
    std::vector<Message> inStream;  // messages that have arrived at this peer
    std::vector<Message> outStream; // messages waiting to be sent by this peer
    std::vector<Peer>    neighbors; // Peers this peer has a link to
public:
                         // send a message to this peer
    void                 send(Message);
                         // preform one step of the Consensus algorithm with the messages in inStream
    void                 preformComputation();
                         // add a link between this peer and another
    void                 addNeighbor(Peer);
                         // remove a link between this peer and another
    void                 removeNeighbor(Peer);
                         // send messages in outStream to each neighbor
    void                 broadcast();
                         // get a list of peers this peer has a link to
    std::vector<Peer>    Neighborhood();
};

#endif /* Peer_hpp */
