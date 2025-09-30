/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef RAFTPEER_HPP
#define RAFTPEER_HPP

#include "../Common/ConsensusPeer.hpp"

namespace quantas {

class RaftPeer : public ConsensusPeer {
public:
    RaftPeer(NetworkInterface* networkInterface);
    RaftPeer(const RaftPeer& rhs);
    ~RaftPeer() override;

    void performComputation() override;
    void initParameters(const std::vector<Peer*>& peers, json parameters) override;
    void endOfRound(std::vector<Peer*>& peers) override;

    double crashOdds() const { return _crashOdds; }
    void setCrashOdds(double odds) { _crashOdds = odds; }

private:
    void maybeCrash();

    double _crashOdds = 0.0;
    size_t _crashRecoveryDelay = 0;
};

} // namespace quantas

#endif /* RAFTPEER_HPP */
