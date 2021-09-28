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
    // Example Peer definitions
    //
    ExamplePeer::~ExamplePeer(){
        
    }

    ExamplePeer::ExamplePeer(const ExamplePeer &rhs) : Peer<ExampleMessage>(rhs){
        counter = rhs.counter;
    }

    ExamplePeer::ExamplePeer(string id) : Peer(id){
        counter =0;
    }

    void ExamplePeer::preformComputation(){
        cout<< "Peer:"<< _id<< " preforming computation"<<endl;
        
        ExampleMessage message;
        message.message = "Message: " + to_string(counter)  + " Hello From ";
        message.aPeerId = _id;
        Packet<ExampleMessage> newMessage(to_string(counter), _id,_id);
        newMessage.setBody(message);
        _outStream.push_back(newMessage);
        
        for (auto it = _neighbors.begin(); it != _neighbors.end(); it++ )
        {
            ExampleMessage message;
            message.message = "Message: " + to_string(counter)  + " Hello From ";
            message.aPeerId = _id;
            Packet<ExampleMessage> newMessage(to_string(counter), it->first,_id);
            newMessage.setBody(message);
            _outStream.push_back(newMessage);
        }
        
        for(int i = 0; i < _inStream.size(); i++){
            cout << endl << _id<< " has receved a message from "<< _inStream[i].sourceId()<< endl;
            cout << "  MESSAGE "<< _inStream[i].id() <<  ":"<< _inStream[i].getMessage().message<<  _inStream[i].getMessage().aPeerId<< endl;
        }
        cout << endl;
        _inStream.clear();
        counter++;
    }

    ostream& ExamplePeer::printTo(ostream &out)const{
        Peer<ExampleMessage>::printTo(out);
        
        out<< _id<< endl;
        out<< "counter:"<< counter<< endl;
        
        return out;
    }

    ostream& operator<< (ostream &out, const ExamplePeer &peer){
        peer.printTo(out);
        return out;
    }
}