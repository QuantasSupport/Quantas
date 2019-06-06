//
//  ByzantineNetwork.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 5/23/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef ByzantineNetwork_hpp
#define ByzantineNetwork_hpp

#include "Network.hpp"

template<class type_msg, class peer_type>
class ByzantineNetwork : public  Network<type_msg,peer_type>{
protected:

    int                                 _numberOfByzantines;
    int                                 _numberOfCorrect;

    // logging
    std::ostream                                                    *_log;
    
public:
    ByzantineNetwork                    ();
    ByzantineNetwork                    (const ByzantineNetwork<type_msg,peer_type>&);
    ~ByzantineNetwork                   ();

    // setters
    void                                setLog              (std::ostream&);

    // getters
    std::vector<peer_type*>             getByzantine        ()const;
    std::vector<peer_type*>             getCorrect          ()const;

    // mutators
    void                                shuffleByzantines   (int);
    void                                makeByzantines      (int);
    void                                makeCorrect         (int);

    // overrides
    void                                initNetwork         (int);

    // logging and debugging
    std::ostream&                       printTo             (std::ostream&)const;
    void                                log                 ()const                                             {printTo(*_log);};

    // operators
    ByzantineNetwork&                   operator=           (const ByzantineNetwork<type_msg,peer_type>&);
    friend std::ostream&                operator<<          (std::ostream &out, const ByzantineNetwork &system) {return system.printTo(out);};

};

template<class type_msg, class peer_type>
ByzantineNetwork<type_msg,peer_type>::ByzantineNetwork() : Network<type_msg,peer_type>(){
    _numberOfByzantines = 0;
    _numberOfCorrect = 0;
}

template<class type_msg, class peer_type>
ByzantineNetwork<type_msg,peer_type>::ByzantineNetwork(const ByzantineNetwork &rhs) : Network<type_msg,peer_type>(rhs){
    if(this == &rhs){
        return;
    }

    _numberOfByzantines = rhs._numberOfByzantines;
    _numberOfCorrect = rhs._numberOfCorrect;
}

template<class type_msg, class peer_type>
ByzantineNetwork<type_msg,peer_type>::~ByzantineNetwork(){

}

template<class type_msg, class peer_type>
ByzantineNetwork<type_msg,peer_type>& ByzantineNetwork<type_msg,peer_type>::operator=(const ByzantineNetwork &rhs){
    if(this == &rhs){
        return *this;
    }

    Network<type_msg,peer_type>::operator=(rhs);

    _numberOfByzantines = rhs._numberOfByzantines;
    _numberOfCorrect = rhs._numberOfCorrect;

    return *this;
}

template<class type_msg, class peer_type>
std::vector<peer_type*> ByzantineNetwork<type_msg,peer_type>::getByzantine()const{
    std::vector<peer_type*> byzantinePeers = std::vector<peer_type*>();
    for(int i = 0; i < Network<type_msg,peer_type>::_peers.size(); i++){
        if(Network<type_msg,peer_type>::_peers[i]->isByzantine()){
            byzantinePeers.push_back(dynamic_cast<peer_type*>(Network<type_msg,peer_type>::_peers[i]));
        }
    }
    return byzantinePeers;
}

template<class type_msg, class peer_type>
std::vector<peer_type*> ByzantineNetwork<type_msg,peer_type>::getCorrect()const{
    std::vector<peer_type*> correctPeers = std::vector<peer_type*>();
    for(int i = 0; i < Network<type_msg,peer_type>::_peers.size(); i++){
        if(!Network<type_msg,peer_type>::_peers[i]->isByzantine()){
            correctPeers.push_back(dynamic_cast<peer_type*>(Network<type_msg,peer_type>::_peers[i]));
        }
    }
    return correctPeers;   
}

template<class type_msg, class peer_type>
void ByzantineNetwork<type_msg,peer_type>::makeByzantines(int numberOfPeers){
    if(numberOfPeers + _numberOfByzantines > Network<type_msg,peer_type>::_peers.size()){
        _numberOfByzantines = Network<type_msg,peer_type>::_peers.size();
        _numberOfCorrect = 0;
    }else{
        _numberOfByzantines += numberOfPeers;
        _numberOfCorrect = _numberOfCorrect - numberOfPeers;
    }
    int i = 0;
    while(getByzantine().size() < _numberOfByzantines){
        if(!Network<type_msg,peer_type>::_peers[i]->isByzantine()){
            Network<type_msg,peer_type>::_peers[i]->makeByzantine();
            i++;
        }else{
            i++;
        }
    }
    shuffleByzantines(_numberOfByzantines);     
}

