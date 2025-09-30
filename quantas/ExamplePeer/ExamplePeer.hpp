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

#ifndef ExamplePeer_hpp
#define ExamplePeer_hpp

#include <vector>
#include "../Common/Peer.hpp"

namespace quantas {

class Packet;
class ExamplePeer2;

class ExamplePeer : public Peer {
public:
    ExamplePeer(NetworkInterface* networkInterface);
    ExamplePeer(const ExamplePeer& rhs);
    ~ExamplePeer() override;

    void initParameters(std::vector<Peer*>& peers, json parameters);
    void performComputation() override;
    void endOfRound(std::vector<Peer*>& peers) override;

    NetworkInterface* releaseNetworkInterface();

    int msgsSent = 0;
    bool changePeerType = false;

private:
    void checkInStrm();
    void logInboundMessage(const Packet& packet) const;
    json buildGreetingPayload() const;
    void logSentMessages(const std::vector<Peer*>& peers) const;
};

} // namespace quantas

#endif /* ExamplePeer_hpp */
