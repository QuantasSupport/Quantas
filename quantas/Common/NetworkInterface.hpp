/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/
//
// This class represents a connection between a peer and it's neighbors. A peer 
// is a derived class from the NetworkInterface class that adds algorithm specific details.
// The NetworkInterface class handles sending and receiving of packets (messages) between peers.
// It also tracks what interfaces (and thus peers) this NetworkInterface is able to communicate with. 
//
// This class is responsable for
// * storeing all the inbound packets to this interface 
// * updating inbound packets delay (decreaseing the delay each round until it is delivered)
// * storing a list of neighbors (a.k.a approved interfaces to send to)
// * storing the maximum delay a packet can have when being sent to a neighbors interface
// * sending packets to other interfaces (including assigning it a delay between 1 and the maximum)
// * storeing received packets at this interface
//
// === RECEIVING MESSAGES ===
// Each instance of NetworkInterface has a list of its neighbors NetworkInterface ID <_neighbors>, 
// and a hashmap <_inBoundChannels> with the ID of this NetworkInterface's neighbors as the key and a 
// queue of inbound packets (or <aChannel>) as the value. When receive is called each packet in each 
// channel has <<moveForward>> called. This decrements the delay on the packet (how many rounds the 
// packet must wait until it's arrvied at it's target interface). Then the head of each channel has 
// <<hasArrived>> called. This returns true is the packet has arrvied (delay == 0) and false otherwise.
// If <<hasArrived>> is true then the packet is moved from the channel in <_inBoundChannels> to 
// the NetworkInterface's <_inStream>. This repeats poping the head of the channel and pushing onto 
// <_inStream> until a packet has not arrived. In this way all packets are received in the same order
// they where sent. 
//
// Note: packets are received in the same order they where sent and only after all packets sent before
// it have been received
// 
//
// === TRANSMITING MESSAGES ===
// Each instance of NetworkInterface has a list of references to it's neighbor's NetworkInterface 
// and a list of the delays to thouse neighbor's interface. These are each stored in hashmaps with
// the neighbor's NetworkInterface ID as the key and either the reference to the interface or delay 
// as the value. The connection between interfaces is called a channel. When transmit is run on a 
// peer derivitive each packet in the outStream is sent. When a packet is sent, the target ID of 
// the packet is used to look up the referenced interface and the delay associated with that interface. 
// The packet delay is set between 1 and the delay between the two interfaces (the delay on the channel)
// The method <<SEND>> is then called on the neighbor's interface (not this object but the instance of 
// NetworkInterface in the target peer). <<SEND>> pushes the packet into the _inBoundChannel of the 
// tagets Peers networkInterface
//


#ifndef NetworkInterface_hpp
#define NetworkInterface_hpp

#include <stdio.h>
#include <vector>
#include <map>
#include <deque>
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include "Packet.hpp"

namespace quantas{

    using std::string;
    using std::deque;
    using std::map;
    using std::ostream;
    using std::vector;
    using std::cout;
    using std::ostream;
    using std::left;
    using std::endl;
    using std::setw;
    using std::boolalpha;
    using std::find;

    
    static const int  LOG_WIDTH  = 27;  // var used for column width in loggin
    typedef long      interfaceId;
    //
    // Base Peer class
    //
    template <class message>
    class NetworkInterface{
    private:
        
        typedef deque<Packet<message> >                 aChannel;

        interfaceId                                     _id;
        map<interfaceId,aChannel>                       _inBoundChannels; // channels from all other interfaces into this interface
        map<interfaceId,int>                            _outBoundChannelDelays; // list of channels delays by there target interface id
        // starts at the number of messages a channel could theoretically
        // process per simulation (e.g. 200 for 100 rounds and max 2 delivered
        // per round); is decremented as messages are delivered. used to prevent
        // undeliverable messages from building up in memory
        map<interfaceId,int>                            _outBoundChannelThroughputLeft;
        map<interfaceId, NetworkInterface<message>* >   _outBoundChannels; // list of all other interfaces in the network (weather they are a neighbor or not) use send to send them a message
        deque<Packet<message> >                         _inStream; // messages that have arrived at this peer
        deque<Packet<message> >                         _outStream; // messages waiting to be sent by this peer
        vector<interfaceId>                             _neighbors; // list of interfaces that are directly connected to this one (i.e. they can send messages directly to each other)
        int                                             _maxMsgsRec; // max number of messages recieved per channel per round
        
         // send a message to this peer
        void                               send                  (Packet<message>);

    protected:
        
        // logging
        ostream                                         *_log;
        bool                                            _printNeighborhood;

