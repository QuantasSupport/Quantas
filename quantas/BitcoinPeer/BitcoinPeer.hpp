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

#ifndef BITCOINPEER_HPP
#define BITCOINPEER_HPP

#include <deque>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "../Common/PowPeer.hpp"

namespace quantas {

// Proof-of-Work peer that communicates entirely with JSON messages and relies on
// PoW for lightweight block bookkeeping.
class BitcoinPeer : public PoWPeer {
public:
    BitcoinPeer(NetworkInterface* interfacePtr);
    BitcoinPeer(const BitcoinPeer& rhs);
    ~BitcoinPeer() override = default;

    void performComputation() override;
    void runProtocolStep(const std::vector<std::string>& overrideParents = {}) override;
    void initParameters(const std::vector<Peer*>& peers, json parameters) override;
    void endOfRound(std::vector<Peer*>& peers) override;

private:
    // Minimal description of a queued transaction
    struct PendingTx {
        int id = -1;
        int roundSubmitted = -1;
        interfaceId submitter = NO_PEER_ID;
    };

    void checkInStrm();
    bool guardSubmit() const;
    bool guardMine() const;
    std::vector<std::string> getParents(const PoW& group) const;
    PendingTx makeTransaction();
    // turns contents into a sendable json format
    json buildTransactionMessage(const PendingTx& pending) const;
    json buildBlockMessage(const PoW::BlockRecord& record,
                           const std::vector<std::string>& parents,
                           int minedRound,
                           const PendingTx& pending) const;

    mutable int submitRate = 20;
    int _mineRate = 1; // mining is determined as mineRate / mineDenominator
    int _mineDenominator = 100; // this comes from Sum(everyones mine rate) * scalar from input

    std::deque<PendingTx> _queue; // current queue of pending txs (has issues when switching branches to be fixed)
    std::set<std::pair<interfaceId, int>> _knownTransactions; // all known transactions (kept to ensure consistency with the pending queue)
    int _localSubmitted = 0; // transaction id counter
    int minedBlocks = 0; // total blocks mined by this peer
};

}

#endif // BITCOINPEER_HPP
