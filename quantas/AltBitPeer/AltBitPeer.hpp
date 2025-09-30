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
#include <vector>

namespace quantas {
	using std::vector;

	class AltBitPeer : public Peer {
	public:
		// methods that must be defined when deriving from Peer
		AltBitPeer(NetworkInterface*);
		AltBitPeer(const AltBitPeer& rhs);
		~AltBitPeer();

		// perform one step of the Algorithm with the messages in inStream
		void                 performComputation() override;
		// perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
		void                 endOfRound(vector<Peer*>& _peers) override;

		void 				 initParameters(const std::vector<Peer*>& _peers, json parameters);

		// the id of the next transaction to submit
		int currentTransaction = 1;
		// number of requests satisfied
		int requestsSatisfied = 0;
		// number of messages sent
		int messagesSent = 0;
		// message number
		int ns = 1;
		// time till resend a message
		int timeOutRate = 4;
		// last round a message was sent
		int previousMessageRound = 0;
		// sends a direct message
		void				  sendMessage(interfaceId peer, json message);
		// submitTrans creates a transaction
		void                  submitTrans(int tranID);
	};
}
#endif /* AltBitPeer_hpp */