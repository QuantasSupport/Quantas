/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include "ExamplePeer.hpp"
#include "ExamplePeer2.hpp"

namespace quantas {

	static bool registerExamplePeer = [](){
		PeerRegistry::registerPeerType("ExamplePeer", 
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

	void ExamplePeer::initParameters(vector<Peer*>& _peers, json parameters) {
		const vector<ExamplePeer*> peers = reinterpret_cast<vector<ExamplePeer*> const&>(_peers);
		
		// Initializing parameters of simulation
		if (parameters.contains("parameter1")) {
			LogWriter::pushValue("parameter1", parameters["parameter1"]);
		}

		if (parameters.contains("parameter2")) {
			LogWriter::pushValue("parameter2", parameters["parameter2"]);
		}

		if (parameters.contains("changePeerType")) {
			LogWriter::pushValue("changePeerType", parameters["changePeerType"]);
			bool changePeerType = true;
		}
	}

	void ExamplePeer::performComputation() {
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

		// Send hello to everyone
		ExampleMessage* msg = new ExampleMessage();
		msg->message = "Message: Hello From " + std::to_string(publicId()) + ". Sent on round: " + std::to_string(RoundManager::currentRound());
		msg->aPeerId = std::to_string(publicId());
		broadcast(msg);
		msgsSent += neighbors().size();
	}

	void ExamplePeer::endOfRound(vector<Peer*>& _peers) {
		vector<ExamplePeer*> peers = reinterpret_cast<vector<ExamplePeer*>&>(_peers);
		int sum = accumulate(peers.begin(), peers.end(), 0, [](int sum, auto p) {
			return sum + p->msgsSent;
		});
		LogWriter::pushValue("sentMessages", sum);

		if (RoundManager::currentRound() == 2) {
			auto oldPeer = dynamic_cast<ExamplePeer*>(_peers[1]);
			if (oldPeer) {
				_peers[1] = new ExamplePeer2(oldPeer);
				delete oldPeer;
			}
		}
	}
}