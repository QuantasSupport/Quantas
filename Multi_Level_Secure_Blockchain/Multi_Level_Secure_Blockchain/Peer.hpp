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
#include <iomanip>
#include "Packet.hpp"

//
// Base Peer class
//
template <class algorithm>
class Peer{
protected:
    std::string                             _id;
    
    // communication vars
    typedef std::vector<Packet<algorithm>>  aChannel;
    typedef std::string                     peerId;
    std::map<peerId,aChannel>               _channels;// list of chanels between this peer and others
    std::map<Peer<algorithm>*,int>          _neighbors;// Peers this peer has a link to and thier delays
    std::vector<Packet<algorithm>>          _inStream;// messages that have arrived at this peer
    std::vector<Packet<algorithm>>          _outStream;// messages waiting to be sent by this peer
    
    
public:
    Peer                                                    ();
    Peer                                                    (std::string);
    Peer                                                    (const Peer &);
    ~Peer                                                   ();
    // Setters
    void                              setID                 (std::string id){_id = id;};
    void                              addNeighbor           (Peer &, int);

    // getters
    std::vector<Peer>                 neighbors             ()const;
    std::string                       id                    ()const               {return _id;};
    bool                              isNeighbor            (std::string)const;
    int                               delayToNeighbor       (std::string)const;
    
    // mutators
    void                              removeNeighbor        (const Peer &p){_neighbors.erase(p);};
    void                              receive               ();
    
    // send a message to this peer
    void                              send                  (Packet<algorithm>);
    // sends all messages in _outStream to thsere respective targets
    void                              transmit              ();
    // preform one step of the Consensus algorithm with the messages in inStream
    virtual void                      preformComputation    (){};

    virtual void                      makeRequest           (){};
    
    Peer&                             operator=             (const Peer&);
    bool                              operator==            (const Peer &rhs)const {return (_id == rhs._id);};
    bool                              operator!=            (const Peer &rhs)const {return !(*this == rhs);};
    friend std::ostream&              operator<<            (std::ostream&, const Peer&);
};

//
// Base Peer definitions
//

template <class algorithm>
Peer<algorithm>::Peer(){
    _id = "NO ID";
    _channels = {};
    _inStream = {};
    _outStream = {};
    _neighbors = {};
}

template <class algorithm>
Peer<algorithm>::Peer(std::string id){
    _id = id;
    _channels = {};
    _inStream = {};
    _outStream = {};
    _neighbors = {};
}

template <class algorithm>
Peer<algorithm>::Peer(const Peer &rhs){
    _id = rhs._id;
    _channels = rhs._channels;
    _inStream = rhs._inStream;
    _outStream = rhs._outStream;
    _neighbors = rhs._neighbors;
}

template <class algorithm>
Peer<algorithm>::~Peer(){
}

template <class algorithm>
void Peer<algorithm>::addNeighbor(Peer<algorithm> &newNeighbor, int delay){
    // grourd to make sure delay is at lest 1, less then 1 will couse errors when calculating delay (divisioin by 0)
    int edgeDelay = delay;
    if(edgeDelay < 1){
        edgeDelay = 1;
    }
    _neighbors.insert(std::pair<Peer*,int>(&newNeighbor,edgeDelay));
    
    typedef std::vector<Packet<algorithm>> aChannel;
    std::pair<std::string, aChannel> newChannel(newNeighbor.id(),{});
    _channels.insert(newChannel);
}

// called on recever
template <class algorithm>
void Peer<algorithm>::send(Packet<algorithm> outMessage){
    _channels[outMessage.sourceId()].push_back(outMessage);
}

// called on sender
template <class algorithm>
void Peer<algorithm>::transmit(){
    while(!_outStream.empty()){
        
        Packet<algorithm> outMessage = _outStream[0];
        _outStream.erase(_outStream.begin());
        
        for(auto it = _neighbors.begin(); it != _neighbors.end(); it++){
            Peer<algorithm>* peer = it->first;
            int maxDelay = it->second;
            if(peer->id() == outMessage.targetId()){
                outMessage.setDelay(maxDelay);
                peer->send(outMessage);
            }
        }
    }
}

template <class algorithm>
void Peer<algorithm>::receive(){
    for(auto i = _neighbors.begin(); i != _neighbors.end(); i++){
        std::string neighborID = i->first->id();
        if(!_channels[neighborID].empty()){
            if(_channels[neighborID].front().hasArrived()){
                _inStream.push_back(_channels[neighborID].front());
                _channels[neighborID].erase(_channels[neighborID].begin());
            }else{
                _channels[neighborID].front().moveForward();
            }
        }
    }
}


template <class algorithm>
bool Peer<algorithm>::isNeighbor(std::string id)const{
    for(auto it = _neighbors.begin(); it != _neighbors.end(); it++){
        if(id == it->first->id()){
            return true;
        }
    }
    return false;
}

template <class algorithm>
int Peer<algorithm>::delayToNeighbor(std::string id)const{
    for(auto it = _neighbors.begin(); it != _neighbors.end(); it++){
        std::string neighborId = it->first->id();
        int delay = it->second;
        if(neighborId == id){
            return delay;
        }
    }
}

template <class algorithm>
Peer<algorithm>& Peer<algorithm>::operator=(const Peer<algorithm> &rhs){
    _id = rhs._id;
    _channels = rhs._channels;
    _inStream = rhs._inStream;
    _outStream = rhs._outStream;
    _neighbors = rhs._neighbors;
    return *this;
}

template <class algorithm>
std::ostream& operator<<(std::ostream &out, const Peer<algorithm> &peer){
    out<< "Peer ID:"<< peer._id<< std::endl;
    out<< "\t"<< "Neighbors:";
    out<< std::left<< std::setw(20);
    out<< "\t"<< "In Stream Size"<< "Out Stream Size"<< std::endl;
    out<< "\t"<< peer._inStream.size()<< peer._outStream.size();
    out<< "\t"<< "ID"<< "Delay"<< "Messages In Channel"<< std::endl;
    for(auto it = peer._neighbors.begin(); it !=  peer._neighbors.end(); it++){
        Peer<algorithm>* neighbor = it->first;
        int delay = it->second;
        out<< "\t"<< neighbor->id()<< delay<< peer._channels[neighbor->id()].size()<< std::endl;
    }
    
}

#endif /* Peer_hpp */