        // functions for peer
        void                               broadcast             (message msg);
        void                               broadcastBut          (message msg, long id);
        void                               unicast               (message msg);
        void                               unicastTo             (message msg, long dest);
        void                               randomMulticast       (message msg);
    public:
        NetworkInterface                                         ();
        NetworkInterface                                         (interfaceId);
        NetworkInterface                                         (const NetworkInterface &);
        ~NetworkInterface                                        (){};
        // Setters
        void                               setID                 (interfaceId id)                           {_id = id;};
        void                               setLogFile            (ostream &o)                               {_log = &o;};
        void                               printNeighborhoodOn   ()                                         {_printNeighborhood = true;}
        void                               printNeighborhoodOff  ()                                         {_printNeighborhood = false;}
        
        // getters
        vector<interfaceId>                neighbors             ()const                                    {return _neighbors;};
        vector<interfaceId>                channels              ()const;                                   
        interfaceId                        id                    ()const                                    {return _id;};
        bool                               isNeighbor            (interfaceId id)const;
        int                                getDelayToNeighbor    (interfaceId id)const;
        size_t                             outStreamSize         ()const                                    {return _outStream.size();};
        size_t                             inStreamSize          ()const                                    {return _inStream.size();};
        bool                               outStreamEmpty        ()const                                    {return _outStream.empty();};
        bool                               inStreamEmpty         ()const                                    {return _inStream.empty();};

        // mutators
        void                               removeChannel         (const NetworkInterface &neighbor)         {_outBoundChannels.erase(neighbor);};
        void                               addChannel            (NetworkInterface &newNeighbor, int delay, int totalCapacityEver);
        void                               clearMessages         ();
        void                               pushToOutSteam        (Packet<message> outMsg)                   {_outStream.push_back(outMsg);};
        Packet<message>                    popInStream           ();
        void                               addNeighbor           (interfaceId neighborIdAdd)                {_neighbors.push_back(neighborIdAdd);};
        void                               removeNeighbor        (interfaceId neighborIdToRemove);
        void                               setMaxMsgsRec         (int maxMsgsRec)                           {_maxMsgsRec = maxMsgsRec;}

        // moves msgs from the channel to the inStream if msg delay is 0 else decrease msg delay by 1
        void                               receive               ();
       
        // sends all messages in _outStream to there respective targets
        void                               transmit              ();
        
        void                               log                   ()const;
        ostream&                           printTo               (ostream&)const;
        NetworkInterface&                  operator=             (const NetworkInterface&);

        // == and != compare all attributes 
        bool                               operator==            (const NetworkInterface &rhs)const          {return (_id == rhs._id);};
        bool                               operator!=            (const NetworkInterface &rhs)const          {return !(*this == rhs);};
        // greater and less then are based on peer id (standard string comparison)
        bool                               operator<=            (const NetworkInterface &rhs)const          {return (_id <= rhs._id);};
        bool                               operator<             (const NetworkInterface &rhs)const          {return (_id < rhs._id);};
        bool                               operator>=            (const NetworkInterface &rhs)const          {return (_id >= rhs._id);};
        bool                               operator>             (const NetworkInterface &rhs)const          {return (_id > rhs._id);};
        bool                               operator<=            (const interfaceId &rhs)const               {return (_id <= rhs);};
        bool                               operator<             (const interfaceId &rhs)const               {return (_id < rhs);};
        bool                               operator>=            (const interfaceId &rhs)const               {return (_id >= rhs);};
        bool                               operator>             (const interfaceId &rhs)const               {return (_id > rhs);};
        template <class messageType>
        friend ostream&                    operator<<            (ostream&, const NetworkInterface<messageType>&);
    };

    template <class message>
    void NetworkInterface<message>::broadcast(message msg){
        for(auto it = _neighbors.begin(); it != _neighbors.end(); it++){
            Packet<message> outPacket = Packet<message>(-1);
            outPacket.setSource(id());
            outPacket.setTarget(*it);
            outPacket.setMessage(msg);
            _outStream.push_back(outPacket);
        }
    }

    // Send to all neighbors except id
    template <class message>
    void NetworkInterface<message>::broadcastBut(message msg, long ident){
        for(auto it = _neighbors.begin(); it != _neighbors.end(); it++){
            if(*it != ident) {
                Packet<message> outPacket = Packet<message>(-1);
                outPacket.setSource(id());
                outPacket.setTarget(*it);
                outPacket.setMessage(msg);
                _outStream.push_back(outPacket);
            }
        }
    }

