//
//  NetworkInterface.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

// peer is an abstract template class derived from the NetworkInterface class. 
// It has the minimum functions that must be defined for the simulator to use a 
// user defined peer. The methods listed in this abstract class are the only 
// methods used in the network and simulation classes. Two methods 
// (deconstructor and performComputation) must be defined by the user, that is 
// they are pure virtual functions. All others can have empty body's. It is 
// however unlikely the user will want to leave them empty. It is templated with 
// a user defined message struct or class.


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
        // perform one step of the Algorithm with the messages in inStream
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
