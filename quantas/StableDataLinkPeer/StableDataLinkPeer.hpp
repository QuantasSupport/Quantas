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

#include <vector>

namespace quantas {

using std::vector;
using nlohmann::json;

class StableDataLinkPeer : public Peer {
public:
	StableDataLinkPeer(NetworkInterface*);
	StableDataLinkPeer(const StableDataLinkPeer& rhs);
	~StableDataLinkPeer() override;

	void performComputation() override;
	void endOfRound(vector<Peer*>& _peers) override;

		int requestsSatisfied = 0;
		int messagesSent = 0;
		int timeOutRate = 4;
		int previousMessageRound = 0;
		bool alive = true;

private:
		void handleAck(int messageNum);
		void handleData(int messageNum);
		void sendMessage(interfaceId peer, const json& message);
		void submitTrans();
		void sendData(int messageNum);
		void resendData();
		void sendAck(int messageNum);
		interfaceId partnerId() const;

		int nextMessageNum = 1;
		int lastSentMessageNum = 0;
		int lastDeliveredMessageNum = 0;
		bool awaitingAck = false;
	};
}
#endif /* StableDataLinkPeer_hpp */
