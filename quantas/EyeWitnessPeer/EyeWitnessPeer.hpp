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
    bool operator!=(const WalletLocation &rhs) { return !(*this == rhs); }
};

struct TransactionRecord {
    // maybe store something symbolizing the sender's signature?

    WalletLocation sender;
    WalletLocation receiver;
};

struct Coin {
    // id needs to be unique across all coins across all peers; use static
    // counter variable in peer class? or uuid generator.
    int id = -1;
    vector<TransactionRecord> history;
    bool operator==(const Coin &rhs) { return id == rhs.id; }
};

struct LocalWallet : public WalletLocation {
    // we do not care about these things for wallets that are only mentioned
    // in transactions, but we need to store them for local wallets

    // coins currently held in this wallet
    std::unordered_map<int, Coin> coins;
    // coins held by this wallet in the past; these records are used for
    // validation when participating in consensus later. this is not a
    // comprehensive list because every coin only needs to be stored once by
    // each peer, so this is missing coins that have simply moved to other
    // wallets stored by this peer at any point
    std::unordered_map<int, Coin> pastCoins;
    void remove(Coin target, bool alsoFromHistory = true) {
        coins.erase(target.id);
        if (alsoFromHistory) {
            pastCoins.erase(target.id);
        }
    }
    // moves a Coin from `coins` to `pastCoins`. assumes that the version of the
    // Coin passed as an argument has the most recent history and should be
    // stored
    Coin *moveToHistory(Coin target) {
        remove(target);
        auto insertion = pastCoins.insert({target.id, target});
        return &(insertion.first->second);
    }
    // adds a Coin to `coins`
    Coin *add(Coin target) {
        auto insertion = coins.insert({target.id, target});
        return &(insertion.first->second);
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
    // this is a PBFT phase or "post-commit"; post-commit informs a coin
    // receiver of a transaction validation
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
    void sabotageWithOwn(Neighborhood n) {
        if (std::find(participants.begin(), participants.end(), n) ==
            participants.end()) {
            participants.pop_back();
            participants.push_back(n);
        }
    }
    // Precondition: peerID is an id in one of the neighborhoods
    Neighborhood getOwn(long peerID) {
        auto own = std::find_if(
            participants.begin(), participants.end(),
            [peerID](Neighborhood &n) { return n.memberIDs.count(peerID) > 0; }
        );
        return *own;
    }
    std::vector<Neighborhood> getOthers(long peerID) {
        std::vector<Neighborhood> others;
        std::copy_if(
            participants.begin(), participants.end(),
            std::back_inserter(others),
            [peerID](Neighborhood &n) { return n.memberIDs.count(peerID) == 0; }
        );
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

    void initParameters(const vector<Peer *> &_peers, json parameters) override;

    // perform one step of the algorithm with the messages in inStream
    void performComputation() override;

    void endOfRound(const vector<Peer<EyeWitnessMessage> *> &_peers) override;

    void broadcastTo(EyeWitnessMessage, Neighborhood);
    void initiateTransaction(bool withinNeighborhood = true);

  private:
    bool validateTransaction(OngoingTransaction);
    bool transactionInProgressFor(Coin);
    ConsensusContacts chooseContactsFor(OngoingTransaction, bool);

    std::unordered_map<int, ConsensusRequest> localRequests;
    std::unordered_map<int, ConsensusRequest> superRequests;
    std::vector<ConsensusRequest> finishedRequests;
    // std::multimap<int, PBFTRequest> recievedRequests;
    // TODO: this should also probably be a container with lookup based on id
    // (address)
    std::vector<LocalWallet> heldWallets;
    int neighborhoodID;
    std::unordered_map<int, ConsensusContacts> contacts;
    bool corrupt = false;

    // maps coin ids to this peer's record of the coin with that id.
    std::unordered_map<int, Coin *> coinDB;

    // counts how many post-commit messages have been received for a given
    // sequence number
    std::unordered_multiset<int> receivedPostCommits;

    // logs that are gathered up and sent to the LogWriter all at once in the
    // last endOfRound(); events aren't sent to the LogWriter directly because
    // that is not thread-safe
    std::vector<json> transactions;
    std::vector<json> validations;
    std::vector<json> messages;

    // technically not realistic for this to be known to all nodes
    inline static std::atomic<int> previousSequenceNumber = -1;
    inline static int issuedCoins = 0;
    inline static int byzantineLeader = -1;

    // parameters from input file; read in initParameters
    inline static int maxNeighborhoodSize = -1;
    inline static int neighborhoodCount = -1;
    inline static int validatorNeighborhoods = -1;
    inline static int byzantineRound = -1;
    inline static int submitRate = 10;
    inline static int maliciousNeighborhoods = 0;

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
    const vector<Peer<EyeWitnessMessage> *> &_peers, json parameters
) {
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
        if (!parameters.contains("maliciousNeighborhoods")) {
            std::cout << "WARNING: parameter byzantineRound is set but "
                         "maliciousNeighborhoods is not specified. Defaulting "
                         "to 1 malicious neighborhood\n";
            maliciousNeighborhoods = 1;
        }
    }
    if (parameters.contains("submitRate")) {
        submitRate = parameters["submitRate"];
    }
    if (parameters.contains("maliciousNeighborhoods")) {
        maliciousNeighborhoods = parameters["maliciousNeighborhoods"];
        if (!parameters.contains("byzantineRound")) {
            std::cout
                << "WARNING: parameter maliciousNeighborhoods is set but "
                   "byzantineRound is not specified. Defaulting "
                   "to creating the malicious neighborhoods at round 10\n";
            byzantineRound = 10;
        }
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
        int actualNeighborhoodSize = std::min(
            maxNeighborhoodSize,
            static_cast<int>(_peers.size()) - i * maxNeighborhoodSize
        );
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
    int fakeHistoryLength = validatorNeighborhoods;
    int neighborhood = 0;
    auto rng = std::default_random_engine{};
    // temporarily maps wallet addresses to past coins
    std::map<int, std::vector<Coin>> pastCoins;
    for (auto &wallets : walletsForNeighborhoods) {
        for (auto &wallet : wallets) {
            for (int i = 0; i < coinsPerWallet; ++i) {
                Coin c = {issuedCoins++, {}};

                if (all.size() < fakeHistoryLength) {
                    std::cout << "Not enough neighborhoods." << std::endl;
                }
                // establish coin history
                WalletLocation sender = all[randMod(all.size())];
                // options is used to pick unique locations for the history
                std::vector<int> options(all.size(), 0);
                for (int j = 0; j < all.size(); j++) {
                    options[j] = j;
                }
                std::shuffle(options.begin(), options.end(), rng);
                for (int j = 0; j < fakeHistoryLength - 1; j++) {
                    WalletLocation reciever = all[options.back()];
                    options.pop_back();
                    c.history.push_back({sender, reciever});
                    sender = c.history.back().receiver;
                }
                c.history.push_back(TransactionRecord{
                    fakeHistoryLength > 1 ? c.history.back().receiver
                                          : all[options.back()],
                    wallet});

                // this is overkill since every neighborhood only needs the coin
                // once
                for (const auto &t : c.history) {
                    pastCoins[t.sender.address].push_back(c);
                }
                wallet.coins.insert({c.id, c});
            }
        }
        ++neighborhood;
    }

    for (auto &n : _peers) {
        // it would be less weird to set these in the constructor, but
        // because this function (initParameters) is not static, at least
        // one Peer needs to be constructed *before* this function can run
        EyeWitnessPeer *e = dynamic_cast<EyeWitnessPeer *>(n);
        assert(e); // cast should have succeeded
        e->neighborhoodID = neighborhoodsForPeers[e->id()];
        e->heldWallets = walletsForNeighborhoods[e->neighborhoodID];
        for (LocalWallet &w : e->heldWallets) {
            for (auto &c : w.coins) {
                e->coinDB[c.first] = &c.second;
            }
            for (Coin &c : pastCoins[w.address]) {
                auto insertion = w.pastCoins.insert({c.id, c});
                e->coinDB[c.id] = &(insertion.first->second);
            }
        }
    }

    // make sure there is no pointer re-use
    std::unordered_set<long> ptrs;
    for (auto &n : _peers) {
        EyeWitnessPeer *e = dynamic_cast<EyeWitnessPeer *>(n);
        assert(e); // cast should have succeeded
        for (auto coinPair : e->coinDB) {
            assert(ptrs.count((long)coinPair.second) == 0);
            ptrs.insert((long)coinPair.second);
        }
    }
}

template <typename ConsensusRequest>
EyeWitnessPeer<ConsensusRequest>::EyeWitnessPeer(long id)
    : Peer<EyeWitnessMessage>(id) {}

template <typename ConsensusRequest>
bool EyeWitnessPeer<ConsensusRequest>::validateTransaction(OngoingTransaction t
) {
    // for now, just checking to see if the coin was last known to be in the
    // sending neighborhood (because internal transactions in the neighborhood
    // aren't visible and need signatures to be checked)
    return corrupt || (t.sender.storedBy.id ==
                       coinDB[t.coin.id]->history.back().receiver.storedBy.id);
}

template <typename ConsensusRequest>
void EyeWitnessPeer<ConsensusRequest>::performComputation() {
    // retrieve messages for requests that are in progress:
    while (!inStreamEmpty()) {
        Packet<EyeWitnessMessage> p = popInStream();
        EyeWitnessMessage message = p.getMessage();
        int seqNum = message.sequenceNum;
        typename std::unordered_map<int, ConsensusRequest>::iterator s;
        if ((s = localRequests.find(seqNum)) != localRequests.end()) {
            if (!validateTransaction(message.trans)) {
                std::cout << "invalid transaction message ignored\n";
                continue;
            }
            s->second.addToConsensus(message);
        } else if ((s = superRequests.find(seqNum)) != superRequests.end()) {
            if (!validateTransaction(message.trans)) {
                std::cout << "invalid transaction message ignored\n";
                continue;
            }
            s->second.addToConsensus(message);
        } else {
            // message about previously unknown transaction
            if (message.messageType == "pre-prepare") {
                if (!validateTransaction(message.trans)) {
                    std::cout << "invalid transaction message ignored\n";
                    continue;
                }
                if (coinDB.count(message.trans.coin.id) > 0) {
                    if (message.trans.sender.storedBy ==
                        message.trans.receiver.storedBy) {
                        ConsensusContacts newContacts =
                            chooseContactsFor(message.trans, true);
                        ConsensusRequest request(
                            message.trans, newContacts.getValidatorNodeCount(),
                            message.sequenceNum, false
                        );
                        request.addToConsensus(message);
                        contacts.insert({message.sequenceNum, newContacts});
                        localRequests.insert(
                            {request.getSequenceNumber(), request}
                        );
                    } else {
                        ConsensusContacts newContacts =
                            chooseContactsFor(message.trans, false);
                        ConsensusRequest request(
                            message.trans, newContacts.getValidatorNodeCount(),
                            message.sequenceNum, false
                        );
                        request.addToConsensus(message);
                        contacts.insert({message.sequenceNum, newContacts});
                        superRequests.insert(
                            {request.getSequenceNumber(), request}
                        );
                    }
                } else {
                    std::cout
                        << "received pre-prepare message about unknown coin\n";
                }
            } else if (message.messageType == "post-commit") {
                // TODO: robustness. should check if coin sender is part of this
                receivedPostCommits.insert(message.sequenceNum);
                if (receivedPostCommits.count(message.sequenceNum) >
                    2. / 3. * validatorNeighborhoods * maxNeighborhoodSize) {
                    auto localReceiver = std::find_if(
                        heldWallets.begin(), heldWallets.end(),
                        [message](LocalWallet w) {
                            return w.address == message.trans.receiver.address;
                        }
                    );
                    coinDB[message.trans.coin.id] =
                        localReceiver->add(message.trans.coin);
                }
            } else {
                std::cout
                    << "received non-pre-prepare, non-post-commit message "
                       "about unknown transaction\n";
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
                auto localSender = std::find_if(
                    heldWallets.begin(), heldWallets.end(),
                    [trans](LocalWallet w) {
                        return w.address == trans.sender.address;
                    }
                );
                auto localReceiver = std::find_if(
                    heldWallets.begin(), heldWallets.end(),
                    [trans](LocalWallet w) {
                        return w.address == trans.receiver.address;
                    }
                );
                if (localReceiver != heldWallets.end() &&
                    localSender != heldWallets.end()) {
                    Coin moving = trans.coin;
                    moving.history.push_back(trans);
                    coinDB[moving.id] = localReceiver->add(moving);
                    localSender->remove(moving);
                }
                finishedRequests.push_back(r);
                validations.push_back(
                    {{"round", getRound()},
                     {"seqNum", r.getSequenceNumber()},
                     {"peer", id()}}
                );
                s = localRequests.erase(s);
            }
        } else {
            while (!r.outboxEmpty()) {
                EyeWitnessMessage m = r.getMessage();
                broadcastTo(m, r.getTransaction().sender.storedBy);
                messages.push_back(
                    {{"round", getRound()},
                     {"transactionType", "local"},
                     {"batchSize", r.getTransaction().sender.storedBy.size()}}
                );
            }
            ++s;
        }
    }

    for (auto s = superRequests.begin(); s != superRequests.end();) {
        ConsensusRequest &r = s->second;
        r.updateConsensus();
        if (r.consensusSucceeded()) {
            int seqNum = r.getSequenceNumber();
            int peerID = id();
            OngoingTransaction trans = r.getTransaction();
            if (trans.sender.storedBy.has(id())) {
                // if we are in the sending committee:
                auto localSender = std::find_if(
                    heldWallets.begin(), heldWallets.end(),
                    [trans](LocalWallet w) {
                        return w.address == trans.sender.address;
                    }
                );
                Coin moving = trans.coin;
                moving.history.push_back(trans);
                coinDB[moving.id] = localSender->moveToHistory(moving);
                assert(coinDB[moving.id]->history.size() > 0);
            } else {
                // if we are a validator:
                if (coinDB.count(trans.coin.id) > 0) {
                    coinDB[trans.coin.id]->history.push_back(trans);
                    EyeWitnessMessage m;
                    m.trans = trans;
                    m.messageType = "post-commit";
                    m.sequenceNum = seqNum;
                    broadcastTo(m, trans.receiver.storedBy);
                    messages.push_back(
                        {{"round", getRound()},
                         {"transactionType", "non-local"},
                         {"batchSize", trans.receiver.storedBy.size()}}
                    );
                } else {
                    std::cout << "reached consensus on an unknown coin?\n";
                }
            }
            finishedRequests.push_back(r);
            validations.push_back(
                {{"round", getRound()},
                 {"seqNum", r.getSequenceNumber()},
                 {"peer", id()}}
            );
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
                                       .getValidatorNodeCount()}}
                );
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
    if ((oneInXChance(submitRate) || corrupt) &&
        heldWallets[0].storedBy.leader == id()) {
        // corrupt nodes always go for out-of-neighborhood transactions
        initiateTransaction(!(corrupt || oneInXChance(4)));
    }
}

template <typename ConsensusRequest>
void EyeWitnessPeer<ConsensusRequest>::broadcastTo(
    EyeWitnessMessage m, Neighborhood n
) {
    for (const long &i : n.memberIDs) {
        unicastTo(m, i);
    }
}

template <typename ConsensusRequest>
void EyeWitnessPeer<ConsensusRequest>::endOfRound(
    const vector<Peer<EyeWitnessMessage> *> &_peers
) {
    const vector<EyeWitnessPeer *> peers =
        reinterpret_cast<vector<EyeWitnessPeer *> const &>(_peers);
    if (lastRound()) {
        LogWriter::getTestLog()["transactions"] = json::array();
        LogWriter::getTestLog()["validations"] = json::array();
        LogWriter::getTestLog()["messages"] = json::array();
        for (const auto &peer : peers) {
            std::copy(
                peer->transactions.begin(), peer->transactions.end(),
                std::back_inserter(LogWriter::getTestLog()["transactions"])
            );
            std::copy(
                peer->validations.begin(), peer->validations.end(),
                std::back_inserter(LogWriter::getTestLog()["validations"])
            );
            std::copy(
                peer->messages.begin(), peer->messages.end(),
                std::back_inserter(LogWriter::getTestLog()["messages"])
            );
        }
    } else if (byzantineRound == getRound()) {
        std::vector<int> shuffledNBHs(neighborhoods.size());
        std::iota(shuffledNBHs.begin(), shuffledNBHs.end(), 0);
        std::shuffle(
            shuffledNBHs.begin(), shuffledNBHs.end(), RANDOM_GENERATOR
        );

        for (int indexIntoShuffle = 0;
             indexIntoShuffle < maliciousNeighborhoods; ++indexIntoShuffle) {
            int corruptNeighborhood = shuffledNBHs[indexIntoShuffle];
            for (int i = 0;
                 i < walletsForNeighborhoods[corruptNeighborhood].size(); i++) {
                LogWriter::getTestLog()["corruptWallets"].push_back(
                    walletsForNeighborhoods[corruptNeighborhood][i].address
                );
            }
            for (int i = 0; i < peers.size(); i++) {
                if (peers[i]->neighborhoodID == corruptNeighborhood) {
                    std::cout << "corrupting " << peers[i]->id() << std::endl;
                    peers[i]->corrupt = true;
                    if (walletsForNeighborhoods[corruptNeighborhood][0]
                            .storedBy.leader == peers[i]->id()) {
                        byzantineLeader = peers[i]->id();
                    }
                }
            }
        }
    }
    std::cout << "completed round " << (getRound() + 1) << "/"
              << (getLastRound() + 1) << "\n";
}

template <typename ConsensusRequest>
bool EyeWitnessPeer<ConsensusRequest>::transactionInProgressFor(Coin c) {
    for (auto &r : localRequests) {
        if (r.second.getTransaction().coin == c) {
            return true;
        }
    }
    for (auto &r : superRequests) {
        if (r.second.getTransaction().coin == c) {
            return true;
        }
    }
    return false;
}

template <typename ConsensusRequest>
ConsensusContacts EyeWitnessPeer<ConsensusRequest>::chooseContactsFor(
    OngoingTransaction t, bool local
) {
    ConsensusContacts result;
    if (local) {
        result = ConsensusContacts(t.coin, 1);
    } else {
        result = ConsensusContacts(t.coin, validatorNeighborhoods);
    }
    if (corrupt) {
        result.sabotageWithOwn(neighborhoods[neighborhoodID]);
    }
    return result;
}

template <typename ConsensusRequest>
// Precondition: peer is a leader in their (single) neighborhood
// Postcondition: ConsensusRequest created UNLESS there are no coins to send
// in this neighborhood's wallets
void EyeWitnessPeer<ConsensusRequest>::initiateTransaction(
    bool withinNeighborhood
) {
    LocalWallet sender;
    Coin c;
    bool honest;

    if (corrupt) {
        vector<LocalWallet> haveCoins;
        std::copy_if(
            heldWallets.begin(), heldWallets.end(),
            std::back_inserter(haveCoins),
            [](const LocalWallet &w) { return w.pastCoins.size() > 0; }
        );
        if (haveCoins.size() != 0) {
            sender = haveCoins[randMod(haveCoins.size())];
            // hacky way to get random element from unordered map
            auto pcIter = std::next(
                sender.pastCoins.begin(), randMod(sender.pastCoins.size())
            );
            c = pcIter->second;
        }
        honest = false;
    }
    if (c.id == -1) {
        vector<LocalWallet> haveCoins;
        std::copy_if(
            heldWallets.begin(), heldWallets.end(),
            std::back_inserter(haveCoins),
            [this](const LocalWallet &w) {
                for (const auto &c : w.coins) {
                    if (!transactionInProgressFor(c.second)) {
                        return true;
                    }
                }
                return false;
            }
        );
        if (haveCoins.size() == 0) {
            return;
        }
        sender = haveCoins[randMod(haveCoins.size())];
        // hacky way to get random element from unordered map
        vector<Coin> availableCoins;
        for (const auto &c : sender.coins) {
            if (!transactionInProgressFor(c.second)) {
                availableCoins.push_back(c.second);
            }
        }
        c = availableCoins[randMod(availableCoins.size())];
        honest = true;
    }

    Neighborhood thisNeighborhood = neighborhoods[neighborhoodsForPeers[id()]];
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
        ConsensusRequest request(
            t, newContacts.getValidatorNodeCount(), seqNum, true
        );
        localRequests.insert({seqNum, request});
    } else {
        newContacts = chooseContactsFor(t, false);
        ConsensusRequest request(
            t, newContacts.getValidatorNodeCount(), seqNum, true
        );
        superRequests.insert({seqNum, request});
        contacts.insert({seqNum, newContacts});
    }

    transactions.push_back(
        {{"round", getRound()},
         {"seqNum", seqNum},
         {"validatorCount", newContacts.getValidatorNodeCount()},
         {"coin", c.id},
         {"sender", sender.address},
         {"honest", honest},
         {"receiver", receiver.address}}
    );
}
} // namespace quantas
#endif /* EyeWitnessPeer_hpp */
