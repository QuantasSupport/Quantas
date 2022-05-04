//
//  ExamplePeer.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/15/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef ExamplePeer_hpp
#define ExamplePeer_hpp

#include "Common/Peer.hpp"

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
    public:
        // methods that must be defined when deriving from Peer
        ExamplePeer                             (long);
        ExamplePeer                             (const ExamplePeer &rhs);
        ~ExamplePeer                            ();

        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation ();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound         (const vector<Peer<ExampleMessage>*>& _peers);

        // addintal method that have defulte implementation from Peer but can be overwritten
        void                 log()const { printTo(*_log); };
        ostream&             printTo(ostream&)const;
        friend ostream& operator<<         (ostream&, const ExamplePeer&);
    };
}
#endif /* ExamplePeer_hpp */
