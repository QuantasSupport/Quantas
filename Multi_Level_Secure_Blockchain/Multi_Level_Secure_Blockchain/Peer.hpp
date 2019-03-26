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
    std::map<peerId,int>                    _channelDelays;// list of channels and there delays
    std::vector<Peer<algorithm>*>           _neighbors; // peers this peer has a link to
    std::vector<Packet<algorithm>>          _inStream;// messages that have arrived at this peer
    std::vector<Packet<algorithm>>          _outStream;// messages waiting to be sent by this peer
    
    
public:
    Peer                                                    ();
    Peer                                                    (std::string);
    Peer                                                    (const Peer &);
    virtual ~Peer                                           ()=0;
    // Setters
    void                              setID                 (std::string id){_id = id;};

    // getters
    std::vector<Peer>                 neighbors             ()const;
    std::string                       id                    ()const               {return _id;};
    bool                              isNeighbor            (std::string id)const;
    int                               getDelayToNeighbor    (std::string id)const;
    
    // mutators
    void                              removeNeighbor        (const Peer &neighbor){_neighbors.erase(neighbor);};
    void                              addNeighbor           (Peer &newNeighbor, int delay);
    
    // tells this peer to create a transation
    virtual void                      makeRequest           ()=0;
    // moves msgs from the channel to the inStream if msg delay is 0 else decrease msg delay by 1
    void                              receive               ();
    // send a message to this peer
    void                              send                  (Packet<algorithm>);
    // sends all messages in _outStream to thsere respective targets
    void                              transmit              ();
    // preform one step of the Consensus algorithm with the messages in inStream
    virtual void                      preformComputation    ()=0;
    
    
    std::ostream&                     print                 (std::ostream&)const;
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
    _inStream = {};
    _outStream = {};
    _neighbors = {};
    _channelDelays = {};
    _channels = {};
}

template <class algorithm>
Peer<algorithm>::Peer(std::string id){
    _id = id;
    _inStream = {};
    _outStream = {};
    _neighbors = {};
    _channelDelays = {};
    _channels = {};
}

template <class algorithm>
Peer<algorithm>::Peer(const Peer &rhs){
    _id = rhs._id;
    _inStream = rhs._inStream;
    _outStream = rhs._outStream;
    _neighbors = rhs._neighbors;
    _channels = rhs._channels;
    _channelDelays = rhs._channelDelays;
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
    _neighbors.push_back(&newNeighbor);
    
    _channelDelays.insert(std::pair<std::string,int>(newNeighbor.id(),edgeDelay));
    
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
        
        for(int i = 0; i < _neighbors.size(); i++){
            std::string neighborID = _neighbors[i]->id();
            int maxDelay = _channelDelays[neighborID];
            if(neighborID == outMessage.targetId()){
                outMessage.setDelay(maxDelay);
                _neighbors[i]->send(outMessage);
            }
        }
    }
}

template <class algorithm>
void Peer<algorithm>::receive(){
    for(int i = 0; i < _neighbors.size(); i++){
        std::string neighborID = _neighbors[i]->id();
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
    for(int i = 0; i < _neighbors.size(); i++){
        if(id == _neighbors[i]->id()){
            return true;
        }
    }
    return false;
}

template <class algorithm>
int Peer<algorithm>::getDelayToNeighbor(std::string id)const{
    for(int i = 0; i < _neighbors.size(); i++){
        std::string neighborId = _neighbors[i].id();
        int delay = _channelDelays[neighborId];
        if(neighborId == id){
            return delay;
        }
    }
}

template <class algorithm>
Peer<algorithm>& Peer<algorithm>::operator=(const Peer<algorithm> &rhs){
    _id = rhs._id;
    _inStream = rhs._inStream;
    _outStream = rhs._outStream;
    _neighbors = rhs._neighbors;
    _channels = rhs._channels;
    _channelDelays = rhs._channelDelays;
    
    return *this;
}

template <class algorithm>
std::ostream& Peer<algorithm>::print(std::ostream &out)const{
    out<< "--- Peer ID:"<< _id<< " ---"<< std::endl;
    out<< std::left;
    out<< "\t"<< std::setw(20)<< "In Stream Size"<< std::setw(20)<< "Out Stream Size"<< std::endl;
    out<< "\t"<< std::setw(20)<< _inStream.size()<< std::setw(20)<< _outStream.size()<<std::endl<<std::endl;
    out<< "\t"<< std::setw(20)<< "Neighbor ID"<< std::setw(20)<< "Delay"<< std::setw(20)<< "Messages In Channel"<< std::endl;
    for(int i = 0; i <  _neighbors.size(); i++){
        std::string neighborId = _neighbors[i]->id();
        //int delay = _channelDelays[neighborId];
        //out<< "\t"<< neighborId<< " "<< delay<< _channels[neighborId].size()<< std::endl;
    }
    out << std::endl;
    return out;
}

template <class algorithm>
std::ostream& operator<<(std::ostream &out, const Peer<algorithm> &peer){
    peer.print(out);
    return out;
}

#endif /* Peer_hpp */
