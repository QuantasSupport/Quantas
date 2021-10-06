//
//  Interface.hpp
//  BlockGuard
//
// This class represents a connection between to peers, this connection is called a Interface
// The channel has a delay 

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

    //
    // Base Peer class
    //
    template <class message>
    class Interface{
    protected:
            // type abbreviations
        typedef deque<Packet<message> >    aChannel;
        typedef long                       peerId;

        peerId                             _id;
        int                                _clock;
        map<peerId,aChannel>               _channels;// list of chanels between this peer and others
        map<peerId,int>                    _channelDelays;// list of channels and there delays
        map<peerId, Interface<message>* >  _neighbors; // peers this peer has a link to
        deque<Packet<message> >            _inStream;// messages that have arrived at this peer
        deque<Packet<message> >            _outStream;// messages waiting to be sent by this peer
        
        // metrics
        int                                _numberOfMessagesSent;
        
        // logging
        ostream                            *_log;
        bool                               _printNeighborhood;
        
    public:
        Interface                                                ();
        Interface                                                (peerId);
        Interface                                                (const Interface &);
        virtual ~Interface                                       ()=0;
        // Setters
        void                               setID                 (peerId id)                         {_id = id;};
        void                               setLogFile            (ostream &o)                        {_log = &o;};
        void                               printNeighborhoodOn   ()                                  {_printNeighborhood = true;}
        void                               printNeighborhoodOff  ()                                  {_printNeighborhood = false;}
        // getters
        vector<long>                       neighbors             ()const;
        long                               id                    ()const                             {return _id;};
        bool                               isNeighbor            (peerId id)const;
        int                                getDelayToNeighbor    (peerId id)const;
        int                                getMessageCount       ()const                             {return _numberOfMessagesSent;};
        int                                getClock              ()const                             {return _clock;};
        deque<Packet<message> >            getInStream           ()const                             {return _inStream;};
        deque<Packet<message> >            getOutStream          ()const                             {return _outStream;};
        deque<Packet<message> >*           getChannelFrom        (const string id)                   {return &(_channels.at(id));};
        
        // mutators
        void                               removeNeighbor        (const Interface &neighbor)         {_neighbors.erase(neighbor);};
        void                               addNeighbor           (Interface &newNeighbor, int delay);

        // moves msgs from the channel to the inStream if msg delay is 0 else decrease msg delay by 1
        void                               receive               ();
        // send a message to this peer
        void                               send                  (Packet<message>);
        // sends all messages in _outStream to there respective targets
        void                               transmit              ();
        

        void                               log                   ()const;
        ostream&                           printTo               (ostream&)const;
        Interface&                         operator=             (const Interface&);

        // == and != compare all attributes 
        bool                               operator==            (const Interface &rhs)const         {return (_id == rhs._id);};
        bool                               operator!=            (const Interface &rhs)const         {return !(*this == rhs);};
        // greater and less then are based on peer id (standard string comparison)
        bool                               operator<=            (const Interface &rhs)const         {return (_id <= rhs._id);};
        bool                               operator<             (const Interface &rhs)const         {return (_id < rhs._id);};
        bool                               operator>=            (const Interface &rhs)const         {return (_id >= rhs._id);};
        bool                               operator>             (const Interface &rhs)const         {return (_id > rhs._id);};
        bool                               operator<=            (const peerId &rhs)const            {return (_id <= rhs);};
        bool                               operator<             (const peerId &rhs)const            {return (_id < rhs);};
        bool                               operator>=            (const peerId &rhs)const            {return (_id >= rhs);};
        bool                               operator>             (const peerId &rhs)const            {return (_id > rhs);};
        friend ostream&                    operator<<            (ostream&, const Interface&);
    };

    //
    // Base Interface definitions
    //

    template <class message>
    Interface<message>::Interface(){
        _id = NO_PEER_ID;
        _inStream = deque<Packet<message> >();
        _outStream = deque<Packet<message> >();
        _neighbors = map<peerId, Interface<message>* >();
        _channelDelays = map<peerId,int>();
        _channels = map<peerId,aChannel>();
        _log = &cout;
        _byzantine = false;
        _numberOfMessagesSent = 0;
        _printNeighborhood = false;
        _busy = false;
        _clock = 0;
    }

    template <class message>
    Interface<message>::Interface(peerId id){
        _id = id;
        _inStream = deque<Packet<message> >();
        _outStream = deque<Packet<message> >();
        _neighbors = map<peerId, Interface<message>* >();
        _channelDelays = map<peerId,int>();
        _channels = map<peerId,aChannel>();
        _log = &cout;
        _byzantine = false;
        _numberOfMessagesSent = 0;
        _printNeighborhood = false;
        _busy = false;
        _clock = 0;
    }

    template <class message>
    Interface<message>::Interface(const Interface &rhs){
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
    Interface<message>::~Interface(){
    }

    template <class message>
    void Interface<message>::addNeighbor(Interface<message> &newNeighbor, int delay){
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
    void Interface<message>::send(Packet<message> outMessage){
        _channels.at(outMessage.sourceId()).push_back(outMessage);
    }

    // called on sender
    template <class message>
    void Interface<message>::transmit(){
        // send all messages to there destantion peer channels  
        while(!_outStream.empty()){
            
            Packet<message> outMessage = _outStream.front();
            _outStream.pop_front();
            
            // if sent to self loop back next round
            if(_id == outMessage.targetId()){
                outMessage.setDelay(1);
                _inStream.push_back(outMessage);
            }else{
                peerId targetId = outMessage.targetId();
                int maxDelay = _channelDelays.at(targetId);
                outMessage.setDelay(maxDelay);
                _neighbors[targetId]->send(outMessage);
            }
            ++_numberOfMessagesSent;
        }
    }

    template <class message>
    void Interface<message>::receive() {
        _clock++;
        for (auto it = _neighbors.begin(); it != _neighbors.end(); ++it) {
            peerId neighborID = it->first;

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
    bool Interface<message>::isNeighbor(peerId id)const{
        if(_neighbors.find(id) != _neighbors.end()){
            return true;
        }
        return false;
    }

    template <class message>
    vector<long> Interface<message>::neighbors()const{
        vector<peerId> neighborIds = vector<peerId>();
        for (auto it=_neighbors.begin(); it!=_neighbors.end(); ++it){
            neighborIds.push_back(it->first);
        }
        return neighborIds;
    }

    template <class message>
    int Interface<message>::getDelayToNeighbor(peerId id)const{
        return _channelDelays.at(id);
    }

    template <class message>
    void Interface<message>::clearMessages(){
        _inStream.clear();
        _outStream.clear();

        for(auto c : _channels){
            c.second.clear();
        }
    }

    template <class message>
    Interface<message>& Interface<message>::operator=(const Interface<message> &rhs){
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
    void Interface<message>::log()const{
        printTo(*_log);
    }

    template <class message>
    ostream& Interface<message>::printTo(ostream &out)const{
        out<< "-- Interface ID:"<< _id<< " --"<< endl;
        out<< left;
        out<< "\t"<< setw(LOG_WIDTH)<< "In Stream Size"<< setw(LOG_WIDTH)<< "Out Stream Size"<< setw(LOG_WIDTH)<< "Message Count"<< setw(LOG_WIDTH)<< "Is Byzantine"<<endl;
        out<< "\t"<< setw(LOG_WIDTH)<< _inStream.size()<< setw(LOG_WIDTH)<< _outStream.size()<<setw(LOG_WIDTH)<< _numberOfMessagesSent<< setw(LOG_WIDTH)<<setw(LOG_WIDTH)<< boolalpha<<  _byzantine<<endl<<endl;
        if(_printNeighborhood){
            out<< "\t"<< setw(LOG_WIDTH)<< "Neighbor ID"<< setw(LOG_WIDTH)<< "Delay"<< setw(LOG_WIDTH)<< "Messages In Interface"<< endl;
            for (auto it=_neighbors.begin(); it!=_neighbors.end(); ++it){
                peerId neighborId = it->first;
                out<< "\t"<< setw(LOG_WIDTH)<< neighborId<< setw(LOG_WIDTH)<< getDelayToNeighbor(neighborId)<< setw(LOG_WIDTH)<<  _channels.at(neighborId).size()<< endl;
            }
        }
        out << endl;
        return out;
    }

    template <class message>
    ostream& operator<<(ostream &out, const Interface<message> &peer){
        peer.printTo(out);
        return out;
    }
}
#endif /* Peer_hpp */
