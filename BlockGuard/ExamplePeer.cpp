//
//  ExamplePeer.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/15/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "ExamplePeer.hpp"
#include "./Common/Peer.hpp"
#include "./Common/Packet.hpp"
#include <iostream>

namespace blockguard {

    using std::cout; 
    using std::to_string; 
    using std::ostream;
    using std::string;
    using std::endl;

    //
    // Example Channel definitions
    //
    ExamplePeer::~ExamplePeer(){
        
    }

    ExamplePeer::ExamplePeer(const ExamplePeer &rhs) : Peer<ExampleMessage>(rhs){
        _counter = rhs._counter;
        _numberOfMessagesSent = rhs._numberOfMessagesSent;
    }

    ExamplePeer::ExamplePeer(long id) : Peer(id){
        _counter =0;
        _numberOfMessagesSent =0;
    }

    void ExamplePeer::performComputation(){
        cout<< "Peer:"<< id() << " performing computation"<<endl;

        // Send message to self
        ExampleMessage message;
        message.message = "Message: it's me " + to_string(id()) + "!";
        message.aPeerId = id();
        Packet<ExampleMessage> newMessage(_counter, id(),id());
        newMessage.setBody(message);
        pushToOutSteam(newMessage);

        // Send hello to everyone else
        message.message = "Message: " + to_string(_counter) + " Hello From " + to_string(id());
        message.aPeerId = id();
        broadcast(message);

        for(int i = 0; i < inStreamSize(); i++){
            Packet<ExampleMessage> newMsg = popInStream();
            cout << endl << id()<< " has receved a message from "<< newMsg.sourceId()<< endl;
            cout << "  MESSAGE "<< newMsg.id() <<  ":"<< newMsg.getMessage().message<<  newMsg.getMessage().aPeerId<< endl;
        }
        cout << endl;
        _counter++;
    }

    ostream& ExamplePeer::printTo(ostream &out)const{
        Peer<ExampleMessage>::printTo(out);
        
        out<< id()<< endl;
        out<< "counter:"<< _counter<< endl;
        
        return out;
    }

    ostream& operator<< (ostream &out, const ExamplePeer &peer){
        peer.printTo(out);
        return out;
    }
}