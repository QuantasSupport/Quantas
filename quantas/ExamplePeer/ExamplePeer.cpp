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

#include "ExamplePeer.hpp"
#include "ExamplePeer2.hpp"

namespace quantas {

static bool registerExamplePeer = []() {
    return PeerRegistry::registerPeerType(
        "ExamplePeer",
        [](interfaceId pubId) { return new ExamplePeer(new NetworkInterfaceAbstract(pubId)); });
}();

ExamplePeer::ExamplePeer(NetworkInterface* networkInterface)
    : Peer(networkInterface) {}

ExamplePeer::ExamplePeer(const ExamplePeer& rhs)
    : Peer(rhs) {
    msgsSent = rhs.msgsSent;
    changePeerType = rhs.changePeerType;
}

ExamplePeer::~ExamplePeer() = default;

void ExamplePeer::initParameters(std::vector<Peer*>& peers, json parameters) {
    if (parameters.contains("parameter1")) {
        LogWriter::pushValue("parameter1", parameters["parameter1"]);
    }

    if (parameters.contains("parameter2")) {
        LogWriter::pushValue("parameter2", parameters["parameter2"]);
    }

    bool shouldChangeType = parameters.value("changePeerType", changePeerType);
    LogWriter::pushValue("changePeerType", shouldChangeType);

    for (Peer* peerPtr : peers) {
        if (auto* example = dynamic_cast<ExamplePeer*>(peerPtr)) {
            example->changePeerType = shouldChangeType;
        }
    }
}

void ExamplePeer::performComputation() {
    LogWriter::pushValue("performs computation", publicId());
    checkInStrm();

    json message = buildGreetingPayload();
    broadcast(message);
    msgsSent += static_cast<int>(neighbors().size());
}

void ExamplePeer::endOfRound(std::vector<Peer*>& peers) {
    logSentMessages(peers);

    if (changePeerType && peers.size() > 1) {
        if (auto* oldPeer = dynamic_cast<ExamplePeer*>(peers[1])) {
            ExamplePeer2* replacement = new ExamplePeer2(oldPeer);
            peers[1] = replacement;
            delete oldPeer;
        }
    }

    for (Peer* peerPtr : peers) {
        if (auto* example = dynamic_cast<ExamplePeer*>(peerPtr)) {
            example->changePeerType = false;
        }
    }

    if (RoundManager::lastRound() <= RoundManager::currentRound()) {
        int total = 0;
        for (Peer* peerPtr : peers) {
            if (auto* first = dynamic_cast<ExamplePeer*>(peerPtr)) {
                total += first->msgsSent;
            } else if (auto* second = dynamic_cast<ExamplePeer2*>(peerPtr)) {
                total += second->msgsSent;
            }
        }
        LogWriter::pushValue("finalMessageCount", total);
    }
}

NetworkInterface* ExamplePeer::releaseNetworkInterface() {
    NetworkInterface* iface = _networkInterface;
    _networkInterface = nullptr;
    return iface;
}

void ExamplePeer::checkInStrm() {
    while (!inStreamEmpty()) {
        Packet packet = popInStream();
        logInboundMessage(packet);
    }
}

void ExamplePeer::logInboundMessage(const Packet& packet) const {
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
                // leave sender as packet source if conversion fails
            }
        }
    }
    logEntry["from"] = sender;
    logEntry["receivedRound"] = RoundManager::currentRound();
    logEntry["contents"] = payload;
    LogWriter::pushValue("receivedMessages", logEntry);
}

json ExamplePeer::buildGreetingPayload() const {
    json payload;
    payload["type"] = "greeting";
    payload["from"] = publicId();
    payload["roundSent"] = RoundManager::currentRound();
    payload["message"] = "Message: Hello From " + std::to_string(publicId()) +
                           ". Sent on round: " + std::to_string(RoundManager::currentRound());
    payload["sequence"] = msgsSent;
    return payload;
}

void ExamplePeer::logSentMessages(const std::vector<Peer*>& peers) const {
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
