//
//  NetworkInterface.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef Peer_hpp
#define Peer_hpp

#include <stdio.h>
#include <vector>
#include <map>
#include <deque>
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "Packet.hpp"
#include "NetworkInterface.hpp"

namespace blockguard{

    using std::string;
    using std::deque;
    using std::map;
    using std::ostream;
    using std::vector;
    using std::cout;
    using std::ostream;
    using std::left;
    using std::endl;
    using std::setw;
    using std::boolalpha;
    
    //
    // Base Peer class
    //
    template <class message>
    class Peer : public NetworkInterface<message>{
    public:
        Peer                                                     ();
        Peer                                                     (long);
        Peer                                                     (const Peer &);
        virtual ~Peer                                            ()=0;
        // tells this peer to create a transaction
        virtual void                       makeRequest           ()=0;
        // perform one step of the Consensus message with the messages in inStream
        virtual void                       performComputation    ()=0;
    };

    template <class message>
    Peer<message>::Peer(): NetworkInterface<message>(){
    }

    template <class message>
    Peer<message>::Peer(long id): NetworkInterface<message>(id){
    }

    template <class message>
    Peer<message>::Peer(const Peer &rhs){
    }

    template <class message>
    Peer<message>::~Peer(){
    }
}

#endif 
