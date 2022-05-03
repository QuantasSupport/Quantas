//
//  ExamplePeer.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/15/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <iostream>
#include "ExamplePeer.hpp"

namespace blockguard {

	//
	// Example Channel definitions
	//
	ExamplePeer::~ExamplePeer() {

	}

	ExamplePeer::ExamplePeer(const ExamplePeer& rhs) : Peer<ExampleMessage>(rhs) {
		_counter = rhs._counter;
	}

	ExamplePeer::ExamplePeer(long id) : Peer(id) {
		_counter = 0;
	}

	void ExamplePeer::performComputation() {
		cout << "Peer:" << id() << " performing computation" << endl;
		// Send message to self
		ExampleMessage message;
		message.message = "Message: it's me " + std::to_string(id()) + "!";
		message.aPeerId = id();
		Packet<ExampleMessage> newMessage(_counter, id(), id());
		newMessage.setMessage(message);
		pushToOutSteam(newMessage);

		// Send hello to everyone else
		message.message = "Message: " + std::to_string(_counter) + " Hello From " + std::to_string(id());
		message.aPeerId = id();
		broadcast(message);

		while (!inStreamEmpty()) {
			Packet<ExampleMessage> newMsg = popInStream();
			cout << endl << id() << " has receved a message from " << newMsg.sourceId() << endl;
			cout << "  MESSAGE " << newMsg.id() << ":" << newMsg.getMessage().message << newMsg.getMessage().aPeerId << endl;
		}
		cout << endl;
		_counter++;
	}

	void ExamplePeer::endOfRound(const vector<Peer<ExampleMessage>*>& _peers) {
		cout << "End of round " << _counter << endl;
	}

	ostream& ExamplePeer::printTo(ostream& out)const {
		Peer<ExampleMessage>::printTo(out);

		out << id() << endl;
		out << "counter:" << _counter << endl;

		return out;
	}

	ostream& operator<< (ostream& out, const ExamplePeer& peer) {
		peer.printTo(out);
		return out;
	}
}