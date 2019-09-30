//	Contains markpbft peer and message classes
//	Created by Mark Gardner for distributed consensus research
//	9/6/2019
//	


#include "MarkPBFT_peer.hpp"
#include <string>
#include "Common/Peer.hpp"
#include <map>
#include <set>

bool markPBFT_message::operator==(const markPBFT_message& rhs) {
	return(
		client_id == rhs.client_id &&
		creator_id == rhs.creator_id &&
		view == rhs.view &&
		type == rhs.type &&
		operation == rhs.operation &&
		operands == rhs.operands &&
		result == rhs.result
		);
}

// Used to set max wait before deciding peer might be byzantine
void markPBFT_peer::setMaxWait() {
	_maxWait = 0;
	for (auto e : _neighbors)
		if (e.second->getDelayToNeighbor(_id) > _maxWait) {
			_maxWait = e.second->getDelayToNeighbor(_id);

		}
	++_maxWait;
}

void markPBFT_peer::setRoundsToRequest(int a) {
	if (a > 0) {
		_roundsToRequest = a;
		_remainingRoundstoRequest = a;
	}
}

std::ostream& markPBFT_peer::printTo(std::ostream& out) const {

	Peer<markPBFT_message>::printTo(out); out << "\tPrimary:" << _isPrimary
		<< "\tState: " << _state << "\tRound: " << _roundCount << std::endl
		<< "\tCommit Count " << _commitCount << "\tPrepare count " << _prepareCount << "voted" << _voteChange
		<< std::endl << std::endl << std::flush;  return out;

}

void markPBFT_peer::setPrimary(bool status) {

	// Make sure no other primary
	if (status) {
		for (auto e : _neighbors) {
			static_cast<markPBFT_peer*>(e.second)->setPrimary(false);

		}
	}
	resetVote();
	_isPrimary = status;
}

void markPBFT_peer::makeRequest() {

	++_roundCount;
	receive();

	if (isByzantine())
	{
		// Since nodes act as client, will still accept replies since they would not normally go to
		// byzantine node
		_inStream.erase(std::remove_if(_inStream.begin(), _inStream.end(),
			[](Packet<markPBFT_message> a) { return a.getMessage().type != reply; }), _inStream.end());
		if (_inStream.empty())
			return;
	}

	//	Primary behaves as client
	if (isPrimary() && !isByzantine()) {
		++_remainingRoundstoRequest;
		if (_remainingRoundstoRequest >= _roundsToRequest) {
			for (int i = 0; i < _requestPerRound; ++i) {
				markPBFT_message preprepareMSG;
				preprepareMSG.creator_id = _id;
				preprepareMSG.client_id = _id;
				preprepareMSG.type = preprepare;

				std::string msgID = _id + std::to_string(++_messageID);

				for (auto e : _neighbors) {
					Packet<markPBFT_message> outPacket(msgID, e.second->id(), _id);
					outPacket.setBody(preprepareMSG);
					outPacket.setDelay(e.second->getDelayToNeighbor(_id));
					_outStream.push_back(outPacket);
				}

				_preprepareSent.insert(msgID);


				_state = preprepare;
			}
			_remainingRoundstoRequest = 0;
		}
	}

	// View Change

	if (_inStream.empty() && !isByzantine()) {
		++_viewCounter;
		if (_viewCounter >= _maxWait)
			_voteChange = true;
	}
	else
		_viewCounter = 0;

    ////////////////////////////////////////////////////////////
	// Process messages
	////////////////////////////////////////////////////////////


	while (!_inStream.empty()) {


		auto inmsg = _inStream.front();
        _inStream.erase(_inStream.begin());


        // pre-prepare phase 2
		if ((inmsg.getMessage().type == preprepare)) {

			if (_receivedMsgLog[inmsg.id()].find(preprepare) == _receivedMsgLog[inmsg.id()].end())
				_receivedMsgLog[inmsg.id()][preprepare] = 1;
			else
				_state = preprepare;
			markPBFT_message prepareMSG;
			prepareMSG.creator_id = _id;
			prepareMSG.client_id = _id;
			prepareMSG.type = prepare;

			for (auto e : _neighbors) {
				Packet<markPBFT_message> outPacket(inmsg.id(), e.second->id(), _id);
				outPacket.setBody(prepareMSG);

				outPacket.setDelay(e.second->getDelayToNeighbor(_id), e.second->getDelayToNeighbor(_id) - 1);

				_outStream.push_back(outPacket);
			}
			// send message to self, help solve 2f+1
			Packet<markPBFT_message> selfPacket(inmsg.id(), _id, _id);
			selfPacket.setDelay(1, 0);
			selfPacket.setBody(prepareMSG);
			_inStream.push_back(selfPacket);
			_prepareSent.insert(inmsg.id());

		
		}

        // phase 3 prepare
		if (inmsg.getMessage().type == prepare && (_commitSent.find(inmsg.id()) == _commitSent.end())) {
			if (_receivedMsgLog[inmsg.id()].find(prepare) == _receivedMsgLog[inmsg.id()].end())
				_receivedMsgLog[inmsg.id()][prepare] = 1;
			else
				++_receivedMsgLog[inmsg.id()][prepare];

			if ((_receivedMsgLog[inmsg.id()][prepare] >= (int)(2 * (_faultTolerance * _neighbors.size())+1)) &&
				((int)(_faultTolerance * _neighbors.size()) != 0)) {
				_state = prepare;
				markPBFT_message commitMSG;
				commitMSG.creator_id = _id;
				commitMSG.client_id = _id;
				commitMSG.type = commit;
				for (auto e : _neighbors) {
					Packet<markPBFT_message> outPacket(inmsg.id(), e.second->id(), _id);
					outPacket.setBody(commitMSG);
					outPacket.setDelay(e.second->getDelayToNeighbor(_id), e.second->getDelayToNeighbor(_id) - 1);
					_outStream.push_back(outPacket);
				}
				_commitSent.insert(inmsg.id());

			}
		}


        // phase 4 commit
		if (inmsg.getMessage().type == commit && (_replySent.find(inmsg.id()) == _replySent.end())) {
			if (_receivedMsgLog[inmsg.id()].find(commit) == _receivedMsgLog[inmsg.id()].end())
				_receivedMsgLog[inmsg.id()][commit] = 1;
			else
				++_receivedMsgLog[inmsg.id()][commit];



			if ((_receivedMsgLog[inmsg.id()][commit] >= (int)(2 * (_faultTolerance * _neighbors.size())+1)) &&
				((int)(_faultTolerance * _neighbors.size()) != 0)) {
				_state = commit;
				markPBFT_message replyMSG;
				replyMSG.creator_id = _id;
				replyMSG.client_id = _id;
				replyMSG.type = reply;

				// Primary is client, send reply to client/self 
				if (isPrimary()) {
					Packet<markPBFT_message> outPacket(inmsg.id(), _id, _id);
					outPacket.setBody(replyMSG);
					outPacket.setDelay(1, 0);
					_inStream.push_back(outPacket);

				}
				else
					for (auto e : _neighbors) {

						if (static_cast<markPBFT_peer*>(e.second)->isPrimary()) {
							Packet<markPBFT_message> outPacket(inmsg.id(), e.second->id(), _id);
							outPacket.setBody(replyMSG);
							outPacket.setDelay(e.second->getDelayToNeighbor(_id), e.second->getDelayToNeighbor(_id) - 1);
							_outStream.push_back(outPacket);
						}
					}
				_replySent.insert(inmsg.id());
			}

		}

        // phase 5 reply to primary (client)
        if (inmsg.getMessage().type == reply && _ledger.find(inmsg.id()) == _ledger.end()) { // ledger is the list of messages gotten from other peers to this one

            // Add count to replylog, if not increment it

            if (_receivedMsgLog[inmsg.id()].find(reply) == _receivedMsgLog[inmsg.id()].end())
                _receivedMsgLog[inmsg.id()][reply] = 1; // counting votes as replies
            else
                ++_receivedMsgLog[inmsg.id()][reply];


            if ((_receivedMsgLog[inmsg.id()][reply] >= (int)(2 * (_faultTolerance * _neighbors.size())+1)) &&
                ((int)(_faultTolerance * _neighbors.size()) != 0)) {
                _ledger.insert(std::make_pair(inmsg.id(), _roundCount)); //

            }

        }

	}
}

void markPBFT_peer::makeRequest(markPBFT_message requestMSG) {
	if (requestMSG.requestGoal == _shard) {}
	// Create message to primary or self if primary
	std::string toID;

	if (isPrimary()&&requestMSG.requestGoal == _shard)
		toID = _id;

	std::map<std::string, Peer<markPBFT_message>*>::iterator targetPeer;
	if (requestMSG.requestGoal == _shard && !isPrimary()) {
		targetPeer = std::find_if(_neighbors.begin(), _neighbors.end(), [&](std::pair< std::string, Peer<markPBFT_message>*> a) {return static_cast<markPBFT_peer*>(a.second)->isPrimary(); });
		toID = (targetPeer->second)->id();
	}
	else {
		targetPeer = std::find_if(_neighbors.begin(), _neighbors.end(), [&](std::pair< std::string, Peer<markPBFT_message>*> a) {
			return static_cast<markPBFT_peer*>(a.second)->getShard() == requestMSG.requestGoal; });
		toID = (targetPeer->second)->id();
	}
	Packet<markPBFT_message> outPacket(requestMSG.client_id, toID, _id);
	outPacket.setBody(requestMSG);
	if (isPrimary() && requestMSG.requestGoal == _shard)
		outPacket.setDelay(1, 0);
	else
		outPacket.setDelay((targetPeer->second)->getDelayToNeighbor(_id), (targetPeer->second)->getDelayToNeighbor(_id) - 1);
	_outStream.push_back(outPacket);

}