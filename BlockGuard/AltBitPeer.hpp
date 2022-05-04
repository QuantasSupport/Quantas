//
//  AltBitPeer.hpp
//  BlockGuard
//
//  Created by Joseph Oglio on 4/14/22.
//  Copyright © 2022 Kent State University. All rights reserved.
//

#ifndef AltBitPeer_hpp
#define AltBitPeer_hpp

#include "./Common/Peer.hpp"

namespace blockguard {


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
		friend ostream& operator<<         (ostream&, const AltBitPeer&);

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
}
#endif /* AltBitPeer_hpp */