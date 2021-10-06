//
//  Interface.hpp
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

    
    static const int  LOG_WIDTH  = 27;  // var used for column width in loggin

    //
    // Base Peer class
    //
    template <class message>
    class Peer{
    protected:
        bool                               _busy;
        bool                               _byzantine;

    public:
        virtual void                       setBusy               (bool busy)                         {_busy = busy;}
        virtual bool					   isByzantine			 ()const                             {return _byzantine;};
        virtual bool					   isBusy				 ()									 {return _busy; }
        virtual void					   setByzantineFlag		 (bool f)                            {_byzantine = f;};
        virtual void                       makeCorrect           ()                                  {_byzantine = false;};
        virtual void                       makeByzantine         ()                                  {_byzantine = true;};
        virtual void                       clearMessages         ();
        // tells this peer to create a transaction
        virtual void                       makeRequest           ()=0;
        // preform one step of the Consensus message with the messages in inStream
        virtual void                       preformComputation    ()=0;
    }
}