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

	int StableDataLinkPeer::currentTransaction = 1;

	StableDataLinkPeer::~StableDataLinkPeer() {

	}

	StableDataLinkPeer::StableDataLinkPeer(const StableDataLinkPeer& rhs) : Peer<StableDataLinkMessage>(rhs) {
		
	}

	StableDataLinkPeer::StableDataLinkPeer(long id) : Peer(id) {
		
	}

	void StableDataLinkPeer::performComputation() {
		if (alive) {
			if (getRound() == 0 && id() == 0) {
				submitTrans(currentTransaction);
			}
			if (previousMessageRound + timeOutRate < getRound()) {// resend lost message
				if (id() == 0) {
					StableDataLinkMessage message;
					message.action = "data";
					message.roundSubmitted = getRound(); // if message lost roundSubmitted isn't accurate
					message.messageNum = currentTransaction - 1;
					previousMessageRound = getRound();
					sendMessage(1, message);
				}
				else {
					StableDataLinkMessage message;
					message.action = "ack";
					message.roundSubmitted = getRound(); // if message lost roundSubmitted isn't accurate
					message.messageNum = currentTransaction - 1;
					previousMessageRound = getRound();
					sendMessage(0, message);
				}
			}
			while (!inStreamEmpty()) {
				Packet<StableDataLinkMessage> packet = popInStream();
				long source = packet.sourceId();
				StableDataLinkMessage message = packet.getMessage();
				if (randMod(messageLossDen) < messageLossNum) { // used for message loss
					continue;
				}
				if (message.action == "ack") {
					ack++;
					if (ack < 3 * c + 2) {
						message.action = "data";
						previousMessageRound = getRound();
						sendMessage(1, message);
					}
					else {
						requestsSatisfied++;
						previousMessageRound = getRound();
						submitTrans(currentTransaction);
						ack = 0;
					}
				}
				else if (message.action == "data") {
					message.action = "ack";
					previousMessageRound = getRound();
					sendMessage(0, message);
				}
			}
		}
	}

	void StableDataLinkPeer::endOfRound(const vector<Peer<StableDataLinkMessage>*>& _peers) {
		const vector<StableDataLinkPeer*> peers = reinterpret_cast<vector<StableDataLinkPeer*> const&>(_peers);
		int satisfied = 0;
		double messages = 0;
		for (int i = 0; i < peers.size(); i++) {
			satisfied += peers[i]->requestsSatisfied;
			messages += peers[i]->messagesSent;
		}
		LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["utility"].push_back(satisfied / messages * 100);
	}

	void StableDataLinkPeer::sendMessage(long peer, StableDataLinkMessage message) {
		Packet<StableDataLinkMessage> newMessage(getRound(), peer, id());
		newMessage.setMessage(message);
		pushToOutSteam(newMessage);
		messagesSent++;
	}

	void StableDataLinkPeer::submitTrans(int tranID) {
		StableDataLinkMessage message;
		message.action = "data";
		message.roundSubmitted = getRound();
		message.messageNum = tranID;
		sendMessage(1, message);
		currentTransaction++;
	}

	std::ostream& StableDataLinkPeer::printTo(std::ostream& out)const {
		Peer<StableDataLinkMessage>::printTo(out);

		out << id() << std::endl;
		out << "counter:" << getRound() << std::endl;

		return out;
	}

	std::ostream& operator<< (std::ostream& out, const StableDataLinkPeer& peer) {
		peer.printTo(out);
		return out;
	}

	Simulation<quantas::StableDataLinkMessage, quantas::StableDataLinkPeer>* generateSim() {
        
        Simulation<quantas::StableDataLinkMessage, quantas::StableDataLinkPeer>* sim = new Simulation<quantas::StableDataLinkMessage, quantas::StableDataLinkPeer>;
        return sim;
    }
}