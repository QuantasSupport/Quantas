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

#ifndef ExamplePeer2_hpp
#define ExamplePeer2_hpp

#include <vector>
#include "ExamplePeer.hpp"

namespace quantas {

class Packet;

class ExamplePeer2 : public Peer {
public:
    ExamplePeer2(NetworkInterface* networkInterface);
    ExamplePeer2(const ExamplePeer2& rhs);
    explicit ExamplePeer2(ExamplePeer* rhs);
    ~ExamplePeer2() override;

    void initParameters(std::vector<Peer*>& peers, json parameters);
    void performComputation() override;
    void endOfRound(std::vector<Peer*>& peers) override;

    int msgsSent = 0;

private:
    void checkInStrm();
    void logInboundMessage(const Packet& packet) const;
    json buildGreetingPayload(int sequenceNumber, int emissionIndex) const;
    void logSentMessages(const std::vector<Peer*>& peers) const;
};

} // namespace quantas

#endif /* ExamplePeer2_hpp */