    // Send to a single neighbor, here the first one
    template <class message>
    void NetworkInterface<message>::unicast(message msg){
        auto it = _neighbors.begin();
        if (_neighbors.size()>0) {
            Packet<message> outPacket = Packet<message>(-1);
            outPacket.setSource(id());
            outPacket.setTarget(*it);
            outPacket.setMessage(msg);
            _outStream.push_back(outPacket);
        }
    }
    
    // Send to a single designated neighbor
    template <class message>
    void NetworkInterface<message>::unicastTo(message msg, long dest){
        for(auto it = _neighbors.begin(); it != _neighbors.end(); it++){
            if(*it == dest) {
                Packet<message> outPacket = Packet<message>(-1);
                outPacket.setSource(id());
                outPacket.setTarget(*it);
                outPacket.setMessage(msg);
                _outStream.push_back(outPacket);
            }
        }
    }
    
    // Multicasts to a random sample of neighbors without repetition. Size of sample is also random.
    template <class message>
    void NetworkInterface<message>::randomMulticast(message msg) {
       
        // interval: [0, n], where n is the amount of neighbors the particular node calling this function has
        int amountOfNeighbors = uniformInt(0, _neighbors.size());
                
        std::vector<interfaceId> out;
        /* NEED TO USE C++17 OR NEWER FOR std::sample */
        std::sample( // std::sample selects 'amountOfNeighbors' elements from the vector '_neighbors' without repetition. Each possible sample has equal probability of appearance
            _neighbors.begin(), _neighbors.end(),
            std::back_inserter(out),
            amountOfNeighbors,
            RANDOM_GENERATOR
        );

        for (auto it = out.begin(); it != out.end(); ++it) { // iterate through vector where the samples are written and send a message to all of them
            Packet<message> outPacket = Packet<message>(-1);
            outPacket.setSource(id());
            outPacket.setTarget(*it);
            outPacket.setMessage(msg);
            _outStream.push_back(outPacket);
        }
    }
	
    template <class message>
    NetworkInterface<message>::NetworkInterface(){
        _id = NO_PEER_ID;
        _inStream = deque<Packet<message> >();
        _outStream = deque<Packet<message> >();
        _outBoundChannels = map<interfaceId, NetworkInterface<message>* >();
        _outBoundChannelDelays = map<interfaceId,int>();
        _inBoundChannels = map<interfaceId,aChannel>();
        _log = &cout;
        _printNeighborhood = false;
    }

    template <class message>
    NetworkInterface<message>::NetworkInterface(interfaceId id){
        _id = id;
        _inStream = deque<Packet<message> >();
        _outStream = deque<Packet<message> >();
        _outBoundChannels = map<interfaceId, NetworkInterface<message>* >();
        _outBoundChannelDelays = map<interfaceId,int>();
        _inBoundChannels = map<interfaceId,aChannel>();
        _log = &cout;
        _printNeighborhood = false;
    }

    template <class message>
    NetworkInterface<message>::NetworkInterface(const NetworkInterface &rhs){
        _id = rhs._id;
        _inStream = rhs._inStream;
        _outStream = rhs._outStream;
        _outBoundChannels = rhs._outBoundChannels;
        _inBoundChannels = rhs._inBoundChannels;
        _outBoundChannelDelays = rhs._outBoundChannelDelays;
        _log = rhs._log;
        _printNeighborhood = rhs._printNeighborhood;
    }

    template <class message>
    void NetworkInterface<message>::addChannel(NetworkInterface<message> &newNeighbor, int delay, int totalCapacityEver){
        // guard to make sure delay is at least 1, less then 1 will cause errors when calculating delay (division by 0)
        int edgeDelay = delay;
        if(edgeDelay < 1){
            edgeDelay = 1;
        }
        _outBoundChannels[newNeighbor.id()] = &newNeighbor;
        _outBoundChannelDelays[newNeighbor.id()] = edgeDelay;
        _outBoundChannelThroughputLeft[newNeighbor.id()] = totalCapacityEver;
        _inBoundChannels[newNeighbor.id()] = deque<Packet<message> >();
    }

    // called on recever
    template <class message>
    void NetworkInterface<message>::send(Packet<message> outMessage){
        _inBoundChannels.at(outMessage.sourceId()).push_back(outMessage);
    }

