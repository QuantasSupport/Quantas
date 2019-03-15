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
#include <map>
#include <string>
#include "Packet.hpp"

//
// Base Peer class
//
template <class algorithm>
class Peer{
private:
    // disallowing empty const so peer must have ID
    Peer(){};
protected:
    std::string                           _id;
    std::vector<Packet<algorithm>>       _channel;
    std::vector<Packet<algorithm>>       _inStream;  // messages that have arrived at this peer
    std::vector<Packet<algorithm>>       _outStream; // messages waiting to be sent by this peer
    std::map<std::string, Peer>          _groupMembers; // Peers this peer has a link to
    
public:
    Peer(std::string);
    Peer(const Peer &);
    ~Peer();
    // Setters
    void                              setID                 (std::string id)      {_id = id;};
    void                              addGroupMembers       (const Peer &n)       {_groupMembers.insert(std::pair<char,int>(n.id(),n));};

    // getters
    std::vector<Peer>                 GroupMembers          ()                    {return _groupMembers;};
    std::string                       id                    ()                    {return _id;};
    bool                              isGroupMember         (std::string);
    
    // mutators
    void                              removeGroupMember     (const Peer &p)       {_groupMembers.erase(p.id());};
    void                              removeGroupMember     (std::string id)      {_groupMembers.erase(id);};
    void                              receive               ();
    
    // send a message to this peer
    void                              sendTo                (Packet<algorithm>);
    // sends all messages in _outStream to there respective targets
    void                              transmit              ();
    // preform one step of the Consensus algorithm with the messages in inStream
    void                              preformComputation    (){};

    Peer&                             operator=             (const Peer&);
    bool                              operator==            (const Peer &rhs)     {return (_id == rhs._id);};
    bool                              operator!=            (const Peer &rhs)     {return !(*this == rhs);};
};

//
// Example Peer used for network testing
//
class ExamplePeer : public Peer<ExampleMessage>{
protected:
    int counter;
public:
    ExamplePeer(std::string);
    ExamplePeer(const ExamplePeer &rhs);
    ~ExamplePeer();
    
    void                 preformComputation();
};

#endif /* Peer_hpp */
