//
//  Network.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/24/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef Network_hpp
#define Network_hpp

#include <stdio.h>
#include <vector>
#include <string>
#include <random>
#include <limits>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <memory>
#include "./../Common/Interface.hpp"

namespace blockguard{

    using std::string;
    using std::ostream;
    using std::vector;
    using std::cout;
    using std::uniform_int_distribution;
    using std::poisson_distribution;
    using std::endl;
    using std::left;
    using std::setw;


    static const string                POISSON = "POISSON";
    static const string                RANDOM  = "RANDOM";
    static const string                ONE     = "ONE";
    template<class type_msg, class peer_type>
    class Network{
    protected:

        vector<Interface<type_msg>*>        _peers;
        int                                 _avgDelay;
        int                                 _maxDelay;
        int                                 _minDelay;
        string                              _distribution;

        ostream                             *_log;

        // string                              createId            ();
        // bool                                idTaken             (string);
        // string                              getUniqueId         ();
        void                                addEdges            (Interface<type_msg>*);
        int                                 getDelay            ();
        peer_type*							getPeerById			(string);

    public:
        Network                                                 ();
        Network                                                 (const Network<type_msg,peer_type>&);
        ~Network                                                ();

        // setters
        void                                initNetwork         (int); // initialize network with peers
        void                                setMaxDelay         (int d)                                         {_maxDelay = d;};
        void                                setAvgDelay         (int d)                                         {_avgDelay = d;};
        void                                setMinDelay         (int d)                                         {_minDelay = d;};
        void                                setToRandom         ()                                              {_distribution = RANDOM;};
        void                                setToPoisson        ()                                              {_distribution = POISSON;};
        void                                setToOne            ()                                              {_distribution = ONE;};
        void                                setLog              (ostream&);

        // getters
        int                                 size                ()const                                         {return (int)_peers.size();};
        int                                 maxDelay            ()const                                         {return _maxDelay;};
        int                                 avgDelay            ()const                                         {return _avgDelay;};
        int                                 minDelay            ()const                                         {return _minDelay;};
        string                              distribution        ()const                                         {return _distribution;};


        //mutators
        void                                receive             ();
        void                                preformComputation  ();
        void                                transmit            ();
        void                                makeRequest         (int i)                                         {_peers[i]->makeRequest();};
        // void                                shuffleByzantines   (int);

        // logging and debugging
        ostream&                            printTo             (ostream&)const;
        void                                log                 ()const                                         {printTo(*_log);};

        // operators
        Network&                            operator=           (const Network&);
        peer_type*                          operator[]          (int);
        const peer_type*                    operator[]          (int)const;
        friend ostream&                     operator<<          (ostream &out, const Network &system)      {return system.printTo(out);};
        void 								makeRequest			(Interface<type_msg> * peer)				            { peer->makeRequest(); }
        int									pickSecurityLevel	(int);

    };

    template<class type_msg, class peer_type>
    Network<type_msg,peer_type>::Network(){
        _peers = vector<Interface<type_msg>*>();
        _avgDelay = 1;
        _maxDelay = 1;
        _minDelay = 1;
        _distribution = RANDOM;
        _log = &cout;
    }

    template<class type_msg, class peer_type>
    Network<type_msg,peer_type>::Network(const Network<type_msg,peer_type> &rhs){
        if(this == &rhs){
            return;
        }

        for(int i = 0; i < _peers.size(); i++){
            delete _peers[i];
        }

        _peers = vector<Interface<type_msg>*>();
        for(int i = 0; i < rhs._peers.size(); i++){
            _peers.push_back(new peer_type(*dynamic_cast<peer_type*>(rhs._peers[i])));
        }
        _avgDelay = rhs._avgDelay;
        _maxDelay = rhs._maxDelay;
        _minDelay = rhs._minDelay;
        _distribution = rhs._distribution;
        _log = rhs._log;
    }

    template<class type_msg, class peer_type>
    Network<type_msg,peer_type>::~Network(){
        for(int i = 0; i < _peers.size(); i++){
            delete _peers[i];
        }
    }

    template<class type_msg, class peer_type>
    void Network<type_msg,peer_type>::setLog(ostream &out){
        _log = &out;
        for(int i = 0; i < _peers.size(); i++){
            peer_type *p = dynamic_cast<peer_type*>(_peers[i]);
            p->setLogFile(out);
        }
    }

