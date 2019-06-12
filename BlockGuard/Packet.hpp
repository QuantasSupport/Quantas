//
//  Message.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef Packet_hpp
#define Packet_hpp

#include <stdio.h>
#include <string>
#include <ctime>
#include <random>

//
//Base Message Class
//

static std::default_random_engine RANDOM_GENERATOR = std::default_random_engine((int)time(nullptr));

template<class content>
class Packet{
private:
    // message must have ID
    Packet(){};
    
protected:
    std::string                 _id; // message id
    std::string                 _targetId; // traget node id
    std::string                 _sourceId; // source node id
    
    content                     _body;
    
    int                         _delay; // delay of the message
    
public:
    Packet                      (std::string id);
    Packet                      (std::string id, std::string to, std::string from);
    Packet                      (const Packet<content>&);
    ~Packet                     ();
    
    // setters
    void        setSource       (std::string s){_sourceId = s;};
    void        setTarget       (std::string t){_targetId = t;};
    void        setDelay        (int delayMax, int delayMin = 0);
    void        setBody         (const content c){_body = c;};
    
    // getters
    std::string id              (){return _id;};
    std::string targetId        (){return _targetId;};
    std::string sourceId        (){return _sourceId;};
    bool        hasArrived      (){return !(bool)(_delay);};
    content     getMessage      (){return _body;};
    int         getDelay        (){return _delay;};
    
    // mutators
    void        moveForward     (){_delay--;};
    
    //void
    
    Packet&     operator=       (const Packet<content> &rhs);
    bool        operator==      (const Packet<content> &rhs);
    bool        operator!=      (const Packet<content> &rhs);
    
};

template<class content>
Packet<content>::Packet(std::string id){
    _id = id;
    _sourceId = "";
    _targetId = "";
    _body = content();
    _delay = 0;
}

template<class content>
Packet<content>::Packet(std::string id, std::string to ,std::string from){
    _id = id;
    _sourceId = from;
    _targetId = to;
    _body = content();
    _delay = 0;
}

template<class content>
Packet<content>::Packet(const Packet<content>& rhs){
    _id = rhs._id;
    _targetId = rhs._targetId;
    _sourceId = rhs._sourceId;
    _body = rhs._body;
    _delay = rhs._delay;
}

template<class content>
Packet<content>::~Packet(){
    // no memory allocated so nothing to do
}

template <class content>
void Packet<content>::setDelay(int maxDelay, int minDelay){
    std::uniform_int_distribution<int> uniformDist(minDelay,(maxDelay - 1));
    _delay = uniformDist(RANDOM_GENERATOR); // max is not included so delay 1 is next round delay 2 is one round waiting and then receve in the following round
}

template<class content>
Packet<content>& Packet<content>::operator=(const Packet<content> &rhs){
    _id = rhs._id;
    _targetId = rhs._targetId;
    _sourceId = rhs._sourceId;
    _body = rhs._body;
    _delay = rhs._delay;
    return *this;
}

template<class content>
bool Packet<content>::operator==(const Packet<content> &rhs){
    return _id == rhs._id;
}

template<class content>
bool Packet<content>::operator!=(const Packet<content> &rhs){
    return !(*this == rhs);
}

#endif /* Message_hpp */
