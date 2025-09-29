/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef PBFTPEER_HPP
#define PBFTPEER_HPP

#include <deque>
#include <vector>
#include <map>
#include "../Common/ConsensusPeer.hpp"


namespace quantas{

    using std::vector;
    using std::map;

    class PBFTPeer : public ConsensusPeer {
    public:
        // methods that must be defined when deriving from Peer
        PBFTPeer                             (NetworkInterface*);
        PBFTPeer                             (const PBFTPeer &rhs);
        ~PBFTPeer                            ();

        void performComputation() override;
        // initialize the configuration of the system
        void initParameters(const std::vector<Peer*>& peers, json parameters) override;
        
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void endOfRound(vector<Peer*>& _peers) override;
        
    };
}
#endif /* PBFTPEER_HPP */
