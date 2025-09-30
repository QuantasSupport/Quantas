/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include "StableDataLinkPeer.hpp"

namespace quantas {

static bool registerStableDataLink = []() {
	return PeerRegistry::registerPeerType(
		"StableDataLinkPeer",
		[](interfaceId pubId) { return new StableDataLinkPeer(new NetworkInterfaceAbstract(pubId)); });
}();

static bool registerStableDataLinkConcrete = []() {
	return PeerRegistry::registerPeerType(
		"StableDataLinkPeerConcrete",
		[](interfaceId) { return new StableDataLinkPeer(new NetworkInterfaceConcrete()); });
}();

StableDataLinkPeer::~StableDataLinkPeer() = default;

StableDataLinkPeer::StableDataLinkPeer(const StableDataLinkPeer& rhs) : Peer(rhs) {}

StableDataLinkPeer::StableDataLinkPeer(NetworkInterface* networkInterface)
	: Peer(networkInterface) {
	timeOutRate = 4;
}

void StableDataLinkPeer::performComputation() {
	const size_t currentRound = RoundManager::currentRound();

	if (publicId() == 0 && !awaitingAck && nextMessageNum == 1) {
		submitTrans();
	}

	while (!inStreamEmpty()) {
		Packet packet = popInStream();
		json message = packet.getMessage();
		const std::string action = message.value("action", "");
		const int messageNum = message.value("messageNum", -1);

		if (action == "ack") {
			handleAck(messageNum);
		} else if (action == "data") {
			handleData(messageNum);
		}
	}

	if (publicId() == 0 && awaitingAck && previousMessageRound + timeOutRate < currentRound) {
		resendData();
	}
}

void StableDataLinkPeer::endOfRound(vector<Peer*>& _peers) {
	const vector<StableDataLinkPeer*>& peers = reinterpret_cast<vector<StableDataLinkPeer*> const&>(_peers);
	double totalSatisfied = 0.0;
	double totalMessages = 0.0;
	for (const auto* peer : peers) {
		totalSatisfied += static_cast<double>(peer->requestsSatisfied);
		totalMessages += static_cast<double>(peer->messagesSent);
	}

	const double utility = totalMessages > 0.0 ? (totalSatisfied / totalMessages) * 100.0 : 0.0;
	const double throughput = peers.empty() ? 0.0 : totalSatisfied / peers.size();

	LogWriter::pushValue("utility", utility);
	LogWriter::pushValue("messages", totalMessages);
	LogWriter::pushValue("throughput", throughput);
}

void StableDataLinkPeer::handleAck(int messageNum) {
	if (publicId() != 0) return;
	if (!awaitingAck) return;
	if (messageNum != lastSentMessageNum) return;

	previousMessageRound = RoundManager::currentRound();
	awaitingAck = false;
	requestsSatisfied++;
	submitTrans();
}

void StableDataLinkPeer::handleData(int messageNum) {
	if (messageNum < 0) return;
	previousMessageRound = RoundManager::currentRound();
	if (messageNum > lastDeliveredMessageNum) {
		lastDeliveredMessageNum = messageNum;
	}
	sendAck(messageNum);
}

void StableDataLinkPeer::sendMessage(interfaceId peer, const json& message) {
	unicastTo(message, peer);
	messagesSent++;
	previousMessageRound = RoundManager::currentRound();
}

void StableDataLinkPeer::submitTrans() {
	if (publicId() != 0) return;
	lastSentMessageNum = nextMessageNum;
	awaitingAck = true;
	sendData(nextMessageNum);
	nextMessageNum++;
}

void StableDataLinkPeer::sendData(int messageNum) {
	json message;
	message["action"] = "data";
	message["roundSubmitted"] = RoundManager::currentRound();
	message["messageNum"] = messageNum;
	sendMessage(partnerId(), message);
}

void StableDataLinkPeer::resendData() {
	if (!awaitingAck) return;
	if (lastSentMessageNum <= 0) return;
	sendData(lastSentMessageNum);
}

void StableDataLinkPeer::sendAck(int messageNum) {
	if (messageNum < 0) return;
	json message;
	message["action"] = "ack";
	message["roundSubmitted"] = RoundManager::currentRound();
	message["messageNum"] = messageNum;
	sendMessage(partnerId(), message);
}

interfaceId StableDataLinkPeer::partnerId() const {
	const auto nbrs = neighbors();
	if (!nbrs.empty()) {
		return *nbrs.begin();
	}
	return (publicId() == 0) ? 1 : 0;
}

}
