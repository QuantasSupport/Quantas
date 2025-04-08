/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include "ExamplePeer2.hpp"

namespace quantas {

	static bool registerExamplePeer2 = [](){
		PeerRegistry::registerPeerType("ExamplePeer2", 
			[](interfaceId pubId){ return new ExamplePeer2(pubId); });
		return true;
	}();

	//
	// Example Channel definitions
	//
	ExamplePeer2::~ExamplePeer2() {

	}

	ExamplePeer2::ExamplePeer2(const ExamplePeer2& rhs) : Peer(rhs) {
		
	}

	ExamplePeer2::ExamplePeer2(interfaceId id) : Peer(id) {
		
	}

	void ExamplePeer2::initParameters(vector<Peer*>& _peers, json parameters) {
		const vector<ExamplePeer2*> peers = reinterpret_cast<vector<ExamplePeer2*> const&>(_peers);
		
		// Initializing parameters of simulation
		if (parameters.contains("parameter1")) {
			LogWriter::pushValue("parameter1", parameters["parameter1"]);
		}

		if (parameters.contains("parameter2")) {
			LogWriter::pushValue("parameter2", parameters["parameter2"]);
		}

		if (parameters.contains("parameter3")) {
			LogWriter::pushValue("parameter3", parameters["parameter3"]);
		}
	}

	void ExamplePeer2::performComputation() {
		LogWriter::pushValue("performs computation", publicId());

		// Read messages from other peers
		while (!inStreamEmpty()) {
			ExampleMessage* newMsg = dynamic_cast<ExampleMessage*>(popInStream().getMessage());
			json msg;
			msg["to"] = publicId();
			msg["from"] = newMsg->aPeerId;
			msg["receivedRound"] = RoundManager::currentRound();
			msg["contents"] = newMsg->message;
			LogWriter::pushValue("receivedMessages", msg);
			delete newMsg;
		}

		// Send hello to everyone 5 times
        for (int i = 0; i < 5; ++i) {
            ExampleMessage* msg = new ExampleMessage();
            msg->message = "Message: Hello From " + std::to_string(publicId()) + ". Sent on round: " + std::to_string(RoundManager::currentRound());
            msg->aPeerId = std::to_string(publicId());
			broadcast(msg);
            msgsSent += neighbors().size();
        }
	}

	void ExamplePeer2::endOfRound(vector<Peer*>& _peers) {
		const vector<ExamplePeer2*> peers = reinterpret_cast<vector<ExamplePeer2*> const&>(_peers);
		int sum = accumulate(peers.begin(), peers.end(), 0, [](int sum, auto p) {
			return sum + p->msgsSent;
		});
		LogWriter::pushValue("sentMessages", sum);
	}
}