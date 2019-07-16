//
//  Peer.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef Peer_hpp
#define Peer_hpp

#include <stdio.h>
#include <vector>
#include <map>
#include <deque>
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "Packet.hpp"

// var used for column width in loggin
static const int LOG_WIDTH = 27;

//
// Base Peer class
//
template <class message>
class Peer{
protected:
    std::string                             _id;
    bool                                    _byzantine;
	bool									_busy;
    int                                     _clock;
    
    // type abbreviations
    typedef std::deque<Packet<message> >    aChannel;
    typedef std::string                     peerId;
    std::map<peerId,aChannel>               _channels;// list of chanels between this peer and others
    std::map<peerId,int>                    _channelDelays;// list of channels and there delays
    std::map<std::string, Peer<message>* >  _neighbors; // peers this peer has a link to
    std::deque<Packet<message> >            _inStream;// messages that have arrived at this peer
    std::deque<Packet<message> >            _outStream;// messages waiting to be sent by this peer
    
    // metrics
    int                                     _numberOfMessagesSent;
    
    // logging
    std::ostream                            *_log;
    bool                                    _printNeighborhood;
    
public:
    Peer                                                    ();
    Peer                                                    (std::string);
    Peer                                                    (const Peer &);
    virtual ~Peer                                           ()=0;
    // Setters
    void                              setID                 (std::string id)                    {_id = id;};
    void                              setLogFile            (std::ostream &o)                   {_log = &o;};
    void                              printNeighborhoodOn   ()                                  {_printNeighborhood = true;}
    void                              printNeighborhoodOff  ()                                  {_printNeighborhood = false;}
    virtual void                      setBusy               (bool busy)                         {_busy = busy;}
    // getters
    std::vector<std::string>          neighbors             ()const;
    std::string                       id                    ()const                             {return _id;};
    bool                              isNeighbor            (std::string id)const;
    int                               getDelayToNeighbor    (std::string id)const;
    int                               getMessageCount       ()const                             {return _numberOfMessagesSent;};
    int                               getClock              ()const                             {return _clock;};
    virtual bool					  isByzantine			()const                             {return _byzantine;};
	virtual bool					  isBusy				()									{return _busy; }
    std::deque<Packet<message> >      getInStream           ()const                             {return _inStream;};
    std::deque<Packet<message> >      getOutStream          ()const                             {return _outStream;};
    
    
    // mutators
    void                              removeNeighbor        (const Peer &neighbor)              {_neighbors.erase(neighbor);};
    void                              addNeighbor           (Peer &newNeighbor, int delay);
    virtual void					  setByzantineFlag		(bool f)                            {_byzantine = f;};
    virtual void                      makeCorrect           ()                                  {_byzantine = false;};
    virtual void                      makeByzantine         ()                                  {_byzantine = true;};

    // tells this peer to create a transation
    virtual void                      makeRequest           ()=0;
    // moves msgs from the channel to the inStream if msg delay is 0 else decrease msg delay by 1
    void                              receive               ();
    // send a message to this peer
    void                              send                  (Packet<message>);
    // sends all messages in _outStream to thsere respective targets
    void                              transmit              ();
    // preform one step of the Consensus message with the messages in inStream
    virtual void                      preformComputation    ()=0;

    void                              log                   ()const;
    std::ostream&                     printTo               (std::ostream&)const;
    Peer&                             operator=             (const Peer&);

    // == and != compare all attributes 
    bool                              operator==            (const Peer &rhs)const              {return (_id == rhs._id);};
    bool                              operator!=            (const Peer &rhs)const              {return !(*this == rhs);};
    // greater and less then are based on peer id (standard string comparison)
    bool                              operator<=            (const Peer &rhs)const              {return (_id <= rhs._id);};
    bool                              operator<             (const Peer &rhs)const              {return (_id < rhs._id);};
    bool                              operator>=            (const Peer &rhs)const              {return (_id >= rhs._id);};
    bool                              operator>             (const Peer &rhs)const              {return (_id > rhs._id);};
    bool                              operator<=            (const std::string &rhs)const       {return (_id <= rhs);};
    bool                              operator<             (const std::string &rhs)const       {return (_id < rhs);};
    bool                              operator>=            (const std::string &rhs)const       {return (_id >= rhs);};
    bool                              operator>             (const std::string &rhs)const       {return (_id > rhs);};

    friend std::ostream&              operator<<            (std::ostream&, const Peer&);
};

//
// Base Peer definitions
//

template <class message>
Peer<message>::Peer(){
    _id = "NO ID";
    _inStream = std::deque<Packet<message> >();
    _outStream = std::deque<Packet<message> >();
    _neighbors = std::map<std::string, Peer<message>* >();
    _channelDelays = std::map<peerId,int>();
    _channels = std::map<peerId,aChannel>();
    _log = &std::cout;
    _byzantine = false;
    _numberOfMessagesSent = 0;
    _printNeighborhood = false;
	_busy = false;
    _clock = 0;
}

template <class message>
Peer<message>::Peer(std::string id){
    _id = id;
    _inStream = std::deque<Packet<message> >();
    _outStream = std::deque<Packet<message> >();
    _neighbors = std::map<std::string, Peer<message>* >();
    _channelDelays = std::map<peerId,int>();
    _channels = std::map<peerId,aChannel>();
    _log = &std::cout;
    _byzantine = false;
    _numberOfMessagesSent = 0;
    _printNeighborhood = false;
	_busy = false;
    _clock = 0;
}

