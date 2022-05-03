//
//  StableDataLinkPeer.hpp
//  BlockGuard
//
//  Created by Joseph Oglio on 4/14/22.
//  Copyright © 2022 Kent State University. All rights reserved.
//

#ifndef StableDataLinkPeer_hpp
#define StableDataLinkPeer_hpp

#include "./Common/Peer.hpp"

namespace blockguard {


	struct StableDataLinkMessage {
		string action; // options are ack, data
		int messageNum;
		int roundSubmitted;
	};

	class StableDataLinkPeer : public Peer<StableDataLinkMessage> {
	protected:
		int _counter;

	public:
		// methods that must be defined when deriving from Peer
		StableDataLinkPeer(long);
		StableDataLinkPeer(const StableDataLinkPeer& rhs);
		~StableDataLinkPeer();

		// perform one step of the Algorithm with the messages in inStream
		void                 performComputation();
		// perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
		void                 endOfRound(const vector<Peer<StableDataLinkMessage>*>& _peers);


		// its normally a good idea to make some getters and setters for a peer to enable testing 
		//  of the peers state durring an experment and to get metrics. 
		int                  getCounter()const { return _counter; };
		void                 setCounter(int c) { _counter = c; };

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
		int messageLossDen = 4;
		int messageLossNum = 1;
		int timeOutRate = 4;
		int previousMessageRound = 0;
		// status of node
		bool alive = true;
		// sends a direct message
		void				 sendMessage(long peer, StableDataLinkMessage message);
		// submitTrans creates a transaction
		void                  submitTrans(int tranID);
	};
}
#endif /* StableDataLinkPeer_hpp */
