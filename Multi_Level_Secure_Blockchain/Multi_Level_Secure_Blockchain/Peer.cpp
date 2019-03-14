//
//  Peer.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <vector>
#include <iostream>
#include "Peer.hpp"
#include "Message.hpp"

//
// Base Peer definitions
//
template <class MessageType>
Peer<MessageType>::Peer(std::string id){
    _id = id;
    _channel = {};
    _inStream = {};
    _outStream = {};
    _groupMembers = {};
}

template <class MessageType>
Peer<MessageType>::Peer(const Peer &rhs){
    _id = rhs._id;
    _channel = rhs._channel;
    _inStream = rhs._inStream;
    _outStream = rhs._outStream;
    _groupMembers = rhs._groupMembers;
}

template <class MessageType>
Peer<MessageType>::~Peer(){
    // no memory allocated so nothing to do
}

template <class MessageType>
void Peer<MessageType>::send(MessageType outMessage){
}

template <class MessageType>
void Peer<MessageType>::transmit(){
}

template <class MessageType>
void Peer<MessageType>::receive(){
}

template <class MessageType>
void Peer<MessageType>::addGroupMembers(const Peer newNeighbor){
    _groupMembers.push_back(newNeighbor);
}

template <class MessageType>
void Peer<MessageType>::removeGroupMember(const Peer oldNeighbor){
    for(int i=0; i < _groupMembers.size(); i++){
        if(_groupMembers[i] == oldNeighbor){
            _groupMembers.erase(_groupMembers.begin() + i);
        }
    }
}

template <class MessageType>
void Peer<MessageType>::removeGroupMember(const std::string oldNeighborId){
    for(int i=0; i < _groupMembers.size(); i++){
        if(_groupMembers[i].id() == oldNeighborId){
            _groupMembers.erase(_groupMembers.begin() + i);
        }
    }
}

template <class MessageType>
bool Peer<MessageType>::isGroupMember(std::string id){
    for(int i=0; i<_groupMembers.size(); i++){
        if(_groupMembers[i].id() == id){
            return true;
        }
    }
    return false;
}

template <class MessageType>
Peer<MessageType>& Peer<MessageType>::operator=(const Peer &rhs){
    _id = rhs._id;
    _channel = rhs._channel;
    _inStream = rhs._inStream;
    _outStream = rhs._outStream;
    _groupMembers = rhs._groupMembers;
    return *this;
}

template <class MessageType>
bool Peer<MessageType>::operator==(const Peer &rhs){
    return (_id == rhs._id);
}

template <class MessageType>
bool Peer<MessageType>::operator!=(const Peer &rhs){
    return !(*this == rhs);
}

//
// basicPeer definitions
//

BasicPeer::BasicPeer(std::string id) : Peer(id){
    counter =0;
}

void BasicPeer::receive(){
}

void BasicPeer::preformComputation(){
    counter++;
    BasicMessage newMesg(counter);
    newMesg.setText("Message " + std::to_string(counter));
    _outStream.push_back(newMesg);
}
