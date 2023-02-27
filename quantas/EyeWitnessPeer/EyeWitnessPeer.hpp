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
        // ¯\_( ͡° ͜ʖ ͡°)_/¯
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
    };

    struct OngoingTransaction: public TransactionRecord {
        // we just need this to verify history for transactions we are currently
        // validating

        Coin coin;
    };

    struct EyeWitnessMessage
    {
        OngoingTransaction trans;
        int sequenceNum = -1;
        // phase for PBFT. i think there should also be an "announcement"
        // message type for sending a transaction from a peer that just
        // "received" it to other neighborhoods that will also need to come to
        // consensus on it. kind of like the request from the client in classic
        // PBFT
        string messageType = "";
        int roundSubmitted;
    };

    // =============== implementation of underlying consensus algorithm ===============

    class StateChangeRequest
    {
    public:
        virtual bool consensusSucceeded() const = 0;
        virtual void updateConsensus(EyeWitnessMessage) = 0;
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
        PBFTRequest(OngoingTransaction t, int neighbors, int seq)
            : transaction(t), neighborhoodSize(neighbors), sequenceNumber(seq)
        {
        }
        void updateConsensus(EyeWitnessMessage c) override
        {
            // TODO: put real code here
            return;
        }
        bool consensusSucceeded() const override
        {
            // TODO: put real code here
            return oneInXChance(10);
        }

    protected:
        OngoingTransaction transaction;
        int neighborhoodSize;
        int sequenceNumber;
        string status = "pre-prepare";
    };

    // ================= peer class that participates directly in the network =================

    class EyeWitnessPeer : public Peer<EyeWitnessMessage>
    {
    public:
        EyeWitnessPeer();
        
        // methods that must be defined when deriving from Peer

        EyeWitnessPeer(long);
        EyeWitnessPeer(const EyeWitnessPeer &rhs);
        ~EyeWitnessPeer();

        void initParameters (const vector<Peer*>& _peers, json parameters) override;

        // perform one step of the algorithm with the messages in inStream
        void performComputation() override;

        // perform any calculations needed at the end of a round such as
        // determining the throughput (only ran once, not for every peer)
        void endOfRound(const vector<Peer<EyeWitnessMessage> *> &_peers) override;

        void broadcastTo(EyeWitnessMessage, Neighborhood);
        void initiateTransaction(bool withinNeighborhood);

    private:
        std::unordered_map<int, StateChangeRequest> localRequests;
        std::unordered_map<int, StateChangeRequest> superRequests;
        std::multimap<int, PBFTRequest> recievedRequests; 
        std::vector<LocalWallet> heldWallets;
        static int issuedCoins;
        static int neighborhoodSize;
    };

    Simulation<quantas::EyeWitnessMessage, quantas::EyeWitnessPeer> *generateSim();
}
#endif /* EyeWitnessPeer_hpp */
