/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef EyeWitnessPeer_hpp
#define EyeWitnessPeer_hpp

#include <deque>
#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas
{

    // parent class for different types of messages sent by this algorithm
    struct EyeWitnessPeerMessage
    {
        int Id = -1;
    };

    // messages used by StateChangeRequest
    // TODO: make this a thing
    struct ConsensusMessage : public EyeWitnessPeerMessage
    {
        int trans = -1; // the transaction id
        int sequenceNum = -1;
        string messageType = ""; // phase
        int roundSubmitted;
    };

    class StateChangeRequest
    {
    public:
        virtual bool consensusSucceeded() const = 0;
        virtual void updateConsensus(ConsensusMessage) = 0;

    protected:
        string status = "pre-prepare";
        int sequenceNumber;
        vector<ConsensusMessage> outbox;
        // transaction
    };

    class PBFTRequest : public StateChangeRequest
    {
        bool consensusSucceeded()
        {
            // TODO: put real code here
            return randMod(10) == 0;
        }
        void updateConsensus(ConsensusMessage c)
        {
            // TODO: put real code here
            return;
        }
        
        // logging methods
        ostream &printTo(ostream &) const;
        friend ostream &operator<<(ostream &, const EyeWitnessPeer &);

        // checkInStrm loops through the in stream adding messsages to receivedMessages or transactions
        void checkInStrm();
        // checkContents loops through the receivedMessages attempting to advance the status of consensus
        void checkContents();
        // submitTrans creates a transaction and broadcasts it to everyone
        void submitTrans(int tranID);
    };

    class EyeWitnessPeer : public Peer<EyeWitnessPeerMessage>
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
        void endOfRound(const vector<Peer<EyeWitnessPeerMessage> *> &_peers) override;
    };

    Simulation<quantas::EyeWitnessPeerMessage, quantas::EyeWitnessPeer> *generateSim();
}
#endif /* EyeWitnessPeer_hpp */
