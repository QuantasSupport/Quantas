/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/
//
// This is the class responsible for holding messages as they are sent from one peer to another (similar to a packet in a real network)
// It is templated so that the user can define their own message (body, payload, etc.) they want to include in the packet. This allows
// them to define what information is allowed in a single message between peers.
//
// A packet needs a source peer id (id of the network interface), target peer id (id of the network interface) and an id.
// The Id of the packet is used for comparison of two packets. Structs can not be compared unless the user defines the equal to and not
// equal operator. As such we do not expect or assume that the user does so. We define a packet ID to overcome this two packets with the
// same id are regarded as equal.


#ifndef Packet_hpp
#define Packet_hpp

#include <stdio.h>
#include <string>
#include <ctime>
#include <random>
#include "LogWriter.hpp"
#include "Distribution.hpp"

namespace quantas{
    
    using std::string;
    
    static const long NO_PEER_ID = -1;  // number used to indicate invalid peer id or un init peer id

    //
    //Base Message Class
    //

    template<class message>
    class Packet{
    private:
        // message must have ID
        Packet(){};
        
    protected:
        long                        _id; // message id 
        long                        _targetId; // target node id
        long                        _sourceId; // source node id
        
        message                     _body;
        
        int                         _delay; // delay of the message
        int                         _round; // round message was sent
        
    public:
        Packet                      (long id);
        Packet                      (long id, long to, long from);
        Packet                      (const Packet<message>&);
        ~Packet                     ();
        
        // setters
        void        setSource       (long s){_sourceId = s;};
        void        setTarget       (long t){_targetId = t;};
        void        setDelay        (int delayMax, int delayMin = 1);
        void        setMessage      (const message c){_body = c;};
        
        // getters
        long        id              ()const {return _id;};
        long        targetId        ()const {return _targetId;};
        long        sourceId        ()const {return _sourceId;};
        bool        hasArrived      ()const {return LogWriter::instance()->getRound() >= _round + _delay;};
        message     getMessage      ()const {return _body;};
        int         getDelay        ()const {return _delay;};
        int         getRound        ()const {return _round;};
        
        // mutators
        //void        moveForward     (){_delay = _delay > 0 ? _delay-1 : 0;};
        
        //void
        
        Packet&     operator=       (const Packet<message> &rhs);
        bool        operator==      (const Packet<message> &rhs) const;
        bool        operator!=      (const Packet<message> &rhs) const;
        
    };

    template<class message>
    Packet<message>::Packet(long id){
        _id = id;
        _sourceId = NO_PEER_ID;
        _targetId = NO_PEER_ID;
        _body = message();
        _delay = 0;
        _round = LogWriter::instance()->getRound();
    }

    template<class message>
    Packet<message>::Packet(long id, long to ,long from){
        _id = id;
        _sourceId = from;
        _targetId = to;
        _body = message();
        _delay = 0;
        _round = LogWriter::instance()->getRound();
    }

    template<class message>
    Packet<message>::Packet(const Packet<message>& rhs){
        _id = rhs._id;
        _targetId = rhs._targetId;
        _sourceId = rhs._sourceId;
        _body = rhs._body;
        _delay = rhs._delay;
        _round = rhs._round;
    }

    template<class message>
    Packet<message>::~Packet(){
        // no memory allocated so nothing to do
    }

    template <class message>
    void Packet<message>::setDelay(int maxDelay, int minDelay){
        // max is not included so delay 1 is next round delay 2 is one round
        // waiting and then receive in the following round
        _delay = uniformInt(minDelay, maxDelay);
    }

    template<class message>
    Packet<message>& Packet<message>::operator=(const Packet<message> &rhs){
        _id = rhs._id;
        _targetId = rhs._targetId;
        _sourceId = rhs._sourceId;
        _body = rhs._body;
        _delay = rhs._delay;
        _round = rhs._round;
        return *this;
    }

    template<class message>
    bool Packet<message>::operator== (const Packet<message> &rhs)const{
        return _id == rhs._id;
    }

    template<class message>
    bool Packet<message>::operator!= (const Packet<message> &rhs)const{
        return !(_id == rhs._id);
    }
}
#endif /* Message_hpp */
