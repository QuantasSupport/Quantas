/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "AltBitPeer.hpp"

namespace quantas {

	static bool registerAltBit = [](){
		PeerRegistry::registerPeerType("AltBitPeer", 
			[](interfaceId pubId){ return new AltBitPeer(new NetworkInterfaceAbstract(pubId)); });
		return true;
	}();

	static bool registerAltBitConcrete = [](){
		PeerRegistry::registerPeerType("AltBitPeerConcrete", 
			[](interfaceId pubId){ return new AltBitPeer(new NetworkInterfaceConcrete()); });
		return true;
	}();

	AltBitPeer::~AltBitPeer() {

	}

	AltBitPeer::AltBitPeer(const AltBitPeer& rhs) : Peer(rhs) {

	}

	AltBitPeer::AltBitPeer(NetworkInterface* networkInterface) : Peer(networkInterface) {
		if (auto networkInterface = dynamic_cast<NetworkInterfaceConcrete*>(getNetworkInterface())) {
			// Concrete simulation
			timeOutRate = 4;
		} else {
			// Abstract simulation
			timeOutRate = 4000;
		}
	}

	void AltBitPeer::performComputation() {
		if (alive) {
			if (messagesSent == 0 && publicId() == 0) {
				submitTrans(currentTransaction);
			} else if (previousMessageRound + timeOutRate < RoundManager::currentRound()) {// resend lost message
				if (publicId() == 0) {
					json message;
					message["action"] = "data";
					message["roundSubmitted"] = RoundManager::currentRound(); // if message lost roundSubmitted isn't accurate
					message["messageNum"] = ns;
					previousMessageRound = RoundManager::currentRound();
					sendMessage(1, message);
				}
				else {
					json message;
					message["action"] = "ack";
					message["roundSubmitted"] = RoundManager::currentRound(); // if message lost roundSubmitted isn't accurate
					message["messageNum"] = ns;
					previousMessageRound = RoundManager::currentRound();
					sendMessage(0, message);
				}
			}
			while (!inStreamEmpty()) {
				// std::cout << publicId() << " received a message" << std::endl;
				Packet packet = popInStream();
				interfaceId source = packet.sourceId();
				json oldMessage = packet.getMessage();
				if (oldMessage["action"] == "ack") {
					if (oldMessage["messageNum"] == ns) {
						previousMessageRound = RoundManager::currentRound();
						requestsSatisfied++;
						ns++;
						submitTrans(currentTransaction);
					}

				}
				else if (oldMessage["action"] == "data") {
					json newMessage = oldMessage;
					previousMessageRound = RoundManager::currentRound();
					ns = oldMessage["messageNum"];
					newMessage["action"] = "ack";
					sendMessage(0, newMessage);
				}
			}
		}
	}

	void AltBitPeer::endOfRound(vector<Peer*>& _peers) {
		if (RoundManager::lastRound() <= RoundManager::currentRound()) {
			const vector<AltBitPeer*> peers = reinterpret_cast<vector<AltBitPeer*> const&>(_peers);
			int satisfied = 0;
			double messages = 0;
			for (int i = 0; i < peers.size(); i++) {
				satisfied += peers[i]->requestsSatisfied;
				messages += peers[i]->messagesSent;
			}

			LogWriter::pushValue("satisfied", satisfied);
			LogWriter::pushValue("messages", messages);
		}
	}

	void AltBitPeer::sendMessage(interfaceId peer, json message) {
		unicastTo(message,peer);
		messagesSent++;
	}

	void AltBitPeer::submitTrans(int tranID) {
		json message;
		message["action"] = "data";
		message["roundSubmitted"] = RoundManager::currentRound();
		message["messageNum"] = ns;
		sendMessage(1, message);
		currentTransaction++;
	}
}