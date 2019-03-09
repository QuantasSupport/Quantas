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
//
//Base Message Class
//
class Message{
private:
    // message must have ID
    Message();
    
protected:
    int             _id;
    std::string     _targetId;
    std::string     _sourceId;
    
public:
    Message              (int id);
    Message              (int id, std::string to, std::string from);
    Message              (const Message&);
    ~Message             ();
    
    void setSource       (std::string s){_sourceId = s;};
    void setTarget       (std::string t){_targetId = t;};
    
    int         id       (){return _id;};
    std::string targetId (){return _targetId;};
    std::string sourceId (){return _sourceId;};
    
    Message& operator=   (const Message &rhs);
    bool operator==      (const Message &rhs);
    bool operator!=      (const Message &rhs);
    
};

//
// Basic Message used for network testing
//
class BasicMessage : Message{
protected:
    std::string       _text;
public:
    BasicMessage       (int id);
    BasicMessage       (const BasicMessage&);
    void               setText(std::string t){_text = t;};
};

#endif /* Message_hpp */
