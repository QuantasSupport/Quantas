/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KademliaPeer_hpp
#define KademliaPeer_hpp

#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas {


	struct KademliaMessage {
		interfaceId reqId;    // id the request is for
		string binId;  // binary id of reqId
		string action; // options are R, N
		int roundSubmitted;
		int hops = 0; // number of times this message has been echoed
	};

	struct KademliaFinger {
		interfaceId Id;			  // id of the finger
		string binId;		  // binary id of a finger
		int group;			  // the level the finger belongs to (binary id difference)
	};
	class KademliaPeer : public Peer<KademliaMessage> {
	public:
		// methods that must be defined when deriving from Peer
		KademliaPeer(interfaceId);
		KademliaPeer(const KademliaPeer& rhs);
		~KademliaPeer();

		// perform one step of the Algorithm with the messages in inStream
		void                 performComputation();
		// perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
		void                 endOfRound(const vector<Peer<KademliaMessage>*>& _peers);
		// the id of the next transaction to submit
		static int currentTransaction;
		// size of binary ids
		int	binaryIdSize;
		// list of nodes list of nodes in different trees than current node
		vector<KademliaFinger> fingers;
		// number of requests satisfied
		int requestsSatisfied = 0;
		// total hops of all satisfied requests
		int totalHops = 0;
		// latency of satisfied requests
		int latency = 0;
		// status of node
		bool alive = true;
		// the binaryId of a node
		string binaryId = "-1";
		// determines the binary Id of a node
		string getBinaryId(interfaceId id);
		// sends a direct message
		void				 sendMessage(interfaceId peer, KademliaMessage message);
		// submitTrans creates a transaction
		void                  submitTrans(int tranID);
		// finds node to transmit message to
		interfaceId				 findRoute(string binId);
	};

	Simulation<quantas::KademliaMessage, quantas::KademliaPeer>* generateSim();
}
#endif /* KademliaPeer_hpp */
