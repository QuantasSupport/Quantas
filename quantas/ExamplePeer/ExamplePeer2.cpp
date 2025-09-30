/*
Copyright 2024

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS.
If not, see <https://www.gnu.org/licenses/>.
*/

#include <string>

#include "ExamplePeer2.hpp"

namespace quantas {

static bool registerExamplePeer2 = []() {
    return PeerRegistry::registerPeerType(
        "ExamplePeer2",
        [](interfaceId pubId) { return new ExamplePeer2(new NetworkInterfaceAbstract(pubId)); });
}();

ExamplePeer2::ExamplePeer2(NetworkInterface* networkInterface)
    : Peer(networkInterface) {}

ExamplePeer2::ExamplePeer2(const ExamplePeer2& rhs)
    : Peer(rhs) {
    msgsSent = rhs.msgsSent;
}

ExamplePeer2::ExamplePeer2(ExamplePeer* rhs)
    : Peer(rhs ? rhs->releaseNetworkInterface() : nullptr) {
    if (rhs) {
        msgsSent = rhs->msgsSent;
    }
}

ExamplePeer2::~ExamplePeer2() = default;

void ExamplePeer2::initParameters(std::vector<Peer*>& peers, json parameters) {
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
    checkInStrm();

    int baseSequence = msgsSent;
    auto neighborCount = static_cast<int>(neighbors().size());

    for (int emissionIndex = 0; emissionIndex < 5; ++emissionIndex) {
        json message = buildGreetingPayload(baseSequence + emissionIndex, emissionIndex);
        broadcast(message);
        msgsSent += neighborCount;
    }
}

void ExamplePeer2::endOfRound(std::vector<Peer*>& peers) {
    logSentMessages(peers);
}

void ExamplePeer2::checkInStrm() {
    while (!inStreamEmpty()) {
        Packet packet = popInStream();
        logInboundMessage(packet);
    }
}

void ExamplePeer2::logInboundMessage(const Packet& packet) const {
    json payload = packet.getMessage();
    json logEntry;
    logEntry["to"] = publicId();
    interfaceId sender = packet.sourceId();
    if (payload.contains("from")) {
        if (payload["from"].is_number_integer()) {
            sender = payload["from"].get<interfaceId>();
        } else if (payload["from"].is_string()) {
            try {
                sender = static_cast<interfaceId>(std::stol(payload["from"].get<std::string>()));
            } catch (...) {
                // keep sender as packet source if conversion fails
            }
        }
    }
    logEntry["from"] = sender;
    logEntry["receivedRound"] = RoundManager::currentRound();
    logEntry["contents"] = payload;
    LogWriter::pushValue("receivedMessages", logEntry);
}

json ExamplePeer2::buildGreetingPayload(int sequenceNumber, int emissionIndex) const {
    json payload;
    payload["type"] = "greeting";
    payload["from"] = publicId();
    payload["roundSent"] = RoundManager::currentRound();
    payload["message"] = "Message: Hello From " + std::to_string(publicId()) +
                           ". Sent on round: " + std::to_string(RoundManager::currentRound());
    payload["sequence"] = sequenceNumber;
    payload["repeatIndex"] = emissionIndex;
    return payload;
}

void ExamplePeer2::logSentMessages(const std::vector<Peer*>& peers) const {
    int total = 0;
    for (Peer* peerPtr : peers) {
        if (auto* first = dynamic_cast<ExamplePeer*>(peerPtr)) {
            total += first->msgsSent;
        } else if (auto* second = dynamic_cast<ExamplePeer2*>(peerPtr)) {
            total += second->msgsSent;
        }
    }
    LogWriter::pushValue("sentMessages", total);
}

} // namespace quantas
