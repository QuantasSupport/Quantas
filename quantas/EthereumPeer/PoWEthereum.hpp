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

#ifndef POWETHEREUM_HPP
#define POWETHEREUM_HPP

#include "../Common/Pow.hpp"

namespace quantas {

// Ethereum-specific PoW ledger that applies a simplified GHOST rule: the
// preferred head is the one whose subtree (including itself) contains the most
// blocks. Heights are used only as a tie breaker to keep behaviour stable.
class PoWEthereum : public PoW {
public:
    explicit PoWEthereum(Committee* committee)
        : PoW(committee) {}
    ~PoWEthereum() override = default;

protected:
    bool preferCandidate(const BlockRecord& candidate,
                         const BlockRecord* incumbent) const override {
        if (!incumbent) return true;
        if (isAncestor(incumbent->hash, candidate.hash)) {
            return true;
        }
        if (isAncestor(candidate.hash, incumbent->hash)) {
            return false;
        }

        const int candidateWeight = subtreeSize(candidate.hash);
        const int incumbentWeight = subtreeSize(incumbent->hash);
        if (candidateWeight != incumbentWeight) {
            return candidateWeight > incumbentWeight;
        }
        if (candidate.height != incumbent->height) {
            return candidate.height > incumbent->height;
        }
        return candidate.hash < incumbent->hash;
    }
};

}

#endif // POWETHEREUM_HPP
