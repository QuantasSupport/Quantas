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
#include "Peer.hpp"

static const std::string POISSON = "POISSON";
static const std::string RANDOM  = "RANDOM";
static const std::string ONE     = "ONE";

template<class type_msg, class peer_type>
class Network{
protected:
    
    std::vector<Peer<type_msg>*>        _peers;
    std::default_random_engine          _randomGenerator;
    int                                 _avgDelay;
    int                                 _maxDelay;
    int                                 _minDelay;
    std::string                         _distribution;
    
    std::ostream                         *_log;
    
    std::string                         createId            ();
    bool                                idTaken             (std::string);
    std::string                         getUniqueId         ();
    void                                addEdges            (Peer<type_msg>*);
    int                                 getDelay            ();
    
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
    void                                setLog              (std::ostream&);
    
    // getters
    int                                 size                ()const                                         {return (int)_peers.size();};
    int                                 maxDelay            ()const                                         {return _maxDelay;};
    int                                 avgDelay            ()const                                         {return _avgDelay;};
    int                                 minDelay            ()const                                         {return _minDelay;};
    std::string                         distribution        ()const                                         {return _distribution;};
    
    //mutators
    void                                receive             ();
    void                                preformComputation  ();
    void                                transmit            ();
    void                                makeRequest         (int i)                                         {_peers[i]->makeRequest();};
    
    void                                shuffleByzantines   (int);
    // logging and debugging
    std::ostream&                       printTo             (std::ostream&)const;
    void                                log                 ()const                                         {printTo(*_log);};
    
    // operators
    Network&                            operator=           (const Network&);
    peer_type*                          operator[]          (int);
    const peer_type*                    operator[]          (int)const;
    friend std::ostream&                operator<<          (std::ostream &out, const Network &system)      {return system.printTo(out);};
};

template<class type_msg, class peer_type>
Network<type_msg,peer_type>::Network(){
    _peers = std::vector<Peer<type_msg>*>();
    int seed = (int)time(nullptr);
    _randomGenerator = std::default_random_engine(seed);
    _avgDelay = 0;
    _maxDelay = std::numeric_limits<int>::max();
    _minDelay = std::numeric_limits<int>::min();
    _distribution = RANDOM;
    _log = &std::cout;
}

