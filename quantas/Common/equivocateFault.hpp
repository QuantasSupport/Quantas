/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef EQUIVOCATEFAULT_HPP
#define EQUIVOCATEFAULT_HPP

#include <iostream>
#include <functional>
#include <sstream>

#include "../Common/Faults.hpp"

namespace quantas {

// Equivocate on certian messages sending quorumA normal messages
// and quorumB messages tainted with a filpped indicator set to true
class EquivocateFault : public Fault {
private:
    std::set<interfaceId> quorumA;
    std::set<interfaceId> quorumB;
    std::set<std::string> applicableMessageTypes;

    static void intersectTargets(const std::set<interfaceId>& src,
                                 const std::set<interfaceId>& filt,
                                 std::set<interfaceId>& out) {
        out.clear();
        std::set_intersection(src.begin(), src.end(),
                              filt.begin(), filt.end(),
                              std::inserter(out, out.begin()));
    }

public:

    EquivocateFault(const std::set<interfaceId>& A,
                    const std::set<interfaceId>& B,
                    const std::set<std::string> messageTypes
                )
        : quorumA(A), quorumB(B), applicableMessageTypes(messageTypes) {}

    virtual ~EquivocateFault() = default;

    // All PBFT control traffic in your code uses multicast; that's all we need to override.
    bool overridesSendType(const std::string& sendType) const override {
        return sendType == "multicast";
    }

    bool onSend(Peer* peer,
                json& msg,
                const std::string& sendType,
                const std::set<interfaceId>& targets = {}) override
    {
        if (sendType != "multicast") return false;
        if (!msg.contains("MessageType")) return false;

        const std::string mt = msg["MessageType"].get<std::string>();

        if (!applicableMessageTypes.count(mt)) return false;
        if (targets.empty()) return false;

        // Partition the current multicast's target set by fixed quorums.
        std::set<interfaceId> A, B;
        intersectTargets(targets, quorumA, A);
        intersectTargets(targets, quorumB, B);

        // Build the conflicting variant
        json alt = msg;
        if (mt == "checkpoint") {
            // flip checkpoint digest but keep seqNum/view the same
            if (alt.contains("digest") && alt["digest"].is_string()) {
                std::string suffix = "#flip";
                std::string s = alt["digest"].get<std::string>();
                if (s.size() >= suffix.size() &&
                    s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0) {
                    s.erase(s.size() - suffix.size());
                }

                msg["digest"] = s;
                alt["digest"] = msg["digest"].get<std::string>() + suffix;
            } else {
                alt["digest"] = std::to_string(std::hash<std::string>{}("alt#" + alt.dump()));
                // may cause issues as the 'original' msg may still contain an altered digest
            }
        } else {
            if (!msg.contains("proposal")) msg["proposal"] = json::object();
            if (!msg["proposal"].contains("Request")) msg["proposal"]["Request"] = json::object();
            msg["proposal"]["Request"]["fault_flip"] = false;
            if (!alt.contains("proposal")) alt["proposal"] = json::object();
            if (!alt["proposal"].contains("Request")) alt["proposal"]["Request"] = json::object();
            alt["proposal"]["Request"]["fault_flip"] = true;
        }

        // Send two conflicting versions to disjoint quorums.
        peer->getNetworkInterface()->multicast(msg, A);
        peer->getNetworkInterface()->multicast(alt, B);

        // Swallow original multicast
        return true;
    }
};

}

#endif /* EQUIVOCATEFAULT_HPP */