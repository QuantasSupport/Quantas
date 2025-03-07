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

	int AltBitPeer::currentTransaction = 1;

	AltBitPeer::~AltBitPeer() {

	}

	AltBitPeer::AltBitPeer(const AltBitPeer& rhs) : Peer<AltBitMessage>(rhs) {

	}

	AltBitPeer::AltBitPeer(interfaceId id) : Peer(id) {

	}

	void AltBitPeer::performComputation() {
		if (alive) {
			if (getRound() == 0 && id() == 0) {
				submitTrans(currentTransaction);
			}
			if (previousMessageRound + timeOutRate < getRound()) {// resend lost message
				if (id() == 0) {
					AltBitMessage message;
					message.action = "data";
					message.roundSubmitted = getRound(); // if message lost roundSubmitted isn't accurate
					message.messageNum = ns;
					previousMessageRound = getRound();
					sendMessage(1, message);
				}
				else {
					AltBitMessage message;
					message.action = "ack";
					message.roundSubmitted = getRound(); // if message lost roundSubmitted isn't accurate
					message.messageNum = ns;
					previousMessageRound = getRound();
					sendMessage(0, message);
				}
			}
			while (!inStreamEmpty()) {
				Packet<AltBitMessage> packet = popInStream();
				interfaceId source = packet.sourceId();
				AltBitMessage message = packet.getMessage();
				if (randMod(messageLossDen) < messageLossNum) { // used for message loss
					continue;
				}
				if (message.action == "ack") {
					if (message.messageNum == ns) {
						previousMessageRound = getRound();
						requestsSatisfied++;
						ns++;
						submitTrans(currentTransaction);
					}

				}
				else if (message.action == "data") {
					previousMessageRound = getRound();
					ns = message.messageNum;
					message.action = "ack";
					sendMessage(0, message);

				}
			}
		}
	}
	void AltBitPeer::endOfRound(const vector<Peer<AltBitMessage>*>& _peers) {
		const vector<AltBitPeer*> peers = reinterpret_cast<vector<AltBitPeer*> const&>(_peers);
		int satisfied = 0;
		double messages = 0;
		for (int i = 0; i < peers.size(); i++) {
			satisfied += peers[i]->requestsSatisfied;
			messages += peers[i]->messagesSent;
		}

		LogWriter::getTestLog()["utility"].push_back(satisfied / messages * 100);
	}

	void AltBitPeer::sendMessage(interfaceId peer, AltBitMessage message) {
		Packet<AltBitMessage> newMessage(getRound(), peer, id());
		newMessage.setMessage(message);
		pushToOutSteam(newMessage);
		messagesSent++;
	}

	void AltBitPeer::submitTrans(int tranID) {
		AltBitMessage message;
		message.action = "data";
		message.roundSubmitted = getRound();
		message.messageNum = ns;
		sendMessage(1, message);
		currentTransaction++;
	}
	
	Simulation<quantas::AltBitMessage, quantas::AltBitPeer>* generateSim() {
        
        Simulation<quantas::AltBitMessage, quantas::AltBitPeer>* sim = new Simulation<quantas::AltBitMessage, quantas::AltBitPeer>;
        return sim;
    }
}