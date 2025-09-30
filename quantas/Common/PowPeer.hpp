/*
Copyright 2024

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version. QUANTAS is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef POWPEER_HPP
#define POWPEER_HPP

#include <vector>

#include "ByzantinePeer.hpp"
#include "Pow.hpp"

namespace quantas {

// Base class for PoW-style peers that share a single on-chain state container.
class PoWPeer : public ByzantinePeer {
public:
    PoWPeer(NetworkInterface* ni) : ByzantinePeer(ni) {}
    PoWPeer(const PoWPeer& rhs) : ByzantinePeer(rhs), _pow(nullptr) {}
    ~PoWPeer() override {
        delete _pow;
    }

    void setPoW(PoW* pow) {
        if (_pow == pow) return;
        // Peers may swap in a shared ledger instance during configuration.
        if (_pow != nullptr) delete _pow;
        _pow = pow;
    }

    PoW* pow() const { return _pow; }

    virtual void runProtocolStep(const std::vector<std::string>& overrideParents = {}) = 0;

protected:
    // Owned pointer to the shared PoW metadata (committee id 0 in our scenario).
    PoW* _pow = nullptr;
};

}

#endif // POWPEER_HPP