template<class type_msg, class peer_type>
Network<type_msg,peer_type>::Network(const Network<type_msg,peer_type> &rhs){
    if(this == &rhs){
        return;
    }
    
    for(int i = 0; i < _peers.size(); i++){
        delete _peers[i];
    }

    _peers = std::vector<Peer<type_msg>*>();
    for(int i = 0; i < rhs._peers.size(); i++){
        _peers.push_back(new peer_type(*dynamic_cast<peer_type*>(rhs._peers[i])));
    }
    int seed = (int)time(nullptr);
    _randomGenerator = std::default_random_engine(seed);
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
void Network<type_msg,peer_type>::setLog(std::ostream &out){
    _log = &out;
    for(int i = 0; i < _peers.size(); i++){
        peer_type *p = dynamic_cast<peer_type*>(_peers[i]);
        p->setLogFile(out);
    }
}

template<class type_msg, class peer_type>
std::string Network<type_msg,peer_type>::createId(){
    char firstPos = '*';
    char secondPos = '*';
    char thirdPos = '*';
    char fourthPos = '*';
    char fifthPos = '*';

    std::uniform_int_distribution<int> uniformDist(0,25);
    // add 'A' to shift char into rnage of upper case letters
    firstPos = uniformDist(_randomGenerator) + 'A';
    secondPos = uniformDist(_randomGenerator) + 'A';
    thirdPos = uniformDist(_randomGenerator) + 'A';
    fourthPos = uniformDist(_randomGenerator) + 'A';
    fifthPos = uniformDist(_randomGenerator) + 'A';
    
    std::string id = "";
    id = id + firstPos + secondPos + thirdPos + fourthPos + fifthPos;
    return id;
}

template<class type_msg, class peer_type>
bool Network<type_msg,peer_type>::idTaken(std::string id){
    for(int i = 0; i < _peers.size(); i++){
        if(_peers[i]->id() == id){
            return true;
        }
    }
    return false;
}

template<class type_msg, class peer_type>
std::string Network<type_msg,peer_type>::getUniqueId(){
    std::string id = createId();
    
    while(idTaken(id)){
        id = createId();
    }
    
    return id;
}

template<class type_msg, class peer_type>
void Network<type_msg,peer_type>::addEdges(Peer<type_msg> *peer){
    for(int i = 0; i < _peers.size(); i++){
        if(_peers[i]->id() != peer->id()){
            if(!_peers[i]->isNeighbor(peer->id())){
                int delay = getDelay();
                // guard agenst 0 and negative numbers
                while(delay < 1 && delay > _maxDelay && delay < _minDelay){
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
        std::uniform_int_distribution<int> randomDistribution(_minDelay,_maxDelay);
        return randomDistribution(_randomGenerator);
    }
    if(_distribution == POISSON){
        std::poisson_distribution<int> poissonDistribution(_avgDelay);
        return poissonDistribution(_randomGenerator);
    }
    if(_distribution == ONE){
        return 1;
    }
    return -1;
}

template<class type_msg, class peer_type>
void Network<type_msg,peer_type>::initNetwork(int numberOfPeers){
    for(int i = 0; i < numberOfPeers; i++){
        _peers.push_back(new peer_type(getUniqueId()));
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
std::ostream& Network<type_msg,peer_type>::printTo(std::ostream &out)const{
    out<< "--- NETWROK SETUP ---"<< std::endl<< std::endl;
    out<< std::left;
    out<< '\t'<< std::setw(LOG_WIDTH)<< "Number of Peers"<< std::setw(LOG_WIDTH)<< "Distribution"<< std::setw(LOG_WIDTH)<< "Min Delay"<< std::setw(LOG_WIDTH)<< "Average Delay"<< std::setw(LOG_WIDTH)<< "Max Delay"<< std::endl;
    out<< '\t'<< std::setw(LOG_WIDTH)<< _peers.size()<< std::setw(LOG_WIDTH)<< _distribution<< std::setw(LOG_WIDTH)<< _minDelay<< std::setw(LOG_WIDTH)<< _avgDelay<< std::setw(LOG_WIDTH)<< _maxDelay<< std::endl;
    
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

    _peers = std::vector<Peer<type_msg>*>();
    for(int i = 0; i < rhs._peers.size(); i++){
        _peers.push_back(new peer_type(*dynamic_cast<peer_type*>(rhs._peers[i])));
    }
    
    int seed = (int)std::chrono::system_clock::now().time_since_epoch().count();
    _randomGenerator = std::default_random_engine(seed);
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
void Network<type_msg,peer_type>::shuffleByzantines(int shuffleCount){
    int shuffled = 0;
    //find list of byzantineFlag peers
    std::vector<int> byzantineIndex;
    std::vector<int> nonByzantineIndex;

    for(int i = 0; i<_peers.size();i++){
        if(_peers[i]->isByzantine()){
            byzantineIndex.push_back (i);
        }else if(!_peers[i]->isByzantine()){
            nonByzantineIndex.push_back (i);
        }
    }

    while (shuffled<shuffleCount){
        //find list of byzantineFlag peers
        int byzantineShuffleIndex = static_cast<int>(rand() % byzantineIndex.size());
        int nonByzantineShuffleIndex = static_cast<int>(rand() % nonByzantineIndex.size());
        _peers[byzantineIndex[byzantineShuffleIndex]]->setByzantineFlag(false);
        _peers[nonByzantineIndex[nonByzantineShuffleIndex]]->setByzantineFlag(true);
        byzantineIndex.erase(byzantineIndex.begin ()+byzantineShuffleIndex);
        nonByzantineIndex.erase(nonByzantineIndex.begin ()+nonByzantineShuffleIndex);
        shuffled++;
    }


}

#endif /* Network_hpp */
