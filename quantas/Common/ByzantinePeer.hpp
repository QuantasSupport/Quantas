/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QUANTAS is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef BYZANTINE_PEER_HPP
#define BYZANTINE_PEER_HPP

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <memory>
#include "Peer.hpp"
#include "Faults.hpp"

namespace quantas {

using nlohmann::json;

class ByzantinePeer : public Peer {
public:
inline ByzantinePeer() {}
    inline ByzantinePeer(NetworkInterface* networkInterface) : Peer(networkInterface) {}
    inline ByzantinePeer(const ByzantinePeer &rhs) {};
    inline virtual ~ByzantinePeer() {};

    virtual void tryPerformComputation() override {
        std::cout << "ByzantinePeer.hpp: " << internalId() << std::endl;
        if (!isCrashed()) {
            bool skipRegular = faultManager.applyPerformComputation(this);
            if (!skipRegular)
                performComputation();
        }
    };

    virtual void initParameters(const std::vector<Peer*>& peers, json parameters) {}

    virtual void performComputation() = 0;

    // Send messages to to others using these
    virtual void unicastTo (json msg, const interfaceId& dest) override { 
        bool skipRegular = faultManager.applyUnicastTo(this, msg, dest);
        if (!skipRegular) 
            _networkInterface->unicastTo(msg, dest);
    };

    virtual void unicast (json msg) override { 
        bool skipRegular = faultManager.applySend(this, msg, "unicast");
        if (!skipRegular) 
            _networkInterface->unicast(msg);
    };

    virtual void multicast (json msg, const std::set<interfaceId>& targets) override { 
        bool skipRegular = faultManager.applySend(this, msg, "multicast", targets);
        if (!skipRegular) 
            _networkInterface->multicast(msg, targets);
    };

    virtual void broadcast (json msg) override { 
        bool skipRegular = faultManager.applySend(this, msg, "broadcast");
        if (!skipRegular) 
            _networkInterface->broadcast(msg);
    };

    virtual void broadcastBut (json msg, const interfaceId& id) override { 
        _networkInterface->broadcastBut(msg, id); 
    };

    virtual void randomMulticast (json msg) override { 
        bool skipRegular = faultManager.applySend(this, msg, "randomMulticast");
        if (!skipRegular) 
            _networkInterface->randomMulticast(msg);
    };

    // moves msgs to the inStream if they've arrived
    void receive() { _networkInterface->receive(); };

    void addFault(Fault* fault) {faultManager.addFault(fault);};

protected:
    FaultManager faultManager;
};

} // namespace quantas

#endif // BYZANTINE_PEER_HPP
