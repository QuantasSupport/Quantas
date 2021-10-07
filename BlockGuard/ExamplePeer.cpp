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

    void ExamplePeer::preformComputation(){
        cout<< "Peer:"<< _id << " preforming computation"<<endl;
        
        ExampleMessage message;
        message.message = "Message: it's me " + to_string(_id) + "!";
        message.aPeerId = _id;
        Packet<ExampleMessage> newMessage(_counter, _id,_id);
        newMessage.setBody(message);
        _outStream.push_back(newMessage);
        
        for (auto it = _neighbors.begin(); it != _neighbors.end(); it++ )
        {
            ExampleMessage message;
            message.message = "Message: " + to_string(_counter)  + " Hello From " + to_string(_id);
            message.aPeerId = _id;
            cout << "SENDING TO " << it->first << endl;
            Packet<ExampleMessage> newMessage(_counter, it->first,_id);
            newMessage.setBody(message);
            _outStream.push_back(newMessage);
        }
        
        for(int i = 0; i < _inStream.size(); i++){
            cout << endl << _id<< " has receved a message from "<< _inStream[i].sourceId()<< endl;
            cout << "  MESSAGE "<< _inStream[i].id() <<  ":"<< _inStream[i].getMessage().message<<  _inStream[i].getMessage().aPeerId<< endl;
        }
        cout << endl;
        _inStream.clear();
        _counter++;
    }

    ostream& ExamplePeer::printTo(ostream &out)const{
        Peer<ExampleMessage>::printTo(out);
        
        out<< _id<< endl;
        out<< "counter:"<< _counter<< endl;
        
        return out;
    }

    ostream& operator<< (ostream &out, const ExamplePeer &peer){
        peer.printTo(out);
        return out;
    }
}