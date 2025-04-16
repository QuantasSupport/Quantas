/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "StableDataLinkPeer.hpp"

namespace quantas {

	static bool registerStableDataLink = [](){
		registerPeerType("StableDataLinkPeer", 
			[](interfaceId pubId){ return new StableDataLinkPeer(new NetworkInterfaceAbstract(pubId)); });
		return true;
	}();

	int StableDataLinkPeer::currentTransaction = 1;

	StableDataLinkPeer::~StableDataLinkPeer() {

	}

	StableDataLinkPeer::StableDataLinkPeer(const StableDataLinkPeer& rhs) : Peer<StableDataLinkMessage>(rhs) {
		
	}

	StableDataLinkPeer::StableDataLinkPeer(NetworkInterface* networkInterface) : Peer(networkInterface) {
		
	}

	void StableDataLinkPeer::performComputation() {
		if (alive) {
			if (RoundManager::currentRound() == 1 && publicId() == 0) {
				submitTrans(currentTransaction);
			}
			if (previousMessageRound + timeOutRate < RoundManager::currentRound()) {// resend lost message
				if (publicId() == 0) {
					StableDataLinkMessage message;
					message.action = "data";
					message.roundSubmitted = RoundManager::currentRound(); // if message lost roundSubmitted isn't accurate
					message.messageNum = currentTransaction - 1;
					previousMessageRound = RoundManager::currentRound();
					sendMessage(1, message);
				}
				else {
					StableDataLinkMessage message;
					message.action = "ack";
					message.roundSubmitted = RoundManager::currentRound(); // if message lost roundSubmitted isn't accurate
					message.messageNum = currentTransaction - 1;
					previousMessageRound = RoundManager::currentRound();
					sendMessage(0, message);
				}
			}
			while (!inStreamEmpty()) {
				Packet packet = popInStream();
				interfaceId source = packet.sourceId();
				StableDataLinkMessage message = packet.getMessage();
				if (randMod(messageLossDen) < messageLossNum) { // used for message loss
					continue;
				}
				if (message.action == "ack") {
					ack++;
					if (ack < 3 * c + 2) {
						message.action = "data";
						previousMessageRound = RoundManager::currentRound();
						sendMessage(1, message);
					}
					else {
						requestsSatisfied++;
						previousMessageRound = RoundManager::currentRound();
						submitTrans(currentTransaction);
						ack = 0;
					}
				}
				else if (message.action == "data") {
					message.action = "ack";
					previousMessageRound = RoundManager::currentRound();
					sendMessage(0, message);
				}
			}
		}
	}

	void StableDataLinkPeer::endOfRound(vector<Peer*>& _peers) {
		const vector<StableDataLinkPeer*> peers = reinterpret_cast<vector<StableDataLinkPeer*> const&>(_peers);
		int satisfied = 0;
		double messages = 0;
		for (int i = 0; i < peers.size(); i++) {
			satisfied += peers[i]->requestsSatisfied;
			messages += peers[i]->messagesSent;
		}
		LogWriter::pushValue("utility", satisfied / messages * 100);
	}

	void StableDataLinkPeer::sendMessage(interfaceId peer, StableDataLinkMessage message) {
		Packet newMessage(RoundManager::currentRound(), peer, publicId());
		newMessage.setMessage(message);
		pushToOutSteam(newMessage);
		messagesSent++;
	}

	void StableDataLinkPeer::submitTrans(int tranID) {
		StableDataLinkMessage message;
		message.action = "data";
		message.roundSubmitted = RoundManager::currentRound();
		message.messageNum = tranID;
		sendMessage(1, message);
		currentTransaction++;
	}
}