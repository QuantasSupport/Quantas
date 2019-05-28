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

    // logging and debugging
    std::ostream&                       printTo             (std::ostream&)const;
    void                                log                 ()const                                             {printTo(*_log);};

    // operators
    ByzantineNetwork&                   operator=           (const ByzantineNetwork<type_msg,peer_type>&);
    friend std::ostream&                operator<<          (std::ostream &out, const ByzantineNetwork &system) {return system.printTo(out);};

};

#endif /* ByzantineNetwork_hpp */

template<class type_msg, class peer_type>
ByzantineNetwork<type_msg,peer_type>::ByzantineNetwork() : Network<type_msg,peer_type>(){
    _numberOfByzantines = 0;
    _numberOfCorrect = 0;
}

template<class type_msg, class peer_type>
ByzantineNetwork<type_msg,peer_type>::ByzantineNetwork(const ByzantineNetwork &rhs) : Network<type_msg,peer_type>(rhs){
    if(this == rhs){
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
    if(this == rhs){
        return;
    }

    Network<type_msg,peer_type>::operator=(rhs);

    _numberOfByzantines = rhs._numberOfByzantines;
    _numberOfCorrect = rhs._numberOfCorrect;

    return *this;
}

template<class type_msg, class peer_type>
std::vector<peer_type*> ByzantineNetwork<type_msg,peer_type>::getByzantine()const{
    std::vector<peer_type*> byzantinePeers = std::vector<peer_type*>();
    for(int i = 0; i < _peers.size(); i++){
        if(_peers[i]->isByzantine()){
            byzantinePeers.push_back(dynamic_cast<peer_type*>(_peers[i]));
        }
    }
    assert(byzantinePeers.size() == _numberOfByzantines);
    return byzantinePeers;
}

template<class type_msg, class peer_type>
std::vector<peer_type*> ByzantineNetwork<type_msg,peer_type>::getCorrect()const{
    std::vector<peer_type*> correctPeers = std::vector<peer_type*>();
    for(int i = 0; i < _peers.size(); i++){
        if(!_peers[i]->isByzantine()){
            correctPeers.push_back(dynamic_cast<peer_type*>(_peers[i]));
        }
    }
    assert(correctPeers.size() == _numberOfCorrect)
    return correctPeers;   
}

template<class type_msg, class peer_type>
void ByzantineNetwork<type_msg,peer_type>::makeByzantines(int numberOfPeers){
    if(numberOfPeers > _peers.size()){
        _numberOfByzantines = _peers.size();
    }else{
        _numberOfByzantines = numberOfPeers;
    }
    int i = 0;
    while(getByzantine().size() < _numberOfByzantines){
        if(!_peers[i]->isByzantine()){
            _peers[i]->makeByzantine();
            i++;
        }else{
            i++;
        }
    }
    shuffleByzantines(numberOfPeers);     
}

template<class type_msg, class peer_type>
void ByzantineNetwork<type_msg,peer_type>::makeCorrect(int numberOfPeers){
    if(numberOfPeers > _peers.size()){
        _numberOfCorrect = _peers.size();
    }else{
        _numberOfCorrect = numberOfPeers;
    }
    int numberOfCorrectAdded = 0;
    int i = 0;
    while(getCorrect().size() < _numberOfCorrect){
        if(_peers[i]->isByzantine()){
            _peers[i]->makeCorrect();
            i++;
        }else{
            i++;
        }
    }
    shuffleByzantines(numberOfPeers);    
}

template<class type_msg, class peer_type>
void ByzantineNetwork<type_msg,peer_type>::shuffleByzantines(int shuffleCount){
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

    // if there are no byzantine peers just return
    if(byzantineIndex.empty()){
        return;
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