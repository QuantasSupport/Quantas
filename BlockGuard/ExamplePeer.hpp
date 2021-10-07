//
//  ExampleNetworkInterface.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/15/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef ExamplePeer_hpp
#define ExamplePeer_hpp

#include "./Common/Peer.hpp"

namespace blockguard{

    using std::string; 
    using std::ostream;

    //
    // Example of a message body type
    //
    struct ExampleMessage{
        
        string aPeerId;
        string message;
        
    };

    //
    // Example Peer used for network testing
    //
    class ExamplePeer : public Peer<ExampleMessage>{
    protected:
        int _counter;
        int _numberOfMessagesSent;
        
    public:
        // methods that must be defined when deriving from Peer
        ExamplePeer                             (long);
        ExamplePeer                             (const ExamplePeer &rhs);
        ~ExamplePeer                            ();
        void                 preformComputation ();
        void                 makeRequest        (){};
        
        // addintal method that have defulte implementation from Peer but can be overwritten
        void                 log                ()const{printTo(*_log);};
        ostream&             printTo            (ostream&)const;
        friend ostream&      operator<<         (ostream&, const ExamplePeer&);

        int                 getMessageCount     (){return _numberOfMessagesSent;};

        // its normally a good idea to make some getters and setters for a peer to enable testing 
        //  of the peers state durring an experment and to get metrics. 
        int                  getCounter         ()const     {return _counter;};
        void                 setCounter         (int c)     {_counter = c;};
    };
}
#endif /* ExamplePeer_hpp */