template<class type_msg, class peer_type>
void ByzantineNetwork<type_msg,peer_type>::makeCorrect(int numberOfPeers){
    if(numberOfPeers + _numberOfCorrect > Network<type_msg,peer_type>::_peers.size()){
        _numberOfCorrect = Network<type_msg,peer_type>::_peers.size();
        _numberOfByzantines = 0;
    }else{
        _numberOfCorrect += numberOfPeers;
        _numberOfByzantines = _numberOfByzantines - numberOfPeers;
    }
    int i = 0;
    while(getCorrect().size() < _numberOfCorrect){
        if(Network<type_msg,peer_type>::_peers[i]->isByzantine()){
            Network<type_msg,peer_type>::_peers[i]->makeCorrect();
            i++;
        }else{
            i++;
        }
    }
    shuffleByzantines(_numberOfCorrect);
}

template<class type_msg, class peer_type>
void ByzantineNetwork<type_msg,peer_type>::initNetwork(int numberOfPeers){
    Network<type_msg,peer_type>::initNetwork(numberOfPeers);
    _numberOfCorrect = numberOfPeers;
    _numberOfByzantines = 0;
}

template<class type_msg, class peer_type>
void ByzantineNetwork<type_msg,peer_type>::shuffleByzantines(int shuffleCount){
    // if the network is 100% correct or Byzantines shuffling does nothing
    if(_numberOfByzantines == 0 || _numberOfCorrect == 0){
        return;
    }
    
    int shuffled = 0;
    //find list of byzantineFlag peers
    std::vector<int> byzantineIndex;
    std::vector<int> nonByzantineIndex;
    
    for(int i = 0; i < Network<type_msg,peer_type>::_peers.size(); i++){
        if(Network<type_msg,peer_type>::_peers[i]->isByzantine()){
            byzantineIndex.push_back (i);
        }else if(!Network<type_msg,peer_type>::_peers[i]->isByzantine()){
            nonByzantineIndex.push_back (i);
        }
    }
    
    // if there are no byzantine peers just return
    if(byzantineIndex.empty()){
        return;
    }
    
    // if correct to Byzantine is not a 50/50 split we need to shufle only the lower number
    if(_numberOfByzantines < shuffleCount){
        shuffleCount = _numberOfByzantines;
    }else if(_numberOfCorrect < shuffleCount){
        shuffleCount = _numberOfCorrect;
    }

    while (shuffled<shuffleCount && !byzantineIndex.empty() && !nonByzantineIndex.empty()){
        //find list of byzantineFlag peers
        int byzantineShuffleIndex = static_cast<int>(rand() % byzantineIndex.size());
        int nonByzantineShuffleIndex = static_cast<int>(rand() % nonByzantineIndex.size());
        if( Network<type_msg,peer_type>::_peers[byzantineIndex[byzantineShuffleIndex]]->makeCorrect() && Network<type_msg,peer_type>::_peers[nonByzantineIndex[nonByzantineShuffleIndex]]->makeByzantine()){
            shuffled++;
        }
        byzantineIndex.erase(byzantineIndex.begin () + byzantineShuffleIndex);
        nonByzantineIndex.erase(nonByzantineIndex.begin () + nonByzantineShuffleIndex);
    }
}

template<class type_msg, class peer_type>
void ByzantineNetwork<type_msg,peer_type>::setLog(std::ostream &out){
    _log = &out;
    Network<type_msg,peer_type>::setLog(out);
}

template<class type_msg, class peer_type>
std::ostream& ByzantineNetwork<type_msg,peer_type>::printTo(std::ostream &out)const{

    Network<type_msg,peer_type>::printTo(out);
    
    out<< "--- Byzantine Metrics --"<< std::endl;
    out<< std::left;
    out<< '\t'<< std::setw(LOG_WIDTH)<< "Number Of Byzantines"<< std::setw(LOG_WIDTH)<< "Number Of Correct"<<  std::endl;
    out<< '\t'<< std::setw(LOG_WIDTH)<< _numberOfByzantines   << std::setw(LOG_WIDTH)<< _numberOfCorrect   << std::endl;
    
    return out;
}

#endif /* ByzantineNetwork_hpp */
