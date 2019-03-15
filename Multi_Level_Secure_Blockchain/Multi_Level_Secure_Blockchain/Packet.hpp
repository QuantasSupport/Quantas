//
//  Message.hpp
//  Multi_Level_Secure_Blockchain
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef Packet_hpp
#define Packet_hpp

#include <stdio.h>
#include <string>
#include <ctime>

const std::string BROADCAST_TARGET = "BROADCAST";

//
//Base Message Class
//
template<class content>
class Packet{
private:
    // message must have ID
    Packet();
    
protected:
    int             _id; // message id
    std::string     _targetId; // traget node id
    std::string     _sourceId; // source node id
    
    content         _body;
    
    int             _delay; // delay of the message
    static int      _DELAY_UPPER_BOUND; // message delay upper bound
    
public:
                Packet          (int id);
                Packet          (int id, std::string to, std::string from);
                Packet          (const Packet<content>&);
                ~Packet         ();
    
    // setters
    void        setSource       (std::string s)                                 {_sourceId = s;};
    void        setTarget       (std::string t)                                 {_targetId = t;};
    void        setDelayBound   (int delay)                                     {_DELAY_UPPER_BOUND = delay;};
    void        setBody         (const content c)                               {_body = c;};
    
    // getters
    int         id              ()                                              {return _id;};
    std::string targetId        ()                                              {return _targetId;};
    std::string sourceId        ()                                              {return _sourceId;};
    bool        hasArrived      ()                                              {return !(_delay);};
    content     getMessage      ()                                              {return _body;};
    
    // mutators
    void        moveForward     ()                                              {_delay++;};
    
    //void
    
    Packet&     operator=       (const Packet<content> &rhs);
    bool        operator==      (const Packet<content> &rhs);
    bool        operator!=      (const Packet<content> &rhs);
    
};

//
// Example of a message body type
//
struct ExampleMessage{

    std::string aPeerId;
    std::string message;
    
};

#endif /* Message_hpp */
