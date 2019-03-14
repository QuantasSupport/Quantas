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
#include <string>
#include "Message.hpp"

//
// Base Peer class
//
template <class MessageType>
class Peer{
private:
    // disallowing empty const so peer must have ID
    Peer(){};
protected:
    std::string _id;
    std::vector<MessageType>        _channel;
    std::vector<MessageType>        _inStream;  // messages that have arrived at this peer
    std::vector<MessageType>        _outStream; // messages waiting to be sent by this peer
    std::vector<Peer>           _groupMembers; // Peers this peer has a link to
    static  int                 _DELAY_UPPER_BOUND; // message delay upper bound
    
public:
    Peer(std::string);
    Peer(const Peer &);
    ~Peer();
    // Setters
    void                              setID(std::string id)                {_id = id;};
    void                              setDelayUpperBound(int max_delay)    {_DELAY_UPPER_BOUND = max_delay;};
    
    // send a message to this peer
    void                              send(MessageType);
    // for each meesage of _outStream if target peer is a neighbor send otherwise broadcast
    void                              transmit();
    // for each message in the _inStream do somthing
    void                              receive();
    // preform one step of the Consensus algorithm with the messages in inStream
    virtual void                      preformComputation() = 0;
    // add a link between this peer and another
    void                              addGroupMembers(const Peer<MessageType>);
    
    // remove a group member
    void                              removeGroupMember(const Peer<MessageType>);
    void                              removeGroupMember(std::string);
    bool                              isGroupMember(std::string);
    
    // getters
    std::vector<Peer<MessageType>>    GroupMembers()                       {return _groupMembers;};
    std::string          id()                                 {return _id;};
    
    Peer<MessageType>&                operator=(const Peer<MessageType>&);
    bool                              operator==(const Peer<MessageType>&);
    bool                              operator!=(const Peer<MessageType>&);
};

//
// Basic Peer used for network testing
//
class BasicPeer : Peer<BasicMessage>{
protected:
    int counter;
public:
    BasicPeer(std::string);
    BasicPeer(const BasicPeer &rhs);
    ~BasicPeer();
    
    void                 receive();
    void                 preformComputation();
};

#endif /* Peer_hpp */
