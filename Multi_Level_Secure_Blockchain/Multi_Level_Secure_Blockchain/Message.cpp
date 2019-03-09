//
//  Message.cpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "Message.hpp"
#include <string>

Message::Message(){
    targetId = "NOT SET";
    sourceId = "NOT SET";
}

Message::Message(std::string to, std::string from){
    targetId = to;
    sourceId = from;
}

Message::Message(const Message& rhs){
    targetId = rhs.targetId;
    sourceId = rhs.sourceId;
}

Message::~Message(){
    // no memory allocated so nothing to do
}

Message Message::operator=(const Message &rhs){
    return Message(rhs);
}

bool Message::operator==(const Message &rhs){
    return targetId == rhs.targetId && sourceId == rhs.sourceId;
}

bool Message::operator!=(const Message &rhs){
    return !(*this == rhs);
}
