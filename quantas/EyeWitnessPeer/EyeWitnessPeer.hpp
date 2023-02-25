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
    struct Neighborhood {
        std::unordered_set<long> memberIDs;
    };

    struct Wallet {
        int address;
        Neighborhood storedBy;
    };

    struct Transaction {
        Wallet sender;
        Wallet receiver;
    };

    // messages used by StateChangeRequest to acheive consensus
    // TODO: make this a thing
    struct EyeWitnessMessage
    {
        Transaction& trans; // the transaction id
        int sequenceNum = -1;
        string messageType = ""; // phase for PBFT
        int roundSubmitted;
    };

    class StateChangeRequest
    {
    public:
        StateChangeRequest(){
            sequenceNumber = numbersUsed++;
        }
        virtual bool consensusSucceeded() const = 0;
        virtual void updateConsensus(EyeWitnessMessage) = 0;

    protected:
        string status = "pre-prepare";
        int sequenceNumber;
        vector<EyeWitnessMessage> outbox;
        Transaction transaction;
        static int numbersUsed;
    };

    class PBFTRequest : public StateChangeRequest
    {
        void updateConsensus(EyeWitnessMessage c)
        {
            // TODO: put real code here
            return;
        }
        bool consensusSucceeded()
        {
            // TODO: put real code here
            return randMod(10) == 0;
        }
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
