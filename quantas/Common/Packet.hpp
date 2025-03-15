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
// A packet needs a source peer id (id of the network interface), target peer id (id of the network interface).


#ifndef Packet_hpp
#define Packet_hpp

#include <iostream>
#include <memory>
#include "RoundManager.hpp"
#include "LogWriter.hpp"

namespace quantas{
    
using std::string;
using std::unique_ptr;

typedef long      interfaceId;
inline static const interfaceId NO_PEER_ID = -1;  // used to indicate invalid peer or un init peers

// Base Message Class
class Message {
public:
    virtual ~Message() = default;
    virtual Message* clone() const = 0;
};

// Packet Class
class Packet {
private:
    interfaceId _targetId{NO_PEER_ID};       // Target node ID
    interfaceId _sourceId{NO_PEER_ID};       // Source node ID
    Message* _body{nullptr};   // Message payload
    int _delay{0};                  // Transmission delay
    int _round{-1};                  // Round message was sent

public:
    inline Packet();
    inline Packet(interfaceId to, interfaceId from, Message* body);
    inline Packet(const Packet& rhs);   // Deep copy constructor
    inline Packet& operator=(const Packet& rhs); // Deep copy assignment
    ~Packet() = default;

    // Setters
    inline void setSource(interfaceId s) { _sourceId = s; }
    inline void setTarget(interfaceId t) { _targetId = t; }
    inline void setDelay(int delayMax, int delayMin = 1);
    inline void setMessage(Message* msg) { _body = msg; }
    inline void deleteMessage() {delete _body;}

    // Getters
    inline interfaceId targetId() const { return _targetId; }
    inline interfaceId sourceId() const { return _sourceId; }
    inline bool hasArrived() const { return RoundManager::instance()->currentRound() >= _round + _delay; }
    inline Message* getMessage() const { return _body; }
    inline int getDelay() const { return _delay; }
    inline int getRoundSent() const { return _round; }
};

// Constructor Implementations
inline Packet::Packet() : _sourceId(NO_PEER_ID), _targetId(NO_PEER_ID), _delay(0) {
    _round = RoundManager::instance()->currentRound();
}

inline Packet::Packet(interfaceId to, interfaceId from, Message* body)
    : _targetId(to), _sourceId(from), _body(body), _delay(0) {
    _round = RoundManager::instance()->currentRound();
}

inline Packet::Packet(const Packet& rhs) 
    : _targetId(rhs._targetId), _sourceId(rhs._sourceId), _body(rhs._body), _delay(rhs._delay), _round(rhs._round) {
}

inline Packet& Packet::operator=(const Packet& rhs) {
    if (this == &rhs) return *this;
    _targetId = rhs._targetId;
    _sourceId = rhs._sourceId;
    _delay = rhs._delay;
    _round = rhs._round;
    _body = rhs._body;
    return *this;
}

inline void Packet::setDelay(int maxDelay, int minDelay) {
    if (maxDelay < 1) maxDelay = 1;
    if (minDelay < 1) minDelay = 1;
    if (minDelay > maxDelay) minDelay = maxDelay;
    // uniformInt is assumed to come from Distribution.hpp
    _delay = uniformInt(minDelay, maxDelay);
}
    
} // namespace quantas
    
#endif /* PACKET_HPP */    