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

// var used for column width in loggin
static const int LOG_WIDTH = 27;

//
// Base Peer class
//
template <class algorithm>
class Peer{
protected:
    std::string                             _id;
    bool                                    _byzantine;
    
    // communication vars
    typedef std::vector<Packet<algorithm> > aChannel;
    typedef std::string                     peerId;
    std::map<peerId,aChannel>               _channels;// list of chanels between this peer and others
    std::map<peerId,int>                    _channelDelays;// list of channels and there delays
    std::vector<Peer<algorithm>* >          _neighbors; // peers this peer has a link to
    std::vector<Packet<algorithm> >         _inStream;// messages that have arrived at this peer
    std::vector<Packet<algorithm> >         _outStream;// messages waiting to be sent by this peer
    
    // metrics
    int                                     _numberOfMessagesSent;
    
    // logging
    std::ostream                            *_log;
    
public:
    Peer                                                    ();
    Peer                                                    (std::string);
    Peer                                                    (const Peer &);
    virtual ~Peer                                           ()=0;
    // Setters
    void                              setID                 (std::string id)                    {_id = id;};
    void                              setLogFile            (std::ostream &o)                   {_log = &o;};

    // getters
    std::vector<std::string>          neighbors             ()const;
    std::string                       id                    ()const                             {return _id;};
    bool                              isNeighbor            (std::string id)const;
    int                               getDelayToNeighbor    (std::string id)const;
    int                               getMessageCount       ()const                             {return _numberOfMessagesSent;};
    
    // mutators
    void                              removeNeighbor        (const Peer &neighbor)              {_neighbors.erase(neighbor);};
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
    
    void                              log                   ()const;
    std::ostream&                     printTo               (std::ostream&)const;
    Peer&                             operator=             (const Peer&);
    bool                              operator==            (const Peer &rhs)const              {return (_id == rhs._id);};
    bool                              operator!=            (const Peer &rhs)const              {return !(*this == rhs);};
    friend std::ostream&              operator<<            (std::ostream&, const Peer&);
    virtual bool					  isByzantine			()                                  {return _byzantine;};
    virtual void					  setByzantineFlag		(bool f)                            {_byzantine = f;};
    virtual void                      makeCorrect           ()                                  {_byzantine = false;};
    virtual void                      makeByzantine         ()                                  {_byzantine = true;};
};

//
// Base Peer definitions
//

template <class algorithm>
Peer<algorithm>::Peer(){
    typedef std::vector<Packet<algorithm> > aChannel;
    typedef std::string peerId;
    _id = "NO ID";
    _inStream = std::vector<Packet<algorithm> >();
    _outStream = std::vector<Packet<algorithm> >();
    _neighbors = std::vector<Peer<algorithm>* >();
    _channelDelays = std::map<peerId,int>();
    _channels = std::map<peerId,aChannel>();
    _log = &std::cout;
    _byzantine = false;
    _numberOfMessagesSent = 0;
}

template <class algorithm>
Peer<algorithm>::Peer(std::string id){
    typedef std::vector<Packet<algorithm> > aChannel;
    typedef std::string peerId;
    _id = id;
    _inStream = std::vector<Packet<algorithm> >();
    _outStream = std::vector<Packet<algorithm> >();
    _neighbors = std::vector<Peer<algorithm>* >();
    _channelDelays = std::map<peerId,int>();
    _channels = std::map<peerId,aChannel>();
    _log = &std::cout;
    _byzantine = false;
    _numberOfMessagesSent = 0;
}

template <class algorithm>
Peer<algorithm>::Peer(const Peer &rhs){
    _id = rhs._id;
    _inStream = rhs._inStream;
    _outStream = rhs._outStream;
    _neighbors = rhs._neighbors;
    _channels = rhs._channels;
    _channelDelays = rhs._channelDelays;
    _log = rhs._log;
    _byzantine = rhs._byzantine;
    _numberOfMessagesSent = rhs._numberOfMessagesSent;
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
    
    _channelDelays[newNeighbor.id()] = edgeDelay;
    _channels[newNeighbor.id()] = std::vector<Packet<algorithm> >();
}

