/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include "ExamplePeer.hpp"

namespace quantas {

	static bool registerExamplePeer = [](){
		registerPeerType("ExamplePeer", 
			[](interfaceId pubId){ return new ExamplePeer(pubId); });
		return true;
	}();

	//
	// Example Channel definitions
	//
	ExamplePeer::~ExamplePeer() {

	}

	ExamplePeer::ExamplePeer(const ExamplePeer& rhs) : Peer(rhs) {
		
	}

	ExamplePeer::ExamplePeer(interfaceId id) : Peer(id) {
		
	}

	void ExamplePeer::initParameters(const vector<Peer*>& _peers, json parameters) {
		const vector<ExamplePeer*> peers = reinterpret_cast<vector<ExamplePeer*> const&>(_peers);
		
		cout << "Initializing parameters of simulation" << endl;

		if (parameters.contains("parameter1")) {
			cout << "parameter1: " << parameters["parameter1"] << endl;
		}

		if (parameters.contains("parameter2")) {
			cout << "parameter2: " << parameters["parameter2"] << endl;
		}

		if (parameters.contains("parameter3")) {
			cout << "parameter3: " << parameters["parameter3"] << endl;
		}
	}

	void ExamplePeer::performComputation() {

		cout << "Peer:" << publicId() << " performing computation" << endl;

		// Read messages from other peers
		while (!inStreamEmpty()) {
			ExampleMessage* newMsg = dynamic_cast<ExampleMessage*>(popInStream().getMessage());
			cout << endl << std::to_string(publicId()) << " has receved a message from " << newMsg->aPeerId << endl;
			cout << newMsg->message << endl;
		}
		cout << endl;

		// Send hello to everyone else
		ExampleMessage* msg = new ExampleMessage();
		msg->message = "Message: Hello From " + std::to_string(publicId()) + ". Sent on round: " + std::to_string(RoundManager::instance()->currentRound());
		msg->aPeerId = std::to_string(publicId());
		broadcast(msg);
	}

	void ExamplePeer::endOfRound(const vector<Peer*>& _peers) {
		const vector<ExamplePeer*> peers = reinterpret_cast<vector<ExamplePeer*> const&>(_peers);
		cout << "End of round " << RoundManager::instance()->currentRound() << endl;
	}
}