//
//  ExamplePeer.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/15/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef ExamplePeer_hpp
#define ExamplePeer_hpp

#include "Peer.hpp"

//
// Example of a message body type
//
struct ExampleMessage{
    
    std::string aPeerId;
    std::string message;
    
};

//
// Example Peer used for network testing
//
class ExamplePeer : public Peer<ExampleMessage>{
protected:
    int counter;
public:
    // methods that must be defined when deriving from Peer
    ExamplePeer                             (std::string);
    ExamplePeer                             (const ExamplePeer &rhs);
    ~ExamplePeer                            ();
    void                 preformComputation ();
    void                 makeRequest        (){};
    void                 log                ()const{printTo(*_log);};
    std::ostream&        printTo            (std::ostream&)const;
    friend std::ostream& operator<<         (std::ostream&, const ExamplePeer&);

    // its normally a good idea to make some getters and setters for a peer to enable testing 
    //  of the peers state durring an experment and to get metrics. 
    int                  getCounter()const                                      {return counter;};
    void                 setCounter(int c)                                      {counter = c;};
};
#endif /* ExamplePeer_hpp */
