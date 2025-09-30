#ifndef FAULTS_HPP
#define FAULTS_HPP

#include "Peer.hpp"
#include <vector>
#include <map>
#include <string>
#include <unordered_set>

namespace quantas {

class Fault {
public:

    Fault() {};
    virtual ~Fault() {};

    // Return true if it should be considered this type of fault
    virtual bool overridesUnicastTo() const { return false; }
    virtual bool overridesSendType(const std::string&) const { return false; }
    virtual bool overridesReceive() const { return false; }
    virtual bool overridesPerformComputation() const { return false; }

    // Hooks
    virtual bool onUnicastTo(Peer* peer, json& msg, const interfaceId& id) { return false; }
    virtual bool onSend(Peer* peer, json& msg, const std::string& sendType, const std::set<interfaceId>& targets = std::set<interfaceId>()) { return false; }
    virtual bool onReceive(Peer* peer, json& msg, const interfaceId& id) { return false; }
    virtual bool onPerformComputation(Peer* peer) { return false; }
};

class FaultManager {
public:
    ~FaultManager() {
        std::unordered_set<Fault*> seen;

        auto null_in = [&](auto& vec, Fault* f){
            for (auto& x : vec) if (x == f) x = nullptr;
        };
        auto null_everywhere = [&](Fault* f){
            null_in(unicastToFaults, f);
            null_in(receiveFaults, f);
            null_in(computationFaults, f);
            for (auto& [_, v] : sendFaults) null_in(v, f);
        };

        auto deleteVec = [&](std::vector<Fault*>& vec){
            for (Fault*& f : vec) {
            if (!f) continue;
            if (seen.insert(f).second) {  // first time we see this address
                delete f;
            }
            null_everywhere(f);           // wipe all copies of the pointer
            }
        };

        deleteVec(unicastToFaults);
        deleteVec(receiveFaults);
        deleteVec(computationFaults);
        for (auto& [_, v] : sendFaults) deleteVec(v);
    }


    void addFault(Fault* fault) {

        if (fault->overridesUnicastTo()) {
            unicastToFaults.push_back(fault);
        }

        // Check overridden onSend behavior for known types
        for (const std::string& sendType : knownSendTypes) {
            // std::cout << "try to add\n";
            if (fault->overridesSendType(sendType)) {
                // std::cout << "added\n";
                sendFaults[sendType].push_back(fault);
            }
        }

        if (fault->overridesReceive()) {
            receiveFaults.push_back(fault);
        }

        if (fault->overridesPerformComputation()) {
            computationFaults.push_back(fault);
        }
    }

    bool applyUnicastTo(Peer* peer, json& msg, const interfaceId& dest) {
        bool overridden = false;
        for (auto* f : unicastToFaults)
            overridden |= f->onUnicastTo(peer, msg, dest);
        return overridden;
    }

    bool applySend(Peer* peer, json& msg, const std::string& sendType, const std::set<interfaceId>& targets = std::set<interfaceId>()) {
        bool overridden = false;
        auto it = sendFaults.find(sendType);
        if (it != sendFaults.end()) {
            for (auto* f : it->second)
                overridden |= f->onSend(peer, msg, sendType, targets);
        }
        return overridden;
    }

    bool applyReceive(Peer* peer, json& msg, const interfaceId& src) {
        bool overridden = false;
        for (auto* f : receiveFaults)
            overridden |= f->onReceive(peer, msg, src);
        return overridden;
    }

    bool applyPerformComputation(Peer* peer) {
        bool overridden = false;
        for (auto* f : computationFaults)
            overridden |= f->onPerformComputation(peer);
        return overridden;
    }

    

private:
    std::vector<Fault*> unicastToFaults;
    std::unordered_map<std::string, std::vector<Fault*>> sendFaults;
    std::vector<Fault*> receiveFaults;
    std::vector<Fault*> computationFaults;

    const std::vector<std::string> knownSendTypes = {
        "unicast", "broadcast",  "multicast", "randomMulticast", "broadcastBut"
    };

};


}

#endif /* FAULTS_HPP */