//
//  Peer.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "Peer.hpp"

//
// Base Peer definitions
//

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
void Peer<algorithm>::sendTo(Packet<algorithm> outMessage){
    outMessage.setTarget(_id);
    _channel.push(outMessage);
}

template <class algorithm>
void Peer<algorithm>::transmit(){
    while(!_outStream.empty()){
        Packet<algorithm> outMessage = _outStream.front();
        _outStream.erase(_outStream.begin());
        _groupMembers[outMessage.targetId()].send(outMessage);
    }
}

template <class algorithm>
void Peer<algorithm>::receive(){
    for(int i=0; i < _channel.size(); i++){
        if(_channel[i].hasArrived()){
            _inStream.push(_channel[i]);
            _channel.erase(i);
        }else{
            _channel[i].moveForward();
        }
    }
}

template <class algorithm>
bool Peer<algorithm>::isGroupMember(std::string id){
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

//
// Example Peer definitions
//
#include <iostream>
ExamplePeer::ExamplePeer(std::string id) : Peer(id){
    counter =0;
}

void ExamplePeer::preformComputation(){
    counter++;
//    for(int i = 0; i < _groupMembers.size(); i++){
//        ExsampleMessage message;
//        message.message = "Hello From ";
//        message.aPeerId = _id;
//        Packet<ExsampleMessage> newMessage(counter, _groupMembers[i] ,_id);
//        _outStream.push_back(newMessage);
//    }
    
    for(int i = 0; i < _inStream.size(); i++){
        std::cout << _id<< " has receved a message from "<< _inStream[i].sourceId()<< std::endl;
        std::cout << "  MESSAGE:"<< _inStream[i].getMessage().message<<  _inStream[i].getMessage().aPeerId<< std::endl;
    }
}
