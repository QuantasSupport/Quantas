//
//  Message.hpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef Message_hpp
#define Message_hpp

#include <stdio.h>
#include <string>

class Message{
protected:
    std::string     targetId;
    std::string     sourceId;
    
public:
    Message           ();
    Message           (std::string to, std::string from);
    Message           (const Message&);
    ~Message          ();
    
    Message operator= (const Message &rhs);
    bool operator==   (const Message &rhs);
    bool operator!=   (const Message &rhs);
    
};

#endif /* Message_hpp */