// called on recever
template <class algorithm>
void Peer<algorithm>::send(Packet<algorithm> outMessage){
    _numberOfMessagesSent++;
    _channels.at(outMessage.sourceId()).push_back(outMessage);
}

// called on sender
template <class algorithm>
void Peer<algorithm>::transmit(){
    while(!_outStream.empty()){
        
        Packet<algorithm> outMessage = _outStream[0];
        _outStream.erase(_outStream.begin());
        
        for(int i = 0; i < _neighbors.size(); i++){
            std::string neighborID = _neighbors[i]->id();
            int maxDelay = _channelDelays.at(neighborID);
            if(neighborID == outMessage.targetId()){
                outMessage.setDelay(maxDelay);
                _neighbors[i]->send(outMessage);
            }else if(_id == outMessage.targetId()){
                _inStream.push_back(outMessage);
            }
        }
    }
}

template <class algorithm>
void Peer<algorithm>::receive(){
    for(int i = 0; i < _neighbors.size(); i++){
        std::string neighborID = _neighbors[i]->id();
        if(!_channels.at(neighborID).empty()){
            if(_channels.at(neighborID).front().hasArrived()){
                _inStream.push_back(_channels.at(neighborID).front());
                _channels.at(neighborID).erase(_channels.at(neighborID).begin());
            }else{
                _channels.at(neighborID).front().moveForward();
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
std::vector<std::string> Peer<algorithm>::neighbors()const{
    std::vector<std::string> neighborIds = std::vector<std::string>();
    for(int i = 0; i < _neighbors.size(); i++){
        neighborIds.push_back(_neighbors[i]->id());
    }
    return neighborIds;
}

template <class algorithm>
int Peer<algorithm>::getDelayToNeighbor(std::string id)const{
    return _channelDelays.at(id);
}

template <class algorithm>
Peer<algorithm>& Peer<algorithm>::operator=(const Peer<algorithm> &rhs){
    _id = rhs._id;
    _inStream = rhs._inStream;
    _outStream = rhs._outStream;
    _neighbors = rhs._neighbors;
    _channels = rhs._channels;
    _channelDelays = rhs._channelDelays;
    _log = rhs._log;
    _byzantine = rhs._byzantine;
    _numberOfMessagesSent = rhs._numberOfMessagesSent;
    
    return *this;
}

template <class algorithm>
void Peer<algorithm>::log()const{
    printTo(*_log);
}

template <class algorithm>
std::ostream& Peer<algorithm>::printTo(std::ostream &out)const{
    out<< "-- Peer ID:"<< _id<< " --"<< std::endl;
    out<< std::left;
    out<< "\t"<< std::setw(LOG_WIDTH)<< "In Stream Size"<< std::setw(LOG_WIDTH)<< "Out Stream Size"<< std::setw(LOG_WIDTH)<< "Is Byzantine"<< std::setw(LOG_WIDTH)<< "Message Count"<<std::endl;
    out<< "\t"<< std::setw(LOG_WIDTH)<< _inStream.size()<< std::setw(LOG_WIDTH)<< _outStream.size()<<std::setw(LOG_WIDTH)<< std::boolalpha<<  _byzantine<< std::setw(LOG_WIDTH)<<std::setw(LOG_WIDTH)<< _numberOfMessagesSent<<std::endl<<std::endl;
    out<< "\t"<< std::setw(LOG_WIDTH)<< "Neighbor ID"<< std::setw(LOG_WIDTH)<< "Delay"<< std::setw(LOG_WIDTH)<< "Messages In Channel"<< std::endl;
    for(int i = 0; i <  _neighbors.size(); i++){
        std::string neighborId = _neighbors[i]->id();
        out<< "\t"<< std::setw(LOG_WIDTH)<< neighborId<< std::setw(LOG_WIDTH)<< getDelayToNeighbor(neighborId)<< std::setw(LOG_WIDTH)<<  _channels.at(neighborId).size()<< std::endl;
    }
    out << std::endl;
    return out;
}

template <class algorithm>
std::ostream& operator<<(std::ostream &out, const Peer<algorithm> &peer){
    peer.printTo(out);
    return out;
}

#endif /* Peer_hpp */