template <class message>
Peer<message>::Peer(const Peer &rhs){
    _id = rhs._id;
    _inStream = rhs._inStream;
    _outStream = rhs._outStream;
    _neighbors = rhs._neighbors;
    _channels = rhs._channels;
    _channelDelays = rhs._channelDelays;
    _log = rhs._log;
    _byzantine = rhs._byzantine;
    _numberOfMessagesSent = rhs._numberOfMessagesSent;
    _printNeighborhood = rhs._printNeighborhood;
	_busy = rhs._busy;
    _clock = rhs._clock;
}

template <class message>
Peer<message>::~Peer(){
}

template <class message>
void Peer<message>::addNeighbor(Peer<message> &newNeighbor, int delay){
    // grourd to make sure delay is at lest 1, less then 1 will couse errors when calculating delay (divisioin by 0)
    int edgeDelay = delay;
    if(edgeDelay < 1){
        edgeDelay = 1;
    }
    _neighbors[newNeighbor.id()] = &newNeighbor;
    _channelDelays[newNeighbor.id()] = edgeDelay;
    _channels[newNeighbor.id()] = std::deque<Packet<message> >();
}

// called on recever
template <class message>
void Peer<message>::send(Packet<message> outMessage){
    _channels.at(outMessage.sourceId()).push_back(outMessage);
}

// called on sender
template <class message>
void Peer<message>::transmit(){
    // send all messages to there destantion peer channels  
    while(!_outStream.empty()){
        
        Packet<message> outMessage = _outStream.front();
        _outStream.pop_front();
        
        // if sent to self loop back next round
        if(_id == outMessage.targetId()){
            outMessage.setDelay(1);
            _inStream.push_back(outMessage);
        }else{
            std::string targetId = outMessage.targetId();
            int maxDelay = _channelDelays.at(targetId);
            outMessage.setDelay(maxDelay);
            _neighbors[targetId]->send(outMessage);
        }
        ++_numberOfMessagesSent;
    }
}

template <class message>
void Peer<message>::receive(){
    _clock++;
    for (auto it=_neighbors.begin(); it!=_neighbors.end(); ++it){
        std::string neighborID = it->first;
        if(!_channels.at(neighborID).empty()){
            if(_channels.at(neighborID).front().hasArrived()){
                _inStream.push_back(_channels.at(neighborID).front());
                _channels.at(neighborID).pop_front();
            }else{
                _channels.at(neighborID).front().moveForward();
            }
        }
    }
}


template <class message>
bool Peer<message>::isNeighbor(std::string id)const{
    if(_neighbors.find(id) != _neighbors.end()){
        return true;
    }
    return false;
}

template <class message>
std::vector<std::string> Peer<message>::neighbors()const{
    std::vector<std::string> neighborIds = std::vector<std::string>();
    for (auto it=_neighbors.begin(); it!=_neighbors.end(); ++it){
        neighborIds.push_back(it->first);
    }
    return neighborIds;
}

template <class message>
int Peer<message>::getDelayToNeighbor(std::string id)const{
    return _channelDelays.at(id);
}

template <class message>
Peer<message>& Peer<message>::operator=(const Peer<message> &rhs){
	if(this == &rhs)
		return *this;
    _id = rhs._id;
    _inStream = rhs._inStream;
    _outStream = rhs._outStream;
    _neighbors = rhs._neighbors;
    _channels = rhs._channels;
    _channelDelays = rhs._channelDelays;
    _log = rhs._log;
    _byzantine = rhs._byzantine;
    _numberOfMessagesSent = rhs._numberOfMessagesSent;
    _printNeighborhood = rhs._printNeighborhood;
	_busy = rhs._busy;
    _clock = rhs._clock;

    return *this;
}

template <class message>
void Peer<message>::log()const{
    printTo(*_log);
}

template <class message>
std::ostream& Peer<message>::printTo(std::ostream &out)const{
    out<< "-- Peer ID:"<< _id<< " --"<< std::endl;
    out<< std::left;
    out<< "\t"<< std::setw(LOG_WIDTH)<< "In Stream Size"<< std::setw(LOG_WIDTH)<< "Out Stream Size"<< std::setw(LOG_WIDTH)<< "Message Count"<< std::setw(LOG_WIDTH)<< "Is Byzantine"<<std::endl;
    out<< "\t"<< std::setw(LOG_WIDTH)<< _inStream.size()<< std::setw(LOG_WIDTH)<< _outStream.size()<<std::setw(LOG_WIDTH)<< _numberOfMessagesSent<< std::setw(LOG_WIDTH)<<std::setw(LOG_WIDTH)<< std::boolalpha<<  _byzantine<<std::endl<<std::endl;
    if(_printNeighborhood){
        out<< "\t"<< std::setw(LOG_WIDTH)<< "Neighbor ID"<< std::setw(LOG_WIDTH)<< "Delay"<< std::setw(LOG_WIDTH)<< "Messages In Channel"<< std::endl;
        for (auto it=_neighbors.begin(); it!=_neighbors.end(); ++it){
            std::string neighborId = it->first;
            out<< "\t"<< std::setw(LOG_WIDTH)<< neighborId<< std::setw(LOG_WIDTH)<< getDelayToNeighbor(neighborId)<< std::setw(LOG_WIDTH)<<  _channels.at(neighborId).size()<< std::endl;
        }
    }
    out << std::endl;
    return out;
}

template <class message>
std::ostream& operator<<(std::ostream &out, const Peer<message> &peer){
    peer.printTo(out);
    return out;
}

#endif /* Peer_hpp */
