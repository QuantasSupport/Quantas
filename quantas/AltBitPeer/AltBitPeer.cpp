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
		registerPeerType("AltBitPeer", 
			[](interfaceId pubId){ return new AltBitPeer(pubId); });
		return true;
	}();

	int AltBitPeer::currentTransaction = 1;

	AltBitPeer::~AltBitPeer() {

	}

	AltBitPeer::AltBitPeer(const AltBitPeer& rhs) : Peer(rhs) {

	}

	AltBitPeer::AltBitPeer(interfaceId id) : Peer(id) {

	}

	void AltBitPeer::performComputation() {
		if (alive) {
			if (RoundManager::instance()->currentRound() == 0 && publicId() == 0) {
				submitTrans(currentTransaction);
			}
			if (previousMessageRound + timeOutRate < RoundManager::instance()->currentRound()) {// resend lost message
				if (publicId() == 0) {
					AltBitMessage* message = new AltBitMessage();
					message->action = "data";
					message->roundSubmitted = RoundManager::instance()->currentRound(); // if message lost roundSubmitted isn't accurate
					message->messageNum = ns;
					previousMessageRound = RoundManager::instance()->currentRound();
					sendMessage(1, message);
				}
				else {
					AltBitMessage* message = new AltBitMessage();
					message->action = "ack";
					message->roundSubmitted = RoundManager::instance()->currentRound(); // if message lost roundSubmitted isn't accurate
					message->messageNum = ns;
					previousMessageRound = RoundManager::instance()->currentRound();
					sendMessage(0, message);
				}
			}
			while (!inStreamEmpty()) {
				Packet packet = popInStream();
				interfaceId source = packet.sourceId();
				AltBitMessage* oldMessage = dynamic_cast<AltBitMessage*>(packet.getMessage());
				if (oldMessage->action == "ack") {
					if (oldMessage->messageNum == ns) {
						previousMessageRound = RoundManager::instance()->currentRound();
						requestsSatisfied++;
						ns++;
						submitTrans(currentTransaction);
					}

				}
				else if (oldMessage->action == "data") {
					AltBitMessage* newMessage = oldMessage->clone();
					previousMessageRound = RoundManager::instance()->currentRound();
					ns = oldMessage->messageNum;
					newMessage->action = "ack";
					sendMessage(0, newMessage);
				}
				delete oldMessage; // delete recieved messages
			}
		}
	}
	void AltBitPeer::endOfRound(const vector<Peer*>& _peers) {
		if (RoundManager::instance()->lastRound() == RoundManager::instance()->currentRound()) {
			const vector<AltBitPeer*> peers = reinterpret_cast<vector<AltBitPeer*> const&>(_peers);
			int satisfied = 0;
			double messages = 0;
			for (int i = 0; i < peers.size(); i++) {
				satisfied += peers[i]->requestsSatisfied;
				messages += peers[i]->messagesSent;
			}

			LogWriter::getTestLog()["utility"].push_back(satisfied / messages * 100);
		}
	}

	void AltBitPeer::sendMessage(interfaceId peer, AltBitMessage* message) {
		unicastTo(message,peer);
		messagesSent++;
	}

	void AltBitPeer::submitTrans(int tranID) {
		AltBitMessage* message = new AltBitMessage();
		message->action = "data";
		message->roundSubmitted = RoundManager::instance()->currentRound();
		message->messageNum = ns;
		sendMessage(1, message);
		currentTransaction++;
	}
}