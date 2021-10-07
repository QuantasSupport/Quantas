//
//  NetworkNetworkInterface.hpp
//  BlockGuard
//
// This class represents a connection between to peers, this connection is called a NetworkInterface
// The channel has a delay 

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
#include "Packet.hpp"

namespace blockguard{

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

    
    static const int  LOG_WIDTH  = 27;  // var used for column width in loggin
    typedef long      interfaceId;
    //
    // Base Peer class
    //
    template <class message>
    class NetworkInterface{
    protected:
        
        typedef deque<Packet<message> >                 aChannel;

        interfaceId                                     _id;
        int                                             _clock;
        map<interfaceId,aChannel>                       _channels;// list of chanels between this peer and others
        map<interfaceId,int>                            _channelDelays;// list of channels and there delays
        map<interfaceId, NetworkInterface<message>* >   _neighbors; // peers this peer has a link to
        deque<Packet<message> >                         _inStream;// messages that have arrived at this peer
        deque<Packet<message> >                         _outStream;// messages waiting to be sent by this peer
        
        // logging
        ostream                                         *_log;
        bool                                            _printNeighborhood;
        
    public:
        NetworkInterface                                         ();
        NetworkInterface                                         (interfaceId);
        NetworkInterface                                         (const NetworkInterface &);
        ~NetworkInterface                                        (){};
        // Setters
        void                               setID                 (interfaceId id)                            {_id = id;};
        void                               setLogFile            (ostream &o)                                {_log = &o;};
        void                               printNeighborhoodOn   ()                                          {_printNeighborhood = true;}
        void                               printNeighborhoodOff  ()                                          {_printNeighborhood = false;}
        
        // getters
        vector<long>                       neighbors             ()const;
        long                               id                    ()const                                     {return _id;};
        bool                               isNeighbor            (interfaceId id)const;
        int                                getDelayToNeighbor    (interfaceId id)const;
        int                                getClock              ()const                                     {return _clock;};
        deque<Packet<message> >            getInStream           ()const                                     {return _inStream;};
        deque<Packet<message> >            getOutStream          ()const                                     {return _outStream;};
        deque<Packet<message> >*           getChannelFrom        (const string id)                           {return &(_channels.at(id));};
        
        // mutators
        void                               removeNeighbor        (const NetworkInterface &neighbor)          {_neighbors.erase(neighbor);};
        void                               addNeighbor           (NetworkInterface &newNeighbor, int delay);
        void                               clearMessages         ();
        void                               addToOutSteam         ();


        // moves msgs from the channel to the inStream if msg delay is 0 else decrease msg delay by 1
        void                               receive               ();
        // send a message to this peer
        void                               send                  (Packet<message>);
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
        friend ostream&                    operator<<            (ostream&, const NetworkInterface&);
    };

    //
    // Base NetworkInterface definitions
    //

    template <class message>
    NetworkInterface<message>::NetworkInterface(){
        _id = NO_PEER_ID;
        _inStream = deque<Packet<message> >();
        _outStream = deque<Packet<message> >();
        _neighbors = map<interfaceId, NetworkInterface<message>* >();
        _channelDelays = map<interfaceId,int>();
        _channels = map<interfaceId,aChannel>();
        _log = &cout;
        _printNeighborhood = false;
        _clock = 0;
    }

    template <class message>
    NetworkInterface<message>::NetworkInterface(interfaceId id){
        _id = id;
        _inStream = deque<Packet<message> >();
        _outStream = deque<Packet<message> >();
        _neighbors = map<interfaceId, NetworkInterface<message>* >();
        _channelDelays = map<interfaceId,int>();
        _channels = map<interfaceId,aChannel>();
        _log = &cout;
        _printNeighborhood = false;
        _clock = 0;
    }

