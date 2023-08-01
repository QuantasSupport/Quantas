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

#ifndef TrailPeer_hpp
#define TrailPeer_hpp

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
    WalletLocation sender;
    WalletLocation receiver;
};

struct Coin {
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

struct TrailMessage {
    OngoingTransaction trans;
    // this is a PBFT phase or "reply"; reply informs a coin
    // receiver of a transaction validation
    string messageType = "";
    int sequenceNum;
    int senderID;
};

// TODO: this is a mess.
// recipients of different kinds of messages during consensus. i am assuming
// for this version of the simulation that each peer is in only one
// neighborhood
struct ConsensusContacts {
    std::vector<Neighborhood> participants;
    ConsensusContacts() = default;
    ConsensusContacts(
        Coin c, int validatorsNeeded, bool skipLastRecipient = false
    ) {
        std::unordered_set<int> uniques;
        for (int i = 0;
             uniques.size() < validatorsNeeded && i < c.history.size(); i++) {
            Neighborhood &prevNeighborhood =
                c.history[c.history.size() - i - (skipLastRecipient ? 2 : 1)]
                    .receiver.storedBy;
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

class PBFTRequest {
  public:
    PBFTRequest(
        OngoingTransaction t, int neighbors, int seq, bool amLeader,
        int ownID = -1
    )
        : transaction(t), consensusPeers(neighbors), sequenceNum(seq),
          leader(amLeader), ownID(ownID), status("pre-prepare") {}

    bool outboxEmpty() { return outbox.size() == 0; }

    // Precondition: !outboxEmpty()
    TrailMessage getMessage() {
        TrailMessage m = outbox.back();
        outbox.pop_back();
        return m;
    }

    void updateConsensus();
    virtual void addToConsensus(TrailMessage c, int sourceID = -1);
    bool consensusSucceeded() const;
    OngoingTransaction getTransaction() const { return transaction; }
    int getSequenceNumber() const { return sequenceNum; }

  protected:
    OngoingTransaction transaction;
    vector<TrailMessage> outbox;

    int consensusPeers;
    int sequenceNum;
    string status;
    bool leader;
    int ownID;
    // Count of the number of times we've seen these messages
    std::map<string, int> statusCount = {
        {"pre-prepare", 0}, {"prepare", 0}, {"commit", 0}};
};

class CrossShardPBFTRequest : public PBFTRequest {
  public:
    CrossShardPBFTRequest(
        OngoingTransaction t, int neighbors, int seq, bool amLeader, int ownID,
        int neighborhoodSize
    )
        : PBFTRequest(t, neighbors, seq, amLeader, ownID) {
        this->neighborhoodSize = neighborhoodSize;
    }
    void addToConsensus(TrailMessage c, int sourceID) override;

  protected:
    int neighborhoodSize;
    std::map<std::pair<string, int>, int> individualMessageCounts;
};

// -- peer class that participates directly in the network --
class TrailPeer : public Peer<TrailMessage> {

  public:
    // methods that must be defined when deriving from Peer

    TrailPeer(long id);

    void initParameters(const vector<Peer *> &_peers, json parameters) override;

    // perform one step of the algorithm with the messages in inStream
    void performComputation() override;

    void endOfRound(const vector<Peer<TrailMessage> *> &_peers) override;

    void broadcastTo(TrailMessage, Neighborhood);
    void initiateTransaction(bool withinNeighborhood = true);
    void initiateRollbacks();

  private:
    bool validateTransaction(OngoingTransaction);
    bool transactionInProgressFor(Coin);
    ConsensusContacts chooseContactsFor(OngoingTransaction, bool);

    std::unordered_map<int, PBFTRequest> localRequests;
    std::unordered_map<int, PBFTRequest> initRequests;
    std::unordered_map<int, CrossShardPBFTRequest> superRequests;
    // stores sequence numbers
    std::unordered_set<int> initsFinished;
    // std::multimap<int, PBFTRequest> recievedRequests;
    // TODO: this should also probably be a container with lookup based on id
    // (address)
    std::vector<LocalWallet> heldWallets;
    int neighborhoodID;
    std::unordered_map<int, ConsensusContacts> contacts;
    bool corrupt = false;

    // maps coin ids to this peer's record of the coin with that id.
    std::unordered_map<int, Coin *> coinDB;

    // counts how many reply messages have been received for a given
    // sequence number
    std::unordered_multiset<int> receivedPostCommits;

    // messages received for a given sequence number before its pre-prepare
    std::unordered_map<int, std::vector<TrailMessage>> errantMessages;

    // logs that are gathered up and sent to the LogWriter all at once in the
    // last endOfRound(); events aren't sent to the LogWriter directly because
    // that is not thread-safe
    std::vector<json> transactions;
    std::vector<json> validations;
    std::vector<json> messages;
    int localMessagesThisRound = 0;
    int superMessagesThisRound = 0;

    // technically not realistic for this to be known to all nodes
    inline static std::atomic<std::int32_t> previousSequenceNumber{-1};
    inline static int issuedCoins = 0;

    // parameters from input file; read in initParameters
    inline static int maxNeighborhoodSize = -1;
    inline static int neighborhoodCount = -1;
    inline static int validatorNeighborhoods = -1;
    inline static int byzantineRound = -1;
    // 1 in x chance for each leader to create a transaction in each round
    inline static int submitRate = -1;
    inline static int maliciousNeighborhoods = 0;
    inline static bool attemptRollback = false;

    // global neighborhood data; populated as a side effect of initParameters
    inline static std::vector<std::vector<LocalWallet>> walletsForNeighborhoods;
    inline static std::unordered_map<long, int> neighborhoodsForPeers;
    inline static std::vector<Neighborhood> neighborhoods;

    // populated in endOfRound if byzantine parameters are set
    inline static std::unordered_set<int> corruptNeighborhoods;
};

Simulation<quantas::TrailMessage, quantas::TrailPeer> *generateSim();
} // namespace quantas
#endif /* TrailPeer_hpp */
