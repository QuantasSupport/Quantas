/*
Copyright 2023

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

#ifndef EyeWitnessPeer_hpp
#define EyeWitnessPeer_hpp

#include <algorithm>
#include <atomic>
#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

// unsolved design problems:

// warning messages: other nodes in neighborhood being like "this person who is
// trying to spend this money has provably signed it away"

namespace quantas {
// -- basic data storage types --

struct Neighborhood {
    int id;
    std::unordered_set<long> memberIDs;
    int leader; // id of the leader of this neighborhood
    bool operator==(const Neighborhood &rhs) {
        return memberIDs == rhs.memberIDs && leader == rhs.leader;
    }
    int size() { return memberIDs.size(); }
    bool has(long peerID) { return memberIDs.count(peerID) > 0; }
};

struct WalletLocation {
    // this is all we need to know for wallets stored in transaction
    // histories

    int address;
    Neighborhood storedBy;
    bool operator==(const WalletLocation &rhs) {
        return address == rhs.address && storedBy == rhs.storedBy;
    }
};

struct TransactionRecord {
    // maybe store something symbolizing the sender's signature?

    WalletLocation sender;
    WalletLocation receiver;
};

struct Coin {
    // id needs to be unique across all coins across all peers; use static
    // counter variable in peer class? or uuid generator.
    int id;
    vector<TransactionRecord> history;
};

struct LocalWallet : public WalletLocation {
    // we do not care about these things for wallets that are only mentioned
    // in transactions, but we need to store them for local wallets

    vector<Coin> coins;
    vector<Coin> pastCoins;
    // moves a Coin from coins to pastCoins. assumes that the version of the
    // Coin passed as an argument has the most recent history and should be
    // stored
    void moveToHistory(Coin target) {
        coins.erase(
            std::remove_if(coins.begin(), coins.end(),
                           [&target](Coin c) { return c.id == target.id; }),
            coins.end());
        pastCoins.push_back(target);
    }
};

struct OngoingTransaction : public TransactionRecord {
    // we just need this to verify history for transactions we are currently
    // validating

    Coin coin;
    int roundSubmitted;
    int roundCompleted;
};

struct EyeWitnessMessage {
    OngoingTransaction trans;
    string messageType = "";
    int sequenceNum;
};

// recipients of different kinds of messages during consensus. i am assuming
// for this version of the simulation that each peer is in only one
// neighborhood
struct ConsensusContacts {
    std::vector<Neighborhood> participants;
    ConsensusContacts() = default;
    ConsensusContacts(Coin c, int validatorCount) {
        std::unordered_set<int> uniques;
        for (int i = 0; uniques.size() < validatorCount && i < c.history.size();
             i++) {
            Neighborhood &prevNeighborhood =
                c.history[c.history.size() - i - 1].receiver.storedBy;
            if (uniques.count(prevNeighborhood.id) == 0) {
                participants.push_back(prevNeighborhood);
                uniques.insert(prevNeighborhood.id);
            }
        }
    }
    // Precondition: peerID is an id in one of the neighborhoods
    Neighborhood getOwn(long peerID) {
        auto own = std::find_if(participants.begin(), participants.end(),
                                [peerID](Neighborhood &n) {
                                    return n.memberIDs.count(peerID) > 0;
                                });
        return *own;
    }
    std::vector<Neighborhood> getOthers(long peerID) {
        std::vector<Neighborhood> others;
        std::copy_if(participants.begin(), participants.end(),
                     std::back_inserter(others), [peerID](Neighborhood &n) {
                         return n.memberIDs.count(peerID) == 0;
                     });
        return others;
    }
    int getValidatorNodeCount() {
        int sum = 0;
        for (const Neighborhood &participant : participants) {
            sum += participant.memberIDs.size();
        }
        return sum;
    }
};

// -- implementation of underlying consensus algorithm --

class StateChangeRequest {
  public:
    virtual bool consensusSucceeded() const = 0;
    virtual void updateConsensus() = 0;
    virtual void addToConsensus(EyeWitnessMessage) = 0;
    bool outboxEmpty() { return outbox.size() == 0; }

    // Precondition: !outboxEmpty()
    EyeWitnessMessage getMessage() {
        EyeWitnessMessage m = outbox.back();
        outbox.pop_back();
        return m;
    }

  protected:
    vector<EyeWitnessMessage> outbox;
};

class PBFTRequest : public StateChangeRequest {
  public:
    PBFTRequest(OngoingTransaction t, int neighbors, int seq, bool amLeader)
        : transaction(t), neighborhoodSize(neighbors), sequenceNum(seq),
          leader(amLeader), status("pre-prepare") {}

    void updateConsensus() override;
    void addToConsensus(EyeWitnessMessage c) override;
    bool consensusSucceeded() const override;
    OngoingTransaction getTransaction() { return transaction; }
    int getSequenceNumber() { return sequenceNum; }

  protected:
    OngoingTransaction transaction;
    int neighborhoodSize;
    int sequenceNum;
    string status;
    bool leader;
    // Count of the number of times we've seen these messages
    std::map<string, int> statusCount = {
        {"pre-prepare", 0}, {"prepare", 0}, {"commit", 0}};
};

// -- peer class that participates directly in the network --
template <typename ConsensusRequest = PBFTRequest>
class EyeWitnessPeer : public Peer<EyeWitnessMessage> {
  public:
    // methods that must be defined when deriving from Peer

    EyeWitnessPeer(long id);
    // EyeWitnessPeer(const EyeWitnessPeer &rhs);

    void initParameters(const vector<Peer *> &_peers, json parameters) override;

    // perform one step of the algorithm with the messages in inStream
    void performComputation() override;

    void endOfRound(const vector<Peer<EyeWitnessMessage> *> &_peers) override;

    // perform any calculations needed at the end of a round such as
    // determining the throughput (only ran once, not for every peer)
    // void endOfRound(const vector<Peer<EyeWitnessMessage> *> &_peers)
    // override;

    void broadcastTo(EyeWitnessMessage, Neighborhood);
    void initiateTransaction(bool withinNeighborhood = true);

  private:
    std::unordered_map<int, ConsensusRequest> localRequests;
    std::unordered_map<int, ConsensusRequest> superRequests;
    std::vector<ConsensusRequest> finishedRequests;
    // std::multimap<int, PBFTRequest> recievedRequests;
    // TODO: this should also probably be a container with lookup based on id
    // (address)
    std::vector<LocalWallet> heldWallets;
    int neighborhoodID;
    std::unordered_map<int, ConsensusContacts> contacts;

    // logs that are gathered up and sent to the LogWriter all at once in the
    // last endOfRound(); events aren't sent to the LogWriter directly because
    // that is not thread-safe
    std::vector<json> transactions;
    std::vector<json> validations;
    std::vector<json> messages;

    // technically not realistic for this to be known to all nodes
    inline static std::atomic<int> previousSequenceNumber = -1;
    inline static int issuedCoins = 0;

    // parameters from input file; read in initParameters
    inline static int maxNeighborhoodSize = -1;
    inline static int neighborhoodCount = -1;
    inline static int validatorNeighborhoods = -1;
    inline static int byzantineRound = -1;

    // global neighborhood data; populated as a side effect of initParameters
    inline static std::vector<std::vector<LocalWallet>> walletsForNeighborhoods;
    inline static std::unordered_map<long, int> neighborhoodsForPeers;
    inline static std::vector<Neighborhood> neighborhoods;
};

Simulation<quantas::EyeWitnessMessage, quantas::EyeWitnessPeer<PBFTRequest>> *
generateSim();

// -- peer class that participates directly in the network --

// gives each peer a neighborhood and gives each neighborhood a set of wallets.
// precondition: "validatorNeighborhoods", "neighborhoodSize", and
// "walletsPerNeighborhood" are set as parameters in the input file
// postcondition: maxNeighborhoodSize is set; neighborhoodCount is set;
// byzantineRound is set; walletsForNeighborhoods contains a vector of
// LocalWallets for each neighborhood; neighborhoodsForPeers maps a peer's id to
// the index of the neighborhood it's in; static members are reset
template <typename ConsensusRequest>
void EyeWitnessPeer<ConsensusRequest>::initParameters(
    const vector<Peer<EyeWitnessMessage> *> &_peers, json parameters) {
    // divide peers into neighborhoods: if the neighborhood size is 5, then
    // the first 5 peers are placed in a neighborhood, then the next 5
    // peers, and so on. if the total number of peers is not a multiple of
    // 5, an "extra" neighborhood stores the spares. then, each neighborhood
    // gets a set of wallets to take care of.

    previousSequenceNumber = -1;
    issuedCoins = 0;
    walletsForNeighborhoods = std::vector<std::vector<LocalWallet>>();
    neighborhoodsForPeers = std::unordered_map<long, int>();
    neighborhoods = std::vector<Neighborhood>();

    validatorNeighborhoods = parameters["validatorNeighborhoods"];
    maxNeighborhoodSize = parameters["neighborhoodSize"];
    const int walletsPerNeighborhood = parameters["walletsPerNeighborhood"];
    if (parameters.contains("byzantineRound")) {
        byzantineRound = parameters["byzantineRound"];
    }

    LogWriter::getTestLog()["roundInfo"]["rounds"] = getLastRound();
    LogWriter::getTestLog()["roundInfo"]["byzantineRound"] = byzantineRound;

    neighborhoodCount =
        ceil(static_cast<float>(_peers.size()) / maxNeighborhoodSize);
    walletsForNeighborhoods.resize(neighborhoodCount);
    vector<WalletLocation> all;
    for (int i = 0; i < neighborhoodCount; ++i) {
        Neighborhood newNeighborhood;
        newNeighborhood.id = i;
        newNeighborhood.leader = i * maxNeighborhoodSize;
        int actualNeighborhoodSize =
            std::min(maxNeighborhoodSize,
                     static_cast<int>(_peers.size()) - i * maxNeighborhoodSize);
        for (int n = 0; n < actualNeighborhoodSize; ++n) {
            const long peerID = i * maxNeighborhoodSize + n;
            newNeighborhood.memberIDs.insert(peerID);
            neighborhoodsForPeers[peerID] = i;
        }
        for (int j = 0; j < walletsPerNeighborhood; ++j) {
            LocalWallet w = {
                static_cast<int>(all.size()), newNeighborhood, {}, {}};
            walletsForNeighborhoods[i].push_back(w);
            all.push_back({w.address, newNeighborhood});
        }
        neighborhoods.push_back(newNeighborhood);
    }

    const int coinsPerWallet = 5;
    const int fakeHistoryLength = 10;
    int neighborhood = 0;
    for (auto &wallets : walletsForNeighborhoods) {
        for (auto &wallet : wallets) {
            for (int i = 0; i < coinsPerWallet; ++i) {
                Coin c = {issuedCoins++, {}};
                for (int j = 0; j < fakeHistoryLength - 1; ++j) {
                    c.history.push_back(
                        {all[randMod(all.size())], all[randMod(all.size())]});
                }
                const auto &destNeighborWallets =
                    walletsForNeighborhoods[neighborhood];
                c.history.push_back(TransactionRecord{
                    c.history.back().receiver,
                    destNeighborWallets[randMod(destNeighborWallets.size())]});
                wallet.coins.push_back(c);
            }
        }
        ++neighborhood;
    }

    for (auto &n : _peers) {
        // it would be less weird to set these in the constructor, but
        // because this function (initParameters) is not static, at least
        // one Peer needs to be constructed *before* this function can run
        EyeWitnessPeer *e = dynamic_cast<EyeWitnessPeer *>(n);
        assert(e);
        e->neighborhoodID = neighborhoodsForPeers[e->id()];
        e->heldWallets = walletsForNeighborhoods[e->neighborhoodID];
    }

    std::cout << "Initialized network parameters" << std::endl;
    std::cout << "Setup:" << std::endl;
    std::cout << "Max neighborhood size: " << maxNeighborhoodSize << std::endl;
    std::cout << "Wallets per neighborhood: " << walletsPerNeighborhood
              << std::endl;
    std::cout << "Neighborhood count: " << neighborhoodCount << std::endl;

    int w = 0;
    for (auto &n : neighborhoods) {
        std::cout << "Neighborhood. Leader: " << n.leader << ", members: ";
        for (const auto &m : n.memberIDs) {
            std::cout << m << " ";
        }
        std::cout << ", wallets: ";
        for (auto &wallet : walletsForNeighborhoods[w++]) {
            std::cout << wallet.address << " ";
            assert(wallet.storedBy == n);
        }
        std::cout << std::endl;
    }
}

template <typename ConsensusRequest>
EyeWitnessPeer<ConsensusRequest>::EyeWitnessPeer(long id)
    : Peer<EyeWitnessMessage>(id) {}

template <typename ConsensusRequest>
void EyeWitnessPeer<ConsensusRequest>::performComputation() {
    // retrieve messages for requests that are in progress:
    while (!inStreamEmpty()) {
        Packet<EyeWitnessMessage> p = popInStream();
        EyeWitnessMessage message = p.getMessage();
        int seqNum = message.sequenceNum;
        typename std::unordered_map<int, ConsensusRequest>::iterator s;
        if ((s = localRequests.find(seqNum)) != localRequests.end()) {
            s->second.addToConsensus(message);
        } else if ((s = superRequests.find(seqNum)) != superRequests.end()) {
            s->second.addToConsensus(message);
        } else {
            // message about previously unknown transaction
            if (message.messageType == "pre-prepare") {
                if (message.trans.sender.storedBy ==
                    message.trans.receiver.storedBy) {
                    ConsensusRequest request(
                        message.trans,
                        message.trans.sender.storedBy.memberIDs.size(), // hmm
                        message.sequenceNum, false);
                    request.addToConsensus(message);
                    contacts.insert({message.sequenceNum,
                                     ConsensusContacts(message.trans.coin, 1)});
                    localRequests.insert(
                        {request.getSequenceNumber(), request});
                } else {
                    ConsensusContacts newContacts(message.trans.coin,
                                                  validatorNeighborhoods);
                    ConsensusRequest request(
                        message.trans, newContacts.getValidatorNodeCount(),
                        message.sequenceNum, false);
                    request.addToConsensus(message);
                    contacts.insert({message.sequenceNum, newContacts});
                    superRequests.insert(
                        {request.getSequenceNumber(), request});
                }
            }
        }
    }
    // update consensus on in-progress requests:
    for (auto s = localRequests.begin(); s != localRequests.end();) {
        ConsensusRequest &r = s->second;
        r.updateConsensus();
        if (r.consensusSucceeded()) {
            OngoingTransaction trans = r.getTransaction();
            if (trans.receiver.storedBy == trans.sender.storedBy) {
                // local transaction; commit changes, update wallets
                // TODO: optimize this search
                auto localSender =
                    std::find_if(heldWallets.begin(), heldWallets.end(),
                                 [trans](LocalWallet w) {
                                     return w.address == trans.sender.address;
                                 });
                auto localReceiver =
                    std::find_if(heldWallets.begin(), heldWallets.end(),
                                 [trans](LocalWallet w) {
                                     return w.address == trans.receiver.address;
                                 });
                if (localReceiver != heldWallets.end() &&
                    localSender != heldWallets.end()) {
                    Coin moving = trans.coin;
                    moving.history.push_back(trans);
                    localSender->moveToHistory(moving);
                    localReceiver->coins.push_back(moving);
                    std::cout << "completed local transaction "
                              << r.getSequenceNumber() << " in round "
                              << getRound() << ". Transferred coin "
                              << moving.id << " from wallet "
                              << localSender->address << " to wallet "
                              << localReceiver->address << std::endl;
                }
                finishedRequests.push_back(r);
                validations.push_back({{"round", getRound()},
                                       {"seqNum", r.getSequenceNumber()},
                                       {"peer", id()}});
                s = localRequests.erase(s);
            }
        } else {
            while (!r.outboxEmpty()) {
                EyeWitnessMessage m = r.getMessage();
                broadcastTo(m, r.getTransaction().sender.storedBy);
                messages.push_back(
                    {{"round", getRound()},
                     {"transactionType", "local"},
                     {"batchSize", r.getTransaction().sender.storedBy.size()}});
            }
            ++s;
        }
    }

    for (auto s = superRequests.begin(); s != superRequests.end();) {
        ConsensusRequest &r = s->second;
        r.updateConsensus();
        if (r.consensusSucceeded()) {
            OngoingTransaction trans = r.getTransaction();
            if (trans.sender.storedBy.has(id())) {
                auto localSender =
                    std::find_if(heldWallets.begin(), heldWallets.end(),
                                 [trans](LocalWallet w) {
                                     return w.address == trans.sender.address;
                                 });
                Coin moving = trans.coin;
                moving.history.push_back(trans);
                localSender->moveToHistory(moving);
            } else {
                // TODO: find coin in a wallet's pastCoins and edit its history,
                // for validation
            }
            finishedRequests.push_back(r);
            std::cout << "completed non-local transaction "
                      << r.getSequenceNumber() << " in round " << getRound()
                      << ". Transferred coin " << trans.coin.id
                      << " from wallet " << trans.sender.address
                      << " to wallet " << trans.receiver.address << std::endl;
            validations.push_back({{"round", getRound()},
                                   {"seqNum", r.getSequenceNumber()},
                                   {"peer", id()}});
            s = superRequests.erase(s);
        } else {
            while (!r.outboxEmpty()) {
                EyeWitnessMessage m = r.getMessage();
                std::vector<Neighborhood> recipients =
                    contacts.at(r.getSequenceNumber()).participants;
                messages.push_back(
                    {{"round", getRound()},
                     {"transactionType", "non-local"},
                     {"batchSize", contacts.at(r.getSequenceNumber())
                                       .getValidatorNodeCount()}});
                for (auto &recipient : recipients) {
                    broadcastTo(m, recipient);
                }
            }
            ++s;
        }
    }

    // assuming non-overlapping neighborhoods, so if we're the leader in one
    // wallet stored by our neighborhood we're a leader in every wallet
    // stored by our neighborhood
    if (oneInXChance(10) && heldWallets[0].storedBy.leader == id()) {
        initiateTransaction(!oneInXChance(4));
    }
}

template <typename ConsensusRequest>
void EyeWitnessPeer<ConsensusRequest>::broadcastTo(EyeWitnessMessage m,
                                                   Neighborhood n) {
    for (const long &i : n.memberIDs) {
        unicastTo(m, i);
    }
}

template <typename ConsensusRequest>
void EyeWitnessPeer<ConsensusRequest>::endOfRound(
    const vector<Peer<EyeWitnessMessage> *> &_peers) {
    if (lastRound()) {
        LogWriter::getTestLog()["transactions"] = json::array();
        LogWriter::getTestLog()["validations"] = json::array();
        LogWriter::getTestLog()["messages"] = json::array();
        for (const auto &genericPeer : _peers) {
            EyeWitnessPeer<ConsensusRequest> *peer =
                dynamic_cast<EyeWitnessPeer<ConsensusRequest> *>(genericPeer);
            std::copy(
                peer->transactions.begin(), peer->transactions.end(),
                std::back_inserter(LogWriter::getTestLog()["transactions"]));
            std::copy(
                peer->validations.begin(), peer->validations.end(),
                std::back_inserter(LogWriter::getTestLog()["validations"]));
            std::copy(peer->messages.begin(), peer->messages.end(),
                      std::back_inserter(LogWriter::getTestLog()["messages"]));
        }
    }
}

template <typename ConsensusRequest>
// Precondition: peer is a leader in their (single) neighborhood
// Postcondition: ConsensusRequest created UNLESS there are no coins to send
// in this neighborhood's wallets
void EyeWitnessPeer<ConsensusRequest>::initiateTransaction(
    bool withinNeighborhood) {
    vector<LocalWallet> haveCoins;
    std::copy_if(heldWallets.begin(), heldWallets.end(),
                 std::back_inserter(haveCoins),
                 [](const LocalWallet &w) { return w.coins.size() > 0; });
    if (haveCoins.size() == 0) {
        return;
    }
    LocalWallet sender = haveCoins[randMod(haveCoins.size())];
    Coin &c = sender.coins[randMod(sender.coins.size())];
    Neighborhood thisNeighborhood = neighborhoods[neighborhoodsForPeers[id()]];
    if (c.history.size() == 0) {
        // hack to allow coins to be processed with no history; should be
        // irrelevant now
        c.history.push_back({{-1, {}}, sender});
    }
    LocalWallet receiver = sender;
    // obtain a receiver that is not the sender
    if (withinNeighborhood) {
        do {
            receiver = heldWallets[randMod(heldWallets.size())];
        } while (receiver.address == sender.address);
    } else {
        // accessing static data to find receiver wallet in other
        // neighborhood - for real transactions, clients would have to tell
        // us about these
        int receiverNeighborhood = neighborhoodID;
        do {
            receiverNeighborhood = randMod(neighborhoodCount);
        } while (receiverNeighborhood == neighborhoodID);
        // i'm assuming that all neighborhoods store at least one wallet
        std::vector<LocalWallet> potential =
            walletsForNeighborhoods[receiverNeighborhood];
        receiver = potential[randMod(potential.size())];
    }
    OngoingTransaction t = {sender, receiver, c, getRound(), -1};
    int seqNum = ++previousSequenceNumber;

    ConsensusContacts newContacts;
    if (withinNeighborhood) {
        newContacts = ConsensusContacts(c, 1);
        ConsensusRequest request(t, newContacts.getValidatorNodeCount(), seqNum,
                                 true);
        localRequests.insert({seqNum, request});
    } else {
        newContacts = ConsensusContacts(c, validatorNeighborhoods);
        ConsensusRequest request(t, newContacts.getValidatorNodeCount(), seqNum,
                                 true);
        superRequests.insert({seqNum, request});
        contacts.insert({seqNum, newContacts});
    }

    transactions.push_back(
        {{"round", getRound()},
         {"seqNum", seqNum},
         {"validatorCount", newContacts.getValidatorNodeCount()}});

    std::cout << "initiated transaction " << seqNum << " in round "
              << getRound() << ". Transferring "
              << "coin " << c.id << " from wallet " << sender.address
              << " to wallet " << receiver.address << ". ";

    if (!withinNeighborhood) {
        std::cout << "neighborhoods ";
        for (const Neighborhood &n : newContacts.getOthers(id())) {
            std::cout << n.id << " ";
        }
        std::cout << "will participate in validation";
    }
    std::cout << std::endl;
}
} // namespace quantas
#endif /* EyeWitnessPeer_hpp */
