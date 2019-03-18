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
#include <iostream>
#include "Packet.hpp"

//
// Base Peer class
//
template <class algorithm>
class Peer{
protected:
    std::string                          _id;
    std::vector<Packet<algorithm>>       _channel;
    std::vector<Packet<algorithm>>       _inStream;  // messages that have arrived at this peer
    std::vector<Packet<algorithm>>       _outStream; // messages waiting to be sent by this peer
    std::map<std::string, Peer*>         _groupMembers; // Peers this peer has a link to
    
public:
    Peer                                                    ();
    Peer                                                    (std::string);
    Peer                                                    (const Peer &);
    ~Peer                                                   ();
    // Setters
    void                              setID                 (std::string id)      {_id = id;};
    void                              addGroupMembers       (Peer &n)             {_groupMembers.insert(std::pair<std::string,Peer<algorithm>*>(n.id(),&n));};

    // getters
    std::vector<Peer>                 GroupMembers          ()const               {return _groupMembers;};
    std::string                       id                    ()const               {return _id;};
    bool                              isGroupMember         (std::string)const;
    
    // mutators
    void                              removeGroupMember     (const Peer &p)       {_groupMembers.erase(p.id());};
    void                              removeGroupMember     (std::string id)      {_groupMembers.erase(id);};
    void                              receive               ();
    
    // send a message to this peer
    void                              send                  (Packet<algorithm>);
    // sends all messages in _outStream to thsere respective targets
    void                              transmit              ();
    // preform one step of the Consensus algorithm with the messages in inStream
    void                              preformComputation    (){};

    Peer&                             operator=             (const Peer&);
    bool                              operator==            (const Peer &rhs)const {return (_id == rhs._id);};
    bool                              operator!=            (const Peer &rhs)const {return !(*this == rhs);};
};

//
// Base Peer definitions
//

template <class algorithm>
Peer<algorithm>::Peer(){
    _id = "NO ID";
    _channel = {};
    _inStream = {};
    _outStream = {};
    _groupMembers = {};
}

template <class algorithm>
Peer<algorithm>::Peer(std::string id){
    _id = id;
    _channel = {};
    _inStream = {};
    _outStream = {};
    _groupMembers = {};
}

template <class algorithm>
Peer<algorithm>::Peer(const Peer &rhs){
    _id = rhs._id;
    _channel = rhs._channel;
    _inStream = rhs._inStream;
    _outStream = rhs._outStream;
    _groupMembers = rhs._groupMembers;
}

template <class algorithm>
Peer<algorithm>::~Peer(){
}

template <class algorithm>
void Peer<algorithm>::send(Packet<algorithm> outMessage){
    _channel.push_back(outMessage);
}

template <class algorithm>
void Peer<algorithm>::transmit(){
    while(!_outStream.empty()){
        Packet<algorithm> outMessage = _outStream[0];
        _outStream.erase(_outStream.begin());
        _groupMembers[outMessage.targetId()]->send(outMessage);
    }
}

template <class algorithm>
void Peer<algorithm>::receive(){
    if(_channel.size() == 0){
        return;
    }
    if(_channel[0].hasArrived()){
        _inStream.push_back(_channel[0]);
        _channel.erase(_channel.begin());
    }else{
        _channel[0].moveForward();
    }
}

template <class algorithm>
bool Peer<algorithm>::isGroupMember(std::string id)const{
    if(_groupMembers.find(id) != _groupMembers.end()){
        return true;
    }
    return false;
}

template <class algorithm>
Peer<algorithm>& Peer<algorithm>::operator=(const Peer<algorithm> &rhs){
    _id = rhs._id;
    _channel = rhs._channel;
    _inStream = rhs._inStream;
    _outStream = rhs._outStream;
    _groupMembers = rhs._groupMembers;
    return *this;
}

#endif /* Peer_hpp */
