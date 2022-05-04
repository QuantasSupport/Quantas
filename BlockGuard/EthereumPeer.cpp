//
//  EthereumPeer.cpp
//  BlockGuard
//
//  Created by Joseph Oglio on 4/14/22.
//  Copyright © 2022 Kent State University. All rights reserved.
//

#include <iostream>
#include "EthereumPeer.hpp"

namespace blockguard {

	int EthereumPeer::currentTransaction = 1;

	EthereumPeer::~EthereumPeer() {

	}

	EthereumPeer::EthereumPeer(const EthereumPeer& rhs) : Peer<EthereumPeerMessage>(rhs) {
		
	}

	EthereumPeer::EthereumPeer(long id) : Peer(id) {
		
	}

	void EthereumPeer::performComputation() {
		if (true)
			checkInStrm();

		if (guardSubmitTrans())
			submitTrans(currentTransaction);

		if (guardMineBlock())
			mineBlock();
	}

	void EthereumPeer::endOfRound(const vector<Peer<EthereumPeerMessage>*>& _peers) {
		const vector<EthereumPeer*> peers = reinterpret_cast<vector<EthereumPeer*> const&>(_peers);
		int length = INT_MAX;
		int index;
		for (int i = 0; i < peers.size(); i++) {
			vector<int> confirmedTrans;
			for (int j = 1; j < peers[i]->blockChain.size(); j++) {
				for (int k = 0; k < peers[i]->blockChain[j].size(); k++) {
					confirmedTrans.push_back(peers[i]->blockChain[j][k].trans.id);
				}
			}
			auto last = std::unique(confirmedTrans.begin(), confirmedTrans.end());
			confirmedTrans.erase(last, confirmedTrans.end());
			if (confirmedTrans.size() < length) {
				// pick the smallest blockchain as "confirmed. Actual confirmed is likely lower but not by much
				length = confirmedTrans.size();
				index = i;
			}
		}
		LogWritter::instance()->data["tests"][LogWritter::instance()->getTest()]["throughput"].push_back(length);
	}

	void EthereumPeer::checkInStrm() {
		while (!inStreamEmpty()) {
			Packet<EthereumPeerMessage> newMsg = popInStream();
			if (newMsg.getMessage().mined) {
				unlinkedBlocks.push_back(newMsg.getMessage().block);
			}
			else {
				transactions.push_back(newMsg.getMessage().block);
			}
		}

		linkBlocks();
	}

	void EthereumPeer::linkBlocks() {
		bool linked;
		do {
			linked = false;
			for (int i = 0; i < unlinkedBlocks.size(); i++) {
				EtherBlock block = unlinkedBlocks[i];
				if (block.length - 1 < blockChain.size()) {
					int tipsfound = 0;
					for (int k = 0; k < block.tipMiners.size(); k++) {
						int pos = block.tipLengths[k];
						int miner = block.tipMiners[k];
						for (int j = 0; j < blockChain[pos].size(); j++) {
							if (blockChain[pos][j].minerId == miner) {
								tipsfound++;
								break;
							}
						}
						if (tipsfound != k + 1)
							break;
					}
					if (tipsfound == block.tipMiners.size()) {
						if (block.length > blockChain.size() - 1)
							blockChain.push_back(vector<EtherBlock>());

						blockChain[block.length].push_back(block);
						unlinkedBlocks.erase(unlinkedBlocks.begin() + i);
						linked = true;
					}
					if (linked)
						break;
				}
			}
		} while (linked);
	}

	bool EthereumPeer::guardSubmitTrans() {
		return rand() % submitRate == 0;
	}

	void EthereumPeer::submitTrans(int tranID) {
		EthereumPeerMessage message;
		message.mined = false;
		message.block.trans.id = tranID;
		message.block.trans.roundSubmitted = getRound();
		broadcast(message);
		currentTransaction++;
	}

	bool EthereumPeer::guardMineBlock() {
		return rand() % mineRate == 0;
	}

	void EthereumPeer::mineBlock() {
		EtherBlock newBlock = findNextTransaction();
		if (newBlock.trans.id != -1) {
			newBlock.minerId = id();
			newBlock.length = blockChain.size();
			vector<int> ids;
			vector<int> lengths;
			findTips(ids, lengths);
			newBlock.tipMiners = ids;
			newBlock.tipLengths = lengths;

			blockChain.push_back(vector<EtherBlock>());
			blockChain[blockChain.size() - 1].push_back(newBlock);

			EthereumPeerMessage message;
			message.block = newBlock;
			message.mined = true;
			broadcast(message);
		}
	}

	EtherBlock EthereumPeer::findNextTransaction() {
		EtherBlock nextTransaction;
		for (int i = 0; i < transactions.size(); ++i) {
			bool next = true;
			EtherBlock block = blockChain[blockChain.size() - 1][0];
			if (transactions[i].trans.id == block.trans.id) {
				continue;
			}
			for (int k = blockChain.size() - 2; k > 0; --k) {
				for (int j = 0; j < blockChain[k].size(); j++) {
					if (blockChain[k][j].minerId == block.tipMiners[0] && blockChain[k][j].length == block.tipLengths[0]) {
						block = blockChain[k][j];
						if (transactions[i].trans.id == block.trans.id)
							next = false;
						break;
					}
				}
				if (!next)
					break;
			}
			if (next) {
				nextTransaction = transactions[i];
				break;
			}
		}
		return nextTransaction;
	}

	void EthereumPeer::findTips(vector<int> &ids, vector<int> &lengths) {
		for (int i = 0; i < blockChain.size(); i++) {
			for (int j = 0; j < blockChain[i].size(); j++) {
				EtherBlock block = blockChain[i][j];
				ids.push_back(block.minerId);
				lengths.push_back(block.length);
				for (int k = 0; k < block.tipMiners.size(); k++) {
					for (int ii = 0; ii < ids.size(); ii++) {
						if (block.tipMiners[k] == ids[ii] && block.tipLengths[k] == lengths[ii]) {
							ids.erase(ids.begin() + ii);
							lengths.erase(lengths.begin() + ii);
							break;
						}
					}
				}
			}
		}

	}

	ostream& EthereumPeer::printTo(ostream& out)const {
		Peer<EthereumPeerMessage>::printTo(out);

		out << id() << endl;
		out << "counter:" << getRound() << endl;

		return out;
	}

	ostream& operator<< (ostream& out, const EthereumPeer& peer) {
		peer.printTo(out);
		return out;
	}
}