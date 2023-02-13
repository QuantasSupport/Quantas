/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LinearChordPeer_hpp
#define LinearChordPeer_hpp

#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas {


	struct LinearChordMessage {
		long reqId;
		std::string action; // options are R, N
		int roundSubmitted;
		int hops = 0; // number of times this message has been echoed
	};

	struct LinearChordFinger {
		long Id;
		int roundUpdated = 0; // round the finger was last updated
	};
	class LinearChordPeer : public Peer<LinearChordMessage> {
	public:
		// methods that must be defined when deriving from Peer
		LinearChordPeer(long);
		LinearChordPeer(const LinearChordPeer& rhs);
		~LinearChordPeer();

		// perform one step of the Algorithm with the messages in inStream
		void                 performComputation();
		// perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
		void                 endOfRound(const vector<Peer<LinearChordMessage>*>& _peers);

		// addintal method that have defulte implementation from Peer but can be overwritten
		void                 log()const { printTo(*_log); };
		ostream& printTo(ostream&)const;
		friend ostream& operator<<         (ostream&, const LinearChordPeer&);

		// the id of the next transaction to submit
		static int                      currentTransaction;
		// list of nodes with 'higher' id than current node
		std::vector<LinearChordFinger> successor;
		// list of nodes with 'lower' id than current node
		std::vector<LinearChordFinger> predecessor;
		// number of requests satisfied
		int requestsSatisfied = 0;
		// total hops of all satisfied requests
		int totalHops = 0;
		// latency of satisfied requests
		int latency = 0;
		// redundancy link number
		int redundantSize = 2;
		static int numberOfNodes;
		// status of node
		bool alive = true;
		// sent every x rounds to indicate node is alive
		void                 heartBeat();
		// sends a direct message
		void				 sendMessage(long peer, LinearChordMessage message);
		// submitTrans creates a transaction
		void                  submitTrans(int tranID);
	};

	Simulation<quantas::LinearChordMessage, quantas::LinearChordPeer>* generateSim();
}
#endif /* LinearChordPeer_hpp */
