//
//  LinearChordPeer.hpp
//  BlockGuard
//
//  Created by Joseph Oglio on 4/14/22.
//  Copyright © 2022 Kent State University. All rights reserved.
//

#ifndef LinearChordPeer_hpp
#define LinearChordPeer_hpp

#include "./Common/Peer.hpp"

namespace blockguard {


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
}
#endif /* LinearChordPeer_hpp */
