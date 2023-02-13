/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef AltBitPeer_hpp
#define AltBitPeer_hpp

#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas {


	struct AltBitMessage {
		string action; // options are ack, data
		int messageNum;
		int roundSubmitted;
	};

	class AltBitPeer : public Peer<AltBitMessage> {
	public:
		// methods that must be defined when deriving from Peer
		AltBitPeer(long);
		AltBitPeer(const AltBitPeer& rhs);
		~AltBitPeer();

		// perform one step of the Algorithm with the messages in inStream
		void                 performComputation();
		// perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
		void                 endOfRound(const vector<Peer<AltBitMessage>*>& _peers);

		// addintal method that have defulte implementation from Peer but can be overwritten
		void                 log()const { printTo(*_log); };
		ostream& printTo(ostream&)const;

		// the id of the next transaction to submit
		static int                      currentTransaction;
		// number of requests satisfied
		int requestsSatisfied = 0;
		// number of messages sent
		int messagesSent = 0;
		// message number
		int ns = 1;
		// num / den = likelyhood of message getting lost
		int messageLossNum = 0;
		int messageLossDen = 1;
		int timeOutRate = 4;
		int previousMessageRound = 0;
		// status of node
		bool alive = true;
		// sends a direct message
		void				 sendMessage(long peer, AltBitMessage message);
		// submitTrans creates a transaction
		void                  submitTrans(int tranID);
	};

	Simulation<quantas::AltBitMessage, quantas::AltBitPeer>* generateSim();
}
#endif /* AltBitPeer_hpp */