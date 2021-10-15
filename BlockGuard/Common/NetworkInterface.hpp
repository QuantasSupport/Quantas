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
        int                                             _clock;
        map<interfaceId,aChannel>                       _inBoundChannels;// channels from all other interfaces into this interface
        map<interfaceId,int>                            _outBoundChannelDelays;// list of channels delays by there target interface id
        map<interfaceId, NetworkInterface<message>* >   _outBoundChannels; // list of all other interfaces in the network (weather they are a neighbor or not) use send to send them a message
        deque<Packet<message> >                         _inStream;// messages that have arrived at this peer
        deque<Packet<message> >                         _outStream;// messages waiting to be sent by this peer
        vector<interfaceId>                             _neighbors; // list of interfaces that are directly connected to this one (i.e. they can send messages directly to each other)
        
         // send a message to this peer
        void                               send                  (Packet<message>);

    protected:
        
        // logging
        ostream                                         *_log;
        bool                                            _printNeighborhood;

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
        int                                getClock              ()const                                    {return _clock;};
        size_t                             outStreamSize         ()const                                    {return _outStream.size();};
        size_t                             inStreamSize          ()const                                    {return _inStream.size();};

        // mutators
        void                               removeChannel         (const NetworkInterface &neighbor)         {_outBoundChannels.erase(neighbor);};
        void                               addChannel            (NetworkInterface &newNeighbor, int delay);
        void                               clearMessages         ();
        void                               pushToOutSteam        (Packet<message> outMsg)                   {_outStream.push_back(outMsg);};
        Packet<message>                    popInStream           ();
        void                               addNeighbor           (interfaceId neighborIdAdd)                {_neighbors.push_back(neighborIdAdd);};
        void                               removeNeighbor        (interfaceId neighborIdToRemove);

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
        _outBoundChannels = map<interfaceId, NetworkInterface<message>* >();
        _outBoundChannelDelays = map<interfaceId,int>();
        _inBoundChannels = map<interfaceId,aChannel>();
        _log = &cout;
        _printNeighborhood = false;
        _clock = 0;
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
        _clock = 0;
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
        _clock = rhs._clock;
    }

    template <class message>
    void NetworkInterface<message>::addChannel(NetworkInterface<message> &newNeighbor, int delay){
        // guard to make sure delay is at lest 1, less then 1 will couse errors when calculating delay (divisioin by 0)
        int edgeDelay = delay;
        if(edgeDelay < 1){
            edgeDelay = 1;
        }
        _outBoundChannels[newNeighbor.id()] = &newNeighbor;
        _outBoundChannelDelays[newNeighbor.id()] = edgeDelay;
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
        // send all messages to there destantion peer channels  
        while(!_outStream.empty()){
            Packet<message> outMessage = _outStream.front();
            _outStream.pop_front();
            if(!isNeighbor(outMessage.targetId())){continue;}// skip messages if they are not sent to a neighbor

            // if sent to self loop back next round
            if(_id == outMessage.targetId()){
                outMessage.setDelay(1);
                _inStream.push_back(outMessage);
            }else{
                interfaceId targetId = outMessage.targetId();
                int maxDelay = _outBoundChannelDelays.at(targetId);
                outMessage.setDelay(maxDelay);
                _outBoundChannels[targetId]->send(outMessage);
            }
        }
    }

    template <class message>
    void NetworkInterface<message>::receive() {
        _clock++;
        for (auto it = _outBoundChannels.begin(); it != _outBoundChannels.end(); ++it) {
            interfaceId neighborID = it->first;

            for(auto msg = _inBoundChannels.at(neighborID).begin(); msg != _inBoundChannels.at(neighborID).end(); msg++){
                msg->moveForward();
            }

            while(!_inBoundChannels.at(neighborID).empty() && _inBoundChannels.at(neighborID).front().hasArrived()){
                _inStream.push_back(_inBoundChannels.at(neighborID).front());
                _inBoundChannels.at(neighborID).pop_front();
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
