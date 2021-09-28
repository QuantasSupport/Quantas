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

namespace blockguard{

    using std::default_random_engine;
    using std::string;
    using std::uniform_int_distribution;

    //
    //Base Message Class
    //

    static default_random_engine RANDOM_GENERATOR = default_random_engine((int)time(nullptr));

    template<class content>
    class Packet{
    private:
        // message must have ID
        Packet(){};
        
    protected:
        string                      _id; // message id
        string                      _targetId; // traget node id
        string                      _sourceId; // source node id
        
        content                     _body;
        
        int                         _delay; // delay of the message
        
    public:
        Packet                      (string id);
        Packet                      (string id, string to, string from);
        Packet                      (const Packet<content>&);
        ~Packet                     ();
        
        // setters
        void        setSource       (string s){_sourceId = s;};
        void        setTarget       (string t){_targetId = t;};
        void        setDelay        (int delayMax, int delayMin = 1);
        void        setBody         (const content c){_body = c;};
        
        // getters
        string      id              ()const {return _id;};
        string      targetId        ()const {return _targetId;};
        string      sourceId        ()const {return _sourceId;};
        bool        hasArrived      ()const {return !(bool)(_delay);};
        content     getMessage      ()const {return _body;};
        int         getDelay        ()const {return _delay;};
        
        // mutators
        void        moveForward     (){_delay = _delay > 0 ? _delay-1 : 0;};
        
        //void
        
        Packet&     operator=       (const Packet<content> &rhs);
        bool        operator==      (const Packet<content> &rhs) const;
        bool        operator!=      (const Packet<content> &rhs) const;
        
    };

    template<class content>
    Packet<content>::Packet(string id){
        _id = id;
        _sourceId = "";
        _targetId = "";
        _body = content();
        _delay = 0;
    }

    template<class content>
    Packet<content>::Packet(string id, string to ,string from){
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
        uniform_int_distribution<int> uniformDist(minDelay, maxDelay);
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
    bool Packet<content>::operator== (const Packet<content> &rhs)const{
        return _id == rhs._id;
    }

    template<class content>
    bool Packet<content>::operator!= (const Packet<content> &rhs)const{
        return !(_id == rhs._id);
    }
}
#endif /* Message_hpp */