    template <class message>
    NetworkInterface<message>::NetworkInterface(const NetworkInterface &rhs){
        _id = rhs._id;
        _inStream = rhs._inStream;
        _outStream = rhs._outStream;
        _neighbors = rhs._neighbors;
        _channels = rhs._channels;
        _channelDelays = rhs._channelDelays;
        _log = rhs._log;
        _printNeighborhood = rhs._printNeighborhood;
        _clock = rhs._clock;
    }

    template <class message>
    void NetworkInterface<message>::addNeighbor(NetworkInterface<message> &newNeighbor, int delay){
        // grourd to make sure delay is at lest 1, less then 1 will couse errors when calculating delay (divisioin by 0)
        int edgeDelay = delay;
        if(edgeDelay < 1){
            edgeDelay = 1;
        }
        _neighbors[newNeighbor.id()] = &newNeighbor;
        _channelDelays[newNeighbor.id()] = edgeDelay;
        _channels[newNeighbor.id()] = deque<Packet<message> >();
    }

    // called on recever
    template <class message>
    void NetworkInterface<message>::send(Packet<message> outMessage){
        _channels.at(outMessage.sourceId()).push_back(outMessage);
    }

    // called on sender
    template <class message>
    void NetworkInterface<message>::transmit(){
        // send all messages to there destantion peer channels  
        while(!_outStream.empty()){
            
            Packet<message> outMessage = _outStream.front();
            _outStream.pop_front();
            
            // if sent to self loop back next round
            if(_id == outMessage.targetId()){
                outMessage.setDelay(1);
                _inStream.push_back(outMessage);
            }else{
                interfaceId targetId = outMessage.targetId();
                int maxDelay = _channelDelays.at(targetId);
                outMessage.setDelay(maxDelay);
                _neighbors[targetId]->send(outMessage);
            }
        }
    }

    template <class message>
    void NetworkInterface<message>::receive() {
        _clock++;
        for (auto it = _neighbors.begin(); it != _neighbors.end(); ++it) {
            interfaceId neighborID = it->first;

            for(auto msg = _channels.at(neighborID).begin(); msg != _channels.at(neighborID).end(); msg++){
                msg->moveForward();
            }

            while(!_channels.at(neighborID).empty() && _channels.at(neighborID).front().hasArrived()){
                _inStream.push_back(_channels.at(neighborID).front());
                _channels.at(neighborID).pop_front();
            }
        }
    }



    template <class message>
    bool NetworkInterface<message>::isNeighbor(interfaceId id)const{
        if(_neighbors.find(id) != _neighbors.end()){
            return true;
        }
        return false;
    }

    template <class message>
    vector<long> NetworkInterface<message>::neighbors()const{
        vector<interfaceId> neighborIds = vector<interfaceId>();
        for (auto it=_neighbors.begin(); it!=_neighbors.end(); ++it){
            neighborIds.push_back(it->first);
        }
        return neighborIds;
    }

    template <class message>
    int NetworkInterface<message>::getDelayToNeighbor(interfaceId id)const{
        return _channelDelays.at(id);
    }

    template <class message>
    void NetworkInterface<message>::clearMessages(){
        _inStream.clear();
        _outStream.clear();

        for(auto c : _channels){
            c.second.clear();
        }
    }

    template <class message>
    NetworkInterface<message>& NetworkInterface<message>::operator=(const NetworkInterface<message> &rhs){
        if(this == &rhs)
            return *this;
        _id = rhs._id;
        _inStream = rhs._inStream;
        _outStream = rhs._outStream;
        _neighbors = rhs._neighbors;
        _channels = rhs._channels;
        _channelDelays = rhs._channelDelays;
        _log = rhs._log;
        _printNeighborhood = rhs._printNeighborhood;
        _clock = rhs._clock;

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
            for (auto it=_neighbors.begin(); it!=_neighbors.end(); ++it){
                interfaceId neighborId = it->first;
                out<< "\t"<< setw(LOG_WIDTH)<< neighborId<< setw(LOG_WIDTH)<< getDelayToNeighbor(neighborId)<< setw(LOG_WIDTH)<<  _channels.at(neighborId).size()<< endl;
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
