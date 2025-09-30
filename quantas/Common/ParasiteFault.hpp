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

#ifndef PARASITEFAULT_HPP
#define PARASITEFAULT_HPP

#include <algorithm>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "Faults.hpp"
#include "PowPeer.hpp"

namespace quantas {

// Fault that emulates a parasite (selfish) mining coalition.  Attackers withhold blocks,
// exchange them privately, mine on the hidden chain, and release it when it overtakes the
// public chain by the configured lead threshold.
class ParasiteFault : public Fault {
public:
    ParasiteFault(int leadThreshold, const std::set<interfaceId>& collaborators)
        : _leadThreshold(std::max(1, leadThreshold)), _collaborators(collaborators) {}

    virtual ~ParasiteFault() = default;

    bool overridesSendType(const std::string& sendType) const override {
        return sendType == "broadcast";
    }

    bool overridesReceive() const override {
        return true;
    }

    bool overridesPerformComputation() const override {
        return true;
    }

    bool onSend(Peer* peer,
                json& msg,
                const std::string& sendType,
                const std::set<interfaceId>& = {}) override {
        if (sendType != "broadcast") return false;
        if (!msg.contains("type") || msg["type"] != "PoW") return false;
        if (msg.value("messageType", std::string()) != "block") return false;

        msg["block"]["parasite"] = true; // mark broadcast so observers can distinguish parasite blocks
        storePrivate(msg);

        json privateMsg = msg;
        privateMsg["parasite_private"] = true;
        for (auto collaborator : _collaborators) {
            if (collaborator == peer->publicId()) continue;
            peer->getNetworkInterface()->unicastTo(privateMsg, collaborator);
        }

        tryRelease(peer);
        return true; // suppress public broadcast for now
    }

    bool onReceive(Peer* peer, json& msg, const interfaceId& src) override {
        if (!msg.contains("type") || msg["type"] != "PoW") return false;
        if (msg.value("messageType", std::string()) != "block") {
            return false;
        }

        const bool isPrivate = msg.value("parasite_private", false);
        const int height = extractHeight(msg);

        if (isPrivate && _collaborators.count(src)) {
            storePrivate(msg);
            tryRelease(peer);
            return false;
        }

        _publicHeight = std::max(_publicHeight, height);
        return false;
    }

    bool onPerformComputation(Peer* peer) override {
        std::vector<std::string> parents = selectPrivateParents();
        if (auto* powPeer = dynamic_cast<PoWPeer*>(peer)) {
            powPeer->runProtocolStep(parents);
            return true;
        }
        return false;
    }

private:
    std::vector<std::string> selectPrivateParents() const {
        if (_privateChain.empty()) return {};
        const StoredBlock* best = nullptr;
        for (const auto& block : _privateChain) {
            if (!best || block.height > best->height ||
                (block.height == best->height && block.hash < best->hash)) {
                best = &block;
            }
        }
        if (!best) return {};
        if (!best->hash.empty()) {
            return {best->hash};
        }
        return best->parents;
    }

    struct StoredBlock {
        json message;
        int height = 0;
        std::string hash;
        std::vector<std::string> parents;
    };

    void storePrivate(const json& msg) {
        if (!msg.contains("block")) return;
        const json& block = msg["block"];
        if (!block.is_object()) return;
        std::string hash = block.value("hash", std::string());
        if (hash.empty()) return;
        if (_privateHashes.insert(hash).second) {
            StoredBlock stored;
            stored.message = msg;
            stored.height = block.value("length", block.value("height", 0));
            stored.hash = hash;
            if (block.contains("parents") && block["parents"].is_array()) {
                for (const auto& parent : block["parents"]) {
                    if (parent.is_string()) stored.parents.push_back(parent.get<std::string>());
                }
            }
            _privateHeight = std::max(_privateHeight, stored.height);
            _privateChain.push_back(std::move(stored));
        }
    }

    void tryRelease(Peer* peer) {
        if (_privateChain.empty()) return;
        if (_privateHeight < _publicHeight + _leadThreshold) return;

        // Release everything in height order so honest peers see a coherent alternative chain.
        std::sort(_privateChain.begin(), _privateChain.end(),
                  [](const StoredBlock& a, const StoredBlock& b) {
                      if (a.height == b.height) {
                          return a.hash < b.hash;
                      }
                      return a.height < b.height;
                  });
        for (auto& stored : _privateChain) {
            json releaseMsg = stored.message;
            if (releaseMsg.contains("parasite_private")) {
                releaseMsg.erase("parasite_private");
            }
            peer->getNetworkInterface()->broadcast(releaseMsg);
        }
        _publicHeight = std::max(_publicHeight, _privateHeight);
        _privateHeight = 0;
        _privateChain.clear();
        _privateHashes.clear();
    }

    int extractHeight(const json& msg) const {
        if (!msg.contains("block")) return 0;
        const json& block = msg["block"];
        if (block.contains("length") && block["length"].is_number_integer()) {
            return block["length"].get<int>();
        }
        if (block.contains("height") && block["height"].is_number_integer()) {
            return block["height"].get<int>();
        }
        return 0;
    }

    int _leadThreshold;
    std::set<interfaceId> _collaborators;
    std::vector<StoredBlock> _privateChain;
    std::unordered_set<std::string> _privateHashes;
    int _privateHeight = 0;
    int _publicHeight = 0;
};

}

#endif // PARASITEFAULT_HPP
