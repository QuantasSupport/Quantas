//
//  KademliaPeer.hpp
//  BlockGuard
//
//  Created by Joseph Oglio on 4/14/22.
//  Copyright © 2022 Kent State University. All rights reserved.
//

#ifndef KademliaPeer_hpp
#define KademliaPeer_hpp

#include "./Common/Peer.hpp"

namespace blockguard {


	struct KademliaMessage {
		long reqId;    // id the request is for
		string binId;  // binary id of reqId
		string action; // options are R, N
		int roundSubmitted;
		int hops = 0; // number of times this message has been echoed
	};

	struct KademliaFinger {
		long Id;			  // id of the finger
		string binId;		  // binary id of a finger
		int group;			  // the level the finger belongs to (binary id difference)
	};
	class KademliaPeer : public Peer<KademliaMessage> {
	public:
		// methods that must be defined when deriving from Peer
		KademliaPeer(long);
		KademliaPeer(const KademliaPeer& rhs);
		~KademliaPeer();

		// perform one step of the Algorithm with the messages in inStream
		void                 performComputation();
		// perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
		void                 endOfRound(const vector<Peer<KademliaMessage>*>& _peers);

		// addintal method that have defulte implementation from Peer but can be overwritten
		void                 log()const { printTo(*_log); };
		ostream& printTo(ostream&)const;
		friend ostream& operator<<         (ostream&, const KademliaPeer&);

		// the id of the next transaction to submit
		static int                      currentTransaction;
		// size of binary ids (must be greater than network size)
		const static int				binaryIdSize = 10;
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
		string getBinaryId(long id);
		// sends a direct message
		void				 sendMessage(long peer, KademliaMessage message);
		// submitTrans creates a transaction
		void                  submitTrans(int tranID);
		// finds node to transmit message to
		long				 findRoute(string binId);
	};
}
#endif /* KademliaPeer_hpp */
