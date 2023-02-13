/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef StableDataLinkPeer_hpp
#define StableDataLinkPeer_hpp

#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas {


	struct StableDataLinkMessage {
		string action; // options are ack, data
		int messageNum;
		int roundSubmitted;
	};

	class StableDataLinkPeer : public Peer<StableDataLinkMessage> {
	public:
		// methods that must be defined when deriving from Peer
		StableDataLinkPeer(long);
		StableDataLinkPeer(const StableDataLinkPeer& rhs);
		~StableDataLinkPeer();

		// perform one step of the Algorithm with the messages in inStream
		void                 performComputation();
		// perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
		void                 endOfRound(const vector<Peer<StableDataLinkMessage>*>& _peers);

		// addintal method that have defulte implementation from Peer but can be overwritten
		void                 log()const { printTo(*_log); };
		ostream& printTo(ostream&)const;
		friend ostream& operator<<         (ostream&, const StableDataLinkPeer&);

		// the id of the next transaction to submit
		static int                      currentTransaction;
		// channel size (non fifo channels not implemented channel size limit not implemented)
		int c = 1;
		// number of requests satisfied
		int requestsSatisfied = 0;
		// number of messages sent
		int messagesSent = 0;
		// number of copies recieved
		int ack = 0;
		// num / den = likelyhood of message getting lost
		int messageLossNum = 0;
		int messageLossDen = 1;
		int timeOutRate = 4;
		int previousMessageRound = 0;
		// status of node
		bool alive = true;
		// sends a direct message
		void				 sendMessage(long peer, StableDataLinkMessage message);
		// submitTrans creates a transaction
		void                  submitTrans(int tranID);
	};

	Simulation<quantas::StableDataLinkMessage, quantas::StableDataLinkPeer>* generateSim();
}
#endif /* StableDataLinkPeer_hpp */
