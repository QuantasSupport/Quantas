/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef EyeWitnessPeer_hpp
#define EyeWitnessPeer_hpp

#include <unordered_set>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <iostream>
#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

// unsolved design problems:

// how to find your neighborhood from a specific state change request
// transaction's history; you could be in the history more than once as part of
// multiple neighborhoods

// warning messages: other nodes in neighborhood being like "this person who is
// trying to spend this money has provably signed it away"


namespace quantas
{
    // =============== basic data storage types ===============

    struct Neighborhood
    {
        std::unordered_set<long> memberIDs;
        int leader; // id of the leader of this neighborhood
        bool operator==(const Neighborhood& rhs) { return memberIDs == rhs.memberIDs; }
    };

    struct WalletLocation
    {
        // this is all we need to know for wallets stored in transaction
        // histories

        int address;
        Neighborhood storedBy;
    };

    struct TransactionRecord
    {
        // maybe store something symbolizing the sender's signature?

        WalletLocation sender;
        WalletLocation receiver;
    };

    struct Coin
    {
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
                std::remove_if(
                    coins.begin(), coins.end(), [&target](Coin c){return c.id == target.id;}
                )
            );
            pastCoins.push_back(target);
        }
    };

    struct OngoingTransaction: public TransactionRecord {
        // we just need this to verify history for transactions we are currently
        // validating

        Coin coin;
        int roundSubmitted;
        int roundCompleted;
    };

    struct EyeWitnessMessage
    {
        OngoingTransaction trans;
        // phase for PBFT. i think there should also be an "announcement"
        // message type for sending a transaction from a peer that just
        // "received" it to other neighborhoods that will also need to come to
        // consensus on it. kind of like the request from the client in classic
        // PBFT
        string messageType = "";
        int sequenceNum;
    };

    // recipients of different kinds of messages during consensus. i am assuming
    // for this version of the simulation that each peer is in only one
    // neighborhood
    struct ConsensusContacts
    {
        std::vector<Neighborhood> participants;
        ConsensusContacts(Coin c, int validatorCount) {
            for (int i=0; i<validatorCount && i<c.history.size(); i++) {
                participants.push_back(c.history[c.history.size()-i-1].receiver.storedBy);
            }
        }
        // Precondition: peerID is an id in one of the neighborhoods
        Neighborhood getOwn(long peerID) {
            auto own = std::find_if(
                participants.begin(),
                participants.end(),
                [peerID](Neighborhood& n){return n.memberIDs.count(peerID) > 0;}
            );
            return *own;
        }
        std::vector<Neighborhood> getOthers(long peerID) {
            std::vector<Neighborhood> others;
            std::copy_if(
                participants.begin(),
                participants.end(),
                std::back_inserter(others),
                [peerID](Neighborhood& n){return n.memberIDs.count(peerID) == 0;}
            );
            return others;
        }
    };

    // =============== implementation of underlying consensus algorithm ===============

    class StateChangeRequest
    {
    public:
        virtual bool consensusSucceeded() const = 0;
        virtual void updateConsensus() = 0;
        virtual void addToConsensus(EyeWitnessMessage) = 0;
        bool outboxEmpty() { return outbox.size() == 0; }

        // Precondition: !outboxEmpty()
        EyeWitnessMessage getMessage()
        {
            EyeWitnessMessage m = outbox.back();
            outbox.pop_back();
            return m;
        }

    protected:
        vector<EyeWitnessMessage> outbox;
    };

    class PBFTRequest : public StateChangeRequest
    {
    public:
        PBFTRequest(OngoingTransaction t, int neighbors, int seq, bool amLeader)
            : transaction(t), neighborhoodSize(neighbors), sequenceNum(seq), leader(amLeader)
        {
        }

        void updateConsensus() override;
        void addToConsensus(EyeWitnessMessage c) override;
        bool consensusSucceeded() const override;
        OngoingTransaction getTransaction() { return transaction; }
        int getSequenceNumber() { return sequenceNum; }

    protected:
        OngoingTransaction transaction;
        int neighborhoodSize;
        int sequenceNum;
        string status = "pre-prepare";
        bool leader;
        std::map<string, int> statusCount = {{"pre-prepare", 0}, // Count of the number of times we've seen these messages
              {"prepare", 0}, 
              {"commit", 0}};
    };

    // ================= peer class that participates directly in the network =================
    template <typename ConsensusRequest=PBFTRequest>
    class EyeWitnessPeer : public Peer<EyeWitnessMessage>
    {
    public:        
        // methods that must be defined when deriving from Peer

        EyeWitnessPeer(long id);
        // EyeWitnessPeer(const EyeWitnessPeer &rhs);

        void initParameters (const vector<Peer*>& _peers, json parameters) override;

        // perform one step of the algorithm with the messages in inStream
        void performComputation() override;

        // perform any calculations needed at the end of a round such as
        // determining the throughput (only ran once, not for every peer)
        // void endOfRound(const vector<Peer<EyeWitnessMessage> *> &_peers) override;

        void broadcastTo(EyeWitnessMessage, Neighborhood);
        void initiateTransaction(bool withinNeighborhood=true);

    private:
        std::unordered_map<int, ConsensusRequest> localRequests;
        std::unordered_map<int, ConsensusRequest> superRequests;
        // std::multimap<int, PBFTRequest> recievedRequests;
        // TODO: this should also probably be a container with lookup based on id (address)
        std::vector<LocalWallet> heldWallets;
        int neighborhoodID;
        std::unordered_map<int, ConsensusContacts> contacts;

        // technically not realistic for this to be known to all nodes
        inline static int previousSequenceNumber=-1;
        inline static int issuedCoins=0;
        
        // parameters from input file; read in initParameters
        inline static int maxNeighborhoodSize=-1;
        inline static int neighborhoodCount=-1;
        inline static int validatorNeighborhoods=-1;

        // global neighborhood data; populated as a side effect of initParameters
        inline static std::vector<std::vector<LocalWallet>> walletsForNeighborhoods;
        inline static std::vector<int> neighborhoodSizes;
        inline static std::unordered_map<long, int> peerIDToNeighborhoodIndex;
    };

    Simulation<quantas::EyeWitnessMessage, quantas::EyeWitnessPeer<PBFTRequest>> *generateSim();

	// ================= peer class that participates directly in the network =================

    

    // gives each peer a neighborhood and gives each neighborhood a set of wallets.
    // precondition: "validatorNeighborhoods", "neighborhoodSize", and
    // "walletsPerNeighborhood" are set as parameters in the input file
    // postcondition: neighborhoodSize is set (from the simulation input
    // parameters); neighborhoodCount is set; walletsForNeighborhoods contains a
    // vector of LocalWallets for each neighborhood; peerIDToNeighborhoodIndex
    // maps a peer's id to the index of the neighborhood it's in
    template <typename ConsensusRequest>
    void EyeWitnessPeer<ConsensusRequest>::initParameters(const vector<Peer<EyeWitnessMessage>*>& _peers, json parameters) {
        // divide peers into neighborhoods: if the neighborhood size is 5, then
        // the first 5 peers are placed in a neighborhood, then the next 5
        // peers, and so on. if the total number of peers is not a multiple of
        // 5, an "extra" neighborhood stores the spares. then, each neighborhood
        // gets a set of wallets to take care of.

        validatorNeighborhoods = parameters["validatorNeighborhoods"];
        maxNeighborhoodSize = parameters["neighborhoodSize"];
        const int walletsPerNeighborhood = parameters["walletsPerNeighborhood"];
        neighborhoodCount = ceil(static_cast<float>(_peers.size()) / maxNeighborhoodSize);
        walletsForNeighborhoods.resize(neighborhoodCount);
        vector<WalletLocation> all;
        for (int i=0; i<neighborhoodCount; ++i) {
            Neighborhood newNeighborhood;
            newNeighborhood.leader = i*maxNeighborhoodSize;
            for (int n=0; n<maxNeighborhoodSize&&i*maxNeighborhoodSize+n<_peers.size();++n) {
                const long peerID = i*maxNeighborhoodSize+n;
                newNeighborhood.memberIDs.insert(peerID);
                peerIDToNeighborhoodIndex[peerID] = i;
            }
            for (int j=0; j<walletsPerNeighborhood; ++j) {
                LocalWallet w = {all.size(), newNeighborhood, {}, {}};
                walletsForNeighborhoods[i].push_back(w);
                all.push_back({w.address, newNeighborhood});
            }
        }

        const int coinsPerWallet = 5;
        for (auto& wallets: walletsForNeighborhoods) {
            for (auto& wallet: wallets) {
                for (int i=0; i<coinsPerWallet; ++i) {
                    // a history could be invented here
                    Coin c = {issuedCoins++, {}};
                    wallet.coins.push_back(c);
                }
            }
        }

        for (auto& n: _peers) {
            // it would be less weird to set these in the constructor, but
            // because this function (initParameters) is not static, at least
            // one Peer needs to be constructed *before* this function can run
            EyeWitnessPeer* e = dynamic_cast<EyeWitnessPeer*>(n);
            assert(e);
            e->neighborhoodID = peerIDToNeighborhoodIndex[e->id()];
            e->heldWallets = walletsForNeighborhoods[neighborhoodID];
        }

        std::cout << "Initialized network parameters" << std::endl;
    }

    template<typename ConsensusRequest>
    EyeWitnessPeer<ConsensusRequest>::EyeWitnessPeer(long id): Peer<EyeWitnessMessage>(id) {
        
    }

    template<typename ConsensusRequest>
    void EyeWitnessPeer<ConsensusRequest>::performComputation() {
        // retrieve messages for requests that are in progress:
        while (!inStreamEmpty())
        {
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
                if (message.messageType == "announcement") {
                    // make super ConsensusRequest
                } else if (message.messageType == "pre-prepare") {
                    // assuming that only the sender needs to reach local consensus
                    ConsensusRequest request(
                        message.trans, message.trans.sender.storedBy.memberIDs.size(),  // hmm
                        message.sequenceNum, false
                    );
                    contacts.insert({
                        message.sequenceNum,
                        ConsensusContacts(message.trans.coin, validatorNeighborhoods)
                    });
                }
            }
        }
        // update consensus on in-progress requests:
        for (auto& s: localRequests) {
            s.second.updateConsensus();
            if (s.second.consensusSucceeded()) {
                OngoingTransaction trans = s.second.getTransaction();
                if (trans.receiver.storedBy == trans.sender.storedBy) {
                    // local transaction; commit changes, update wallets
                    // TODO: optimize this search
                    auto localSender = std::find_if(
                        heldWallets.begin(),
                        heldWallets.end(),
                        [trans](LocalWallet w){
                            return w.address == trans.sender.address;
                    });
                    auto localReceiver = std::find_if(
                        heldWallets.begin(),
                        heldWallets.end(),
                        [trans](LocalWallet w){
                            return w.address == trans.receiver.address;
                    });
                    if (localReceiver != heldWallets.end() && localSender != heldWallets.end()) {
                        Coin moving = trans.coin;
                        moving.history.push_back(trans);
                        localSender->moveToHistory(moving);
                        localReceiver->coins.push_back(moving);
                    }
                    // TODO: log transaction latency?
                } else {
                    // nonlocal transaction; start state change request in superRequests
                }
            } else {
                while (!s.second.outboxEmpty()) {
                    broadcastTo(
                        s.second.getMessage(),
                        contacts.at(s.second.getSequenceNumber()).getOwn(id())
                    );
                }
            }
        }
        for (auto& s: superRequests) {
            s.second.updateConsensus();
            if (s.second.consensusSucceeded()) {
                // update internal coin snapshots for transaction; if this
                // is in the receiver neighborhood, add new wallet and then
                // conduct internal transaction
            }else {
                while (!s.second.outboxEmpty()) {
                    // choose message recipients and relay
                    // use contacts
                }
            }
        }
        // assuming non-overlapping neighborhoods, so if we're the leader in one
        // wallet stored by our neighborhood we're a leader in every wallet
        // stored by our neighborhood
        if (oneInXChance(10) && heldWallets[0].storedBy.leader == id()) {
            initiateTransaction();
        }

    }

    template<typename ConsensusRequest>
    void EyeWitnessPeer<ConsensusRequest>::broadcastTo(EyeWitnessMessage m, Neighborhood n) {
        for (const long& i : n.memberIDs) {
            unicastTo(m, i);
        }
    }

    template<typename ConsensusRequest>
    // Precondition: peer is a leader in their (single) neighborhood
    // Postcondition: ConsensusRequest created UNLESS there are no coins to send
    // in this neighborhood's wallets
    void EyeWitnessPeer<ConsensusRequest>::initiateTransaction(bool withinNeighborhood) {
        vector<LocalWallet> haveCoins;
        std::copy_if(heldWallets.begin(), heldWallets.end(), std::back_inserter(haveCoins),
            [](const LocalWallet& w){return w.coins.size() > 0;}
        );
        if (haveCoins.size() == 0) {
            return;
        }
        LocalWallet sender = haveCoins[randMod(haveCoins.size())];
        Coin& c = sender.coins[randMod(sender.coins.size())];
        LocalWallet receiver = sender;
        // obtain a receiver that is not the sender
        do {
            if (withinNeighborhood) {
                receiver = heldWallets[randMod(heldWallets.size())];
            } else {
                // TODO: create cross-neighborhood transaction
            }
        } while (receiver.address == sender.address);
        OngoingTransaction t = {sender, receiver, c};
        int seqNum = ++previousSequenceNumber;
        ConsensusRequest request(
            t, neighborhoodSizes[peerIDToNeighborhoodIndex[id()]], seqNum, true
        );
        localRequests.insert({seqNum, request});
        contacts.insert({seqNum, ConsensusContacts(c, validatorNeighborhoods)});
    }
}
#endif /* EyeWitnessPeer_hpp */
