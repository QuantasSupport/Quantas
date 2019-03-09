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
class Peer{
private:
    // disallowing empty const so peer must have ID
    Peer(){};
protected:
    std::string _id;
    std::vector<Message> _inStream;  // messages that have arrived at this peer
    std::vector<Message> _outStream; // messages waiting to be sent by this peer
    std::vector<Peer>    _neighbors; // Peers this peer has a link to
    
    bool messageAlreadyReceved(Message);
    
public:
    Peer(std::string);
    Peer(const Peer &);
    ~Peer();
                         // Sets Peer id
    void                 setID(std::string);
                         // send a message to this peer
    void                 send(Message);
                         // send a messages each neighbor
    void                 broadcast(Message);
                         // for each meesage of _outStream if target peer is a neighbor send otherwise broadcast
    void                 transmit();
                         // for each message in the _inStream do somthing
    void                 receive();
                         // preform one step of the Consensus algorithm with the messages in inStream
    void                 preformComputation();
                         // add a link between this peer and another
    void                 addNeighbor(const Peer);
                         // remove a link between this peer and another
    void                 removeNeighbor(const Peer);
    void                 removeNeighbor(std::string);
    
                         // getters
    std::vector<Peer>    Neighborhood()   {return _neighbors;};
    std::string          id()             {return _id;};
    
    Peer&                operator=(const Peer &);
    bool                 operator==(const Peer &);
    bool                 operator!=(const Peer &);
};

//
// Basic Peer used for network testing
//
class BasicPeer : Peer{
protected:
    std::vector<BasicMessage> _inStream;  // messages that have arrived at this peer
    std::vector<BasicMessage> _outStream; // messages waiting to be sent by this peer
    int counter;
public:
    BasicPeer(std::string);
    BasicPeer(const Peer &rhs);
    ~BasicPeer();
    
    void                 receive();
    void                 preformComputation();
};

#endif /* Peer_hpp */
