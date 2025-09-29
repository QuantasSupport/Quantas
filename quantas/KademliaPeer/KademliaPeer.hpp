/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KademliaPeer_hpp
#define KademliaPeer_hpp

#include <set>
#include <string>
#include <vector>

#include "../Common/Json.hpp"
#include "../Common/Peer.hpp"

namespace quantas {

struct KademliaFinger {
    interfaceId Id{NO_PEER_ID};     // id of the finger
    std::string binId;              // binary id of a finger
    int group{-1};                  // the level the finger belongs to (binary id difference)
};

class KademliaPeer : public Peer {
public:
    KademliaPeer(NetworkInterface*);
    KademliaPeer(const KademliaPeer& rhs);
    ~KademliaPeer() override;

    void initParameters(const std::vector<Peer*>& peers, json parameters) override;
    void performComputation() override;
    void endOfRound(std::vector<Peer*>& peers) override;

private:
    // high-level workflow
    void checkInStrm();
    void handleLookup(json msg);
    void submitLookup(int transactionId);

    // helpers
    void applyGlobalParameters();
    void ensureInitialized();
    std::string getBinaryId(interfaceId id) const;
    interfaceId findRoute(const std::string& targetBinaryId,
                          interfaceId targetId,
                          const std::set<interfaceId>& neighborSet) const;
    interfaceId selectFingerForGroup(int group,
                                     const std::set<interfaceId>& neighborSet) const;
    interfaceId selectClosestByDistance(interfaceId targetId,
                                        const std::set<interfaceId>& neighborSet) const;
    static int firstDifferentBit(const std::string& lhs, const std::string& rhs);
    void rebuildFingerTable(const std::set<interfaceId>& neighborSet);
    size_t neighborFingerprint(const std::set<interfaceId>& neighborSet) const;
    json makeLookupMessage(interfaceId targetId,
                           const std::string& targetBinaryId,
                           int transactionId) const;

    static int s_currentTransactionId;
    static std::vector<interfaceId> s_allPeerIds;
    static int s_binaryIdSize;

    int _binaryIdSize{0};
    std::string _binaryId{""};
    std::vector<interfaceId> _allPeerIds;
    std::vector<KademliaFinger> _fingers;
    size_t _lastNeighborFingerprint{0};

    int _requestsSatisfied{0};
    int _totalHops{0};
    int _latency{0};
    bool _alive{true};
    bool _initialized{false};
};
}
#endif /* KademliaPeer_hpp */
