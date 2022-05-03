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
#include <algorithm>
#include "NetworkInterface.hpp"
#include "../Logging/LogWritter.hpp"

namespace blockguard{

    using std::string;
    using std::deque;
    using std::map;
    using std::ostream;
    using std::vector;
    using std::cout;
    using std::endl;
    
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
        // perform one step of the Algorithm with the messages in inStream
        virtual void                       performComputation    ()=0;
        virtual void                       endOfRound            (const vector<Peer<message>*>& _peers) = 0;
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
