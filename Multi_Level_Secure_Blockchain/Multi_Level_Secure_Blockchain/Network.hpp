//
//  Network.hpp
//  Multi_Level_Secure_Blockchain
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
#include "Peer.hpp"

template<class peer_type>
class Network{
protected:
    
    std::vector<Peer<peer_type>*>   _peers;
    std::default_random_engine      _randomGenerator;
    
    std::string                     createId            ()const;
    bool                            idTaken             (std::string)const;
    std::string                     getUniqueId         ()const;
    void                            addEdges            (Peer<peer_type>*, int avgDelay);
    
public:
    Network                                             ();
    Network                                             (const Network<peer_type>&);
    ~Network                                            ();
    
    // setters
    void                            initNetwork         (const int); // initialize network with peers
    
    //accessors
    int                             size                ()const                              {return _peers.size();};
    
    //mutators
};

template<class peer_type>
Network<peer_type>::Network(){
    _peers = {};
    _randomGenerator = std::default_random_engine();
}

template<class peer_type>
Network<peer_type>::Network(const Network<peer_type> &rhs){
    _peers = rhs._peers;
    _randomGenerator = std::default_random_engine();
}

template<class peer_type>
Network<peer_type>::~Network(){
    for(int i = 0; i < _peers.size(); i++){
        delete _peers[i];
    }
}

template<class peer_type>
std::string Network<peer_type>::createId()const{
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

template<class peer_type>
bool Network<peer_type>::idTaken(std::string id)const{
    for(int i = 0; i < _peers.size(); i++){
        if(_peers[i]->id() == id){
            return true;
        }
    }
    return false;
}

template<class peer_type>
std::string Network<peer_type>::getUniqueId()const{
    std::string id = createId();
    
    while(idTaken(id)){
        id = createId();
    }
    
    return id;
}

template <class peer_type>
void Network<peer_type>::addEdges(Peer<peer_type>* peer, int avgDelay){
    std::poisson_distribution<int> poissonDist(avgDelay);
    
    for(int i = 0; i < _peers.size(); i++){
        if(_peers[i]->id() != peer->id()){
            if(_peers[i]->isNeighbor(peer)){
                int delay = poissonDist(_randomGenerator);
                
            }
        }
    }
}

template<class peer_type>
void Network<peer_type>::initNetwork(int numberOfPeers){
    for(int i = 0; i < numberOfPeers; i++){
        _peers.push_back(new Peer<peer_type>(getUniqueId()));
    }
}


#endif /* Network_hpp */
