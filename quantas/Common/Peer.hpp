/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

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
#include <algorithm>
#include "NetworkInterface.hpp"
#include "LogWriter.hpp"

namespace quantas{

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
        Peer                                                       ();
        Peer                                                       (long);
        Peer                                                       (const Peer &);
        virtual ~Peer                                              () = 0;
        // initialize any user defined parameters
        virtual void                       initParameters          (const vector<Peer<message>*>& _peers, json parameters) {};
        // perform one step of the Algorithm with the messages in inStream
        virtual void                       performComputation      () = 0;
        // ran once per round, used to submit transactions or collect metrics
        virtual void                       endOfRound              (const vector<Peer<message>*>& _peers) {};
        static int                         getRound                ()                                     { return _round; };
        static int                         getLastRound            ()                                     { return _lastRound;};
        static void                        initializeRound         ()                                     { _round = 0; };
        static void                        incrementRound          ()                                     { _round++; };
        static void                        initializeLastRound     (int lastRound)                        { _lastRound = lastRound; };
        static bool                        lastRound               ()                                     { return _lastRound == _round; };
        static void                        initializeSourcePoolSize(int sourcePoolSize)                   { _sourcePoolSize = sourcePoolSize; };
        static int                         getSourcePoolSize       ()                                     { return _sourcePoolSize; };
    private:
        // current round
        static int                         _round;
        // last round
        static int                         _lastRound;
        // size of source pool (FOR BLOCKCHAIN IN DYNAMIC NETWORKS)
        static int                         _sourcePoolSize;
    };

    template <class message>
    int Peer<message>::_round = 0;
    
    template <class message>
    int Peer<message>::_lastRound = 0;
    
    template <class message>
    int Peer<message>::_sourcePoolSize = 0;

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
