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

Peer::Peer(std::string id){
    _id = id;
    _inStream = {};
    _outStream = {};
    _neighbors = {};
}

Peer::Peer(const Peer &rhs){
    _id = rhs._id;
    _inStream = rhs._inStream;
    _outStream = rhs._outStream;
    _neighbors = rhs._neighbors;
}

Peer::~Peer(){
    // no memory allocated so nothing to do
}

void Peer::setID(std::string id){
    _id = id;
}

void Peer::send(Message outMessage){
    _inStream.push_back(outMessage);
}

void Peer::broadcast(Message mesgToBroadcast){
    if(!messageAlreadyReceved(mesgToBroadcast)){
        return;
    }
    for(int id=0; id < _neighbors.size(); id++){
        if(_neighbors[id].id() != mesgToBroadcast.sourceId()){
            _neighbors[id].send(mesgToBroadcast);
        }
    }
}

void Peer::transmit(){
    while(!_outStream.empty()){
    Message outMessage = _outStream.back();
    _outStream.pop_back();
        // looks for target as neighbor if found send and return;
        for(int i=0; i<_neighbors.size(); i++){
            if(_neighbors[i].id() == outMessage.targetId()){
                _neighbors[i].send(outMessage);
                return;
            }
        }
        // target not a neighbor broadcast
        broadcast(outMessage);
    }
}

void Peer::receive(){
    _inStream.clear();
    // left blank for base class needs to overwritten for derived classes
}

void Peer::preformComputation(){
    // left empty for base class
}

void Peer::addNeighbor(const Peer newNeighbor){
    _neighbors.push_back(newNeighbor);
}

void Peer::removeNeighbor(const Peer oldNeighbor){
    for(int i=0; i < _neighbors.size(); i++){
        if(_neighbors[i] == oldNeighbor){
            _neighbors.erase(_neighbors.begin() + i);
        }
    }
}

void Peer::removeNeighbor(const std::string oldNeighborId){
    for(int i=0; i < _neighbors.size(); i++){
        if(_neighbors[i].id() == oldNeighborId){
            _neighbors.erase(_neighbors.begin() + i);
        }
    }
}

Peer& Peer::operator=(const Peer &rhs){
    _id = rhs._id;
    _inStream = rhs._inStream;
    _outStream = rhs._outStream;
    _neighbors = rhs._neighbors;
    return *this;
}

bool Peer::operator==(const Peer &rhs){
    return (_id == rhs._id && _neighbors == rhs._neighbors);
}

bool Peer::operator!=(const Peer &rhs){
    return !(*this == rhs);
}

// util methods
bool Peer::messageAlreadyReceved(Message aMessage){
    for(int i=0; i<_inStream.size(); i++){
        if(_inStream[i].id() == aMessage.id()){
            return true;
        }
    }
    return false;
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
    BasicMessage newMesg;
    newMesg.setText("Message " + std::to_string(counter));
    _outStream.push_back(newMesg);
}