    template<class type_msg, class peer_type>
    void Network<type_msg,peer_type>::addEdges(Interface<type_msg> *peer){
        for(int i = 0; i < _peers.size(); i++){
            if(_peers[i]->id() != peer->id()){
                if(!_peers[i]->isNeighbor(peer->id())){
                    int delay = getDelay();
                    // guard agenst 0 and negative numbers
                    while(delay < 1 || delay > _maxDelay || delay < _minDelay){
                        delay = getDelay();
                    }
                    peer->addNeighbor(*_peers[i], delay);
                    _peers[i]->addNeighbor(*peer,delay);
                }
            }
        }
    }

    template<class type_msg, class peer_type>
    int Network<type_msg,peer_type>::getDelay(){
        if(_distribution == RANDOM){
            uniform_int_distribution<int> randomDistribution(_minDelay,_maxDelay);
            return randomDistribution(RANDOM_GENERATOR);
        }
        if(_distribution == POISSON){
            poisson_distribution<int> poissonDistribution(_avgDelay);
            return poissonDistribution(RANDOM_GENERATOR);
        }
        if(_distribution == ONE){
            return 1;
        }
        return -1;
    }

    template<class type_msg, class peer_type>
    void Network<type_msg,peer_type>::initNetwork(int numberOfPeers){
        for(int i = 0; i < numberOfPeers; i++){
            _peers.push_back(new peer_type(i));
        }
        for(int i = 0; i < _peers.size(); i++){
            addEdges(_peers[i]);
        }
    }

    template<class type_msg, class peer_type>
    void Network<type_msg,peer_type>::receive(){
        for(int i = 0; i < _peers.size(); i++){
            _peers[i]->receive();
        }
    }

    template<class type_msg, class peer_type>
    void Network<type_msg,peer_type>::preformComputation(){
        for(int i = 0; i < _peers.size(); i++){
            _peers[i]->preformComputation();
        }
    }

    template<class type_msg, class peer_type>
    void Network<type_msg,peer_type>::transmit(){
        for(int i = 0; i < _peers.size(); i++){
            _peers[i]->transmit();
        }
    }

    template<class type_msg, class peer_type>
    ostream& Network<type_msg,peer_type>::printTo(ostream &out)const{
        out<< "--- NETWROK SETUP ---"<< endl<< endl;
        out<< left;
        out<< '\t'<< setw(LOG_WIDTH)<< "Number of Peers"<< setw(LOG_WIDTH)<< "Distribution"<< setw(LOG_WIDTH)<< "Min Delay"<< setw(LOG_WIDTH)<< "Average Delay"<< setw(LOG_WIDTH)<< "Max Delay"<< endl;
        out<< '\t'<< setw(LOG_WIDTH)<< _peers.size()<< setw(LOG_WIDTH)<< _distribution<< setw(LOG_WIDTH)<< _minDelay<< setw(LOG_WIDTH)<< _avgDelay<< setw(LOG_WIDTH)<< _maxDelay<< endl;

        for(int i = 0; i < _peers.size(); i++){
            peer_type *p = dynamic_cast<peer_type*>(_peers[i]);
            p->printTo(out);
        }

        return out;
    }

    template<class type_msg, class peer_type>
    Network<type_msg, peer_type>& Network<type_msg,peer_type>::operator=(const Network<type_msg, peer_type> &rhs){
        if(this == &rhs){
            return *this;
        }

        for(int i = 0; i < _peers.size(); i++){
            delete _peers[i];
        }

        _peers = vector<Interface<type_msg>*>();
        for(int i = 0; i < rhs._peers.size(); i++){
            _peers.push_back(new peer_type(*dynamic_cast<peer_type*>(rhs._peers[i])));
        }

        _avgDelay = rhs._avgDelay;
        _maxDelay = rhs._maxDelay;
        _minDelay = rhs._minDelay;
        _distribution = rhs._distribution;

        return *this;
    }

    template<class type_msg, class peer_type>
    peer_type* Network<type_msg,peer_type>::operator[](int i){
        return dynamic_cast<peer_type*>(_peers[i]);
    }

    template<class type_msg, class peer_type>
    const peer_type* Network<type_msg,peer_type>::operator[](int i)const{
        return dynamic_cast<peer_type*>(_peers[i]);
    }

    template<class type_msg, class peer_type>
    peer_type* Network<type_msg,peer_type>::getPeerById(string id){
        for(int i = 0; i<_peers.size(); i++){
            if(_peers[i]->id() ==  id)
                return dynamic_cast<peer_type*>(_peers[i]);
        }
        return nullptr;
    }
}
#endif /* Network_hpp */
