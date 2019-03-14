//
//  Message.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "Message.hpp"
#include <string>

//
// Base message definitions
//
Message::Message(int id){
    _id = id;
    _targetId = "";
    _sourceId = "";
    _delay = -1;
}

Message::Message(int id, std::string to, std::string from){
    _id = id;
    _targetId = to;
    _sourceId = from;
    _delay = -1;
}

Message::Message(const Message& rhs){
    _id = rhs._id;
    _targetId = rhs._targetId;
    _sourceId = rhs._sourceId;
}

Message::~Message(){
    // no memory allocated so nothing to do
}

Message& Message::operator=(const Message &rhs){
    _id = rhs._id;
    _targetId = rhs._targetId;
    _sourceId = rhs._sourceId;
    return *this;
}

bool Message::operator==(const Message &rhs){
    return _id == rhs._id;
}

bool Message::operator!=(const Message &rhs){
    return !(*this == rhs);
}

//
// Basic message definitions (networking)
//

BasicMessage::BasicMessage(int id) : Message(id){
    _text = "";
}

BasicMessage::BasicMessage(const BasicMessage& rhs) : Message(rhs){
    _text = rhs._text;
}
