/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include "ExamplePeer.hpp"

namespace quantas {

	//
	// Example Channel definitions
	//
	ExamplePeer::~ExamplePeer() {

	}

	ExamplePeer::ExamplePeer(const ExamplePeer& rhs) : Peer<ExampleMessage>(rhs) {
		
	}

	ExamplePeer::ExamplePeer(long id) : Peer(id) {
		
	}

	void ExamplePeer::performComputation() {
		cout << "Peer:" << id() << " performing computation" << endl;
		// Send message to self
		ExampleMessage msg;
		msg.message = "Message: it's me " + std::to_string(id()) + "!";
		msg.aPeerId = std::to_string(id());
		Packet<ExampleMessage> newMsg(getRound(), id(), id());
		newMsg.setMessage(msg);
		pushToOutSteam(newMsg);
		pushToOutSteam(newMsg);

		// Send hello to everyone else
		msg.message = "Message: Hello From " + std::to_string(id()) + ". Sent on round: " + std::to_string(getRound());
		msg.aPeerId = std::to_string(id());
		broadcast(msg);
		broadcast(msg);
		broadcast(msg);

		while (!inStreamEmpty()) {
			Packet<ExampleMessage> newMsg = popInStream();
			cout << endl << std::to_string(id()) << " has receved a message from " << newMsg.getMessage().aPeerId << endl;
			cout << newMsg.getMessage().message << endl;
		}
		cout << endl;
	}

	void ExamplePeer::endOfRound(const vector<Peer<ExampleMessage>*>& _peers) {
		cout << "End of round " << getRound() << endl;
	}

	ostream& ExamplePeer::printTo(ostream& out)const {
		Peer<ExampleMessage>::printTo(out);

		out << id() << endl;
		out << "counter:" << getRound() << endl;

		return out;
	}

	ostream& operator<< (ostream& out, const ExamplePeer& peer) {
		peer.printTo(out);
		return out;
	}

	Simulation<quantas::ExampleMessage, quantas::ExamplePeer>* generateSim() {
        
        Simulation<quantas::ExampleMessage, quantas::ExamplePeer>* sim = new Simulation<quantas::ExampleMessage, quantas::ExamplePeer>;
        return sim;
    }
}