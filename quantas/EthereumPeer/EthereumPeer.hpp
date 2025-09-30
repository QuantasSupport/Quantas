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

#ifndef ETHEREUMPEER_HPP
#define ETHEREUMPEER_HPP

#include <deque>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "../Common/PowPeer.hpp"

namespace quantas {

// PoW peer that follows Ethereum's GHOST weighting rule via PoWEthereum.
class EthereumPeer : public PoWPeer {
public:
    EthereumPeer(NetworkInterface* interfacePtr);
    EthereumPeer(const EthereumPeer& rhs);
    ~EthereumPeer() override = default;

    void performComputation() override;
    void runProtocolStep(const std::vector<std::string>& overrideParents = {}) override;
    void initParameters(const std::vector<Peer*>& peers, json parameters) override;
    void endOfRound(std::vector<Peer*>& peers) override;

private:
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
    json buildTransactionMessage(const PendingTx& pending) const;
    json buildBlockMessage(const PoW::BlockRecord& record,
                           const std::vector<std::string>& parents,
                           int minedRound,
                           const PendingTx& pending) const;

    mutable int submitRate = 20;
    int _mineRate = 1;
    int _mineDenominator = 100;

    std::deque<PendingTx> _queue;
    std::set<std::pair<interfaceId, int>> _knownTransactions;
    int _localSubmitted = 0;
};

}

#endif // ETHEREUMPEER_HPP
