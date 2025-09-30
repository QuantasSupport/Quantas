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

#ifndef POWBITCOIN_HPP
#define POWBITCOIN_HPP

#include "../Common/Pow.hpp"

namespace quantas {

// Bitcoin-specific PoW ledger that applies the traditional longest-chain rule
// and exposes Bitcoin's single-parent mining policy to peers.
class PoWBitcoin : public PoW {
public:
    explicit PoWBitcoin(Committee* committee)
        : PoW(committee) {}
    ~PoWBitcoin() override = default;

protected:
    bool preferCandidate(const BlockRecord& candidate,
                         const BlockRecord* incumbent) const override {
        if (!incumbent) return true;
        if (candidate.height != incumbent->height) {
            return candidate.height > incumbent->height;
        }
        // Tie-break deterministically to keep behaviour stable across peers.
        return candidate.hash < incumbent->hash;
    }

    std::vector<std::string> selectParentsForNextBlock(const BlockRecord* best) const override {
        if (best) {
            return {best->hash};
        }
        return {"GENESIS"};
    }
};

}

#endif // POWBITCOIN_HPP
