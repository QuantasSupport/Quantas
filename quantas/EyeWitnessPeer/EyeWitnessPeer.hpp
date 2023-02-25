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
#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas
{
    struct Neighborhood
    {
        std::unordered_set<long> memberIDs;
    };

    struct Wallet
    {
        int address;
        Neighborhood storedBy;
    };

    struct Transaction
    {
        Wallet sender;
        Wallet receiver;
    };

    // messages used by StateChangeRequest to acheive consensus
    // TODO: make this a thing
    struct EyeWitnessMessage
    {
        Transaction &trans; // the transaction id
        int sequenceNum = -1;
        string messageType = ""; // phase for PBFT
        int roundSubmitted;
    };

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
        PBFTRequest(Transaction t, int neighbors, int seq)
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
            return randMod(10) == 0;
        }

    protected:
        Transaction transaction;
        int neighborhoodSize;
        int sequenceNumber;
        static int numbersUsed;
        string status = "pre-prepare";
    };

    class EyeWitnessPeer : public Peer<EyeWitnessMessage>
    {
    public:
        // methods that must be defined when deriving from Peer
        EyeWitnessPeer(long);
        EyeWitnessPeer(const EyeWitnessPeer &rhs);
        ~EyeWitnessPeer();

        // perform one step of the algorithm with the messages in inStream
        void performComputation() override;
        
        // perform any calculations needed at the end of a round such as
        // determining the throughput (only ran once, not for every peer)
        void endOfRound(const vector<Peer<EyeWitnessMessage> *> &_peers) override;

        void broadcastTo(EyeWitnessMessage, Neighborhood);

    private:
        std::unordered_map<int, StateChangeRequest> localRequests;
        std::unordered_map<int, StateChangeRequest> superRequests;
    };

    Simulation<quantas::EyeWitnessMessage, quantas::EyeWitnessPeer> *generateSim();
}
#endif /* EyeWitnessPeer_hpp */