    // called on sender
    template <class message>
    void NetworkInterface<message>::transmit(){
        // send all messages to their destination peer channels  
        while(!_outStream.empty()){
			Packet<message> outMessage = _outStream.front();
			_outStream.pop_front();
			if (_id == outMessage.targetId()) {// if sent to self loop back next round
				outMessage.setDelay(1);
				_inStream.push_back(outMessage);
			}
			else if (!isNeighbor(outMessage.targetId()))// skip messages if they are not sent to a neighbor
			{
				continue;
			}
			else {
				interfaceId targetId = outMessage.targetId();
                if (_outBoundChannelThroughputLeft.at(targetId) > 0){
                    --_outBoundChannelThroughputLeft[targetId];
                    int maxDelay = _outBoundChannelDelays.at(targetId);
                    outMessage.setDelay(maxDelay);
                    _outBoundChannels[targetId]->send(outMessage);
                }
			}
		}
    }

    template <class message>
    void NetworkInterface<message>::receive() {
        for (int i = 0; i != _inBoundChannels.size(); ++i) {
            int rec = 0;
            while(!_inBoundChannels[i].empty() && _inBoundChannels[i].front().hasArrived() && rec < _maxMsgsRec){
                _inStream.push_back(_inBoundChannels[i].front());
                _inBoundChannels[i].pop_front();
                rec++;
            }
        }
    }


    template <class message>
    bool NetworkInterface<message>::isNeighbor(interfaceId id)const{
        if(find(_neighbors.begin(), _neighbors.end(), id) != _neighbors.end()){
            return true;
        }
        return false;
    }

    template <class message>
    vector<interfaceId> NetworkInterface<message>::channels()const{
        vector<interfaceId> channelsToPeersByIds = vector<interfaceId>();
        for (auto it=_outBoundChannels.begin(); it!=_outBoundChannels.end(); ++it){
            channelsToPeersByIds.push_back(it->first);
        }
        return channelsToPeersByIds;
    }

    template <class message>
    int NetworkInterface<message>::getDelayToNeighbor(interfaceId id)const{
        return _outBoundChannelDelays.at(id);
    }

    template <class message>
    void NetworkInterface<message>::clearMessages(){
        _inStream.clear();
        _outStream.clear();

        for(auto c : _inBoundChannels){
            c.second.clear();
        }
    }

    template <class message>
    Packet<message> NetworkInterface<message>::popInStream(){
        Packet<message> msg = _inStream.front();
        _inStream.pop_front();
        return msg;
    }

    template <class message>
    void NetworkInterface<message>::removeNeighbor(interfaceId neighborIdToRemove){
        _neighbors.erase(std::remove(_neighbors.begin(), _neighbors.end(), neighborIdToRemove), _neighbors.end());
    }

    template <class message>
    NetworkInterface<message>& NetworkInterface<message>::operator=(const NetworkInterface<message> &rhs){
        if(this == &rhs)
            return *this;
        _id = rhs._id;
        _inStream = rhs._inStream;
        _outStream = rhs._outStream;
        _outBoundChannels = rhs._outBoundChannels;
        _inBoundChannels = rhs._inBoundChannels;
        _outBoundChannelDelays = rhs._outBoundChannelDelays;
        _log = rhs._log;
        _printNeighborhood = rhs._printNeighborhood;

        return *this;
    }

    template <class message>
    void NetworkInterface<message>::log()const{
        printTo(*_log);
    }

    template <class message>
    ostream& NetworkInterface<message>::printTo(ostream &out)const{
        out<< "-- NetworkInterface ID:"<< _id<< " --"<< endl;
        out<< left;
        out<< "\t"<< setw(LOG_WIDTH)<< "In Stream Size"<< setw(LOG_WIDTH)<< "Out Stream Size"<<endl;
        out<< "\t"<< setw(LOG_WIDTH)<< _inStream.size()<< setw(LOG_WIDTH)<< _outStream.size()<<endl<<endl;
        if(_printNeighborhood){
            out<< "\t"<< setw(LOG_WIDTH)<< "Neighbor ID"<< setw(LOG_WIDTH)<< "Delay"<< setw(LOG_WIDTH)<< "Messages In NetworkInterface"<< endl;
            for (auto it=_outBoundChannels.begin(); it!=_outBoundChannels.end(); ++it){
                interfaceId neighborId = it->first;
                out<< "\t"<< setw(LOG_WIDTH)<< neighborId<< setw(LOG_WIDTH)<< getDelayToNeighbor(neighborId)<< setw(LOG_WIDTH)<<  _inBoundChannels.at(neighborId).size()<< endl;
            }
        }
        out << endl;
        return out;
    }

    template <class message>
    ostream& operator<<(ostream &out, const NetworkInterface<message> &peer){
        peer.printTo(out);
        return out;
    }
}
#endif 
