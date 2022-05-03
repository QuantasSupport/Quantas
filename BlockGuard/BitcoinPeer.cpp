//
//  BitcoinPeer.cpp
//  BlockGuard
//
//  Created by Joseph Oglio on 4/14/22.
//  Copyright © 2022 Kent State University. All rights reserved.
//


#include <iostream>
#include "BitcoinPeer.hpp"

namespace blockguard {

	int BitcoinPeer::currentTransaction = 1;

	BitcoinPeer::~BitcoinPeer() {

	}

	BitcoinPeer::BitcoinPeer(const BitcoinPeer& rhs) : Peer<BitcoinMessage>(rhs) {
		_counter = rhs._counter;
	}

	BitcoinPeer::BitcoinPeer(long id) : Peer(id) {
		_counter = 0;
	}

	void BitcoinPeer::performComputation() {
		if (true)
			checkInStrm();

		if (guardSubmitTrans())
			submitTrans(currentTransaction);

		if (guardMineBlock())
			mineBlock();

		// increments round number
		_counter++;
	}

	void BitcoinPeer::endOfRound(const vector<Peer<BitcoinMessage>*>& _peers) {
		const vector<BitcoinPeer*> peers = reinterpret_cast<vector<BitcoinPeer*> const&>(_peers);
		int length = peers[0]->blockChain.size();
		int index = 0;
		for (int i = 0; i < peers.size(); i++) {
			if  (peers[i]->blockChain.size() < length) {
				length = peers[i]->blockChain.size();
				index = i;
			}
		}
		LogWritter::instance()->data["tests"][LogWritter::instance()->getTest()]["throughput"].push_back(length - 1);
		if (_counter == 0) {
			// no blocks are confirmed in the first round
			LogWritter::instance()->data["tests"][LogWritter::instance()->getTest()]["throughput"].push_back(0);
		}
		else {
			int prevIndex = LogWritter::instance()->data["tests"][LogWritter::instance()->getTest()]["throughput"].size() - 1;
			int prevLength = LogWritter::instance()->data["tests"][LogWritter::instance()->getTest()]["throughput"][prevIndex];
			int prevLatency = LogWritter::instance()->data["tests"][LogWritter::instance()->getTest()]["latency"][prevIndex];
			int newBlocks = length - prevLength;
			if (newBlocks > 0) {
				int tipMiner = peers[index]->blockChain[length - 1][0].tipMiner;
				prevLatency += _counter - peers[index]->blockChain[length - 1][0].trans.roundSubmitted;
				for (int i = 2; i <= newBlocks; i++) {
					for (int j = 0; j < peers[index]->blockChain[length - i].size(); j++) {
						if (tipMiner == peers[index]->blockChain[length - i][j].tipMiner) {
							prevLatency += _counter - peers[index]->blockChain[length - i][j].trans.roundSubmitted;
							tipMiner = peers[index]->blockChain[length - i][j].tipMiner;
							break;
						}
					}
				}
				LogWritter::instance()->data["tests"][LogWritter::instance()->getTest()]["latency"].push_back(prevLatency);
			}
		}
	}

	void BitcoinPeer::checkInStrm() {
		while (!inStreamEmpty()) {
			Packet<BitcoinMessage> newMsg = popInStream();
			if (newMsg.getMessage().mined) {
				unlinkedBlocks.push_back(newMsg.getMessage().block);
			}
			else {
				transactions.push_back(newMsg.getMessage().block);
			}
		}

		linkBlocks();
	}

	void BitcoinPeer::linkBlocks() {
		bool linked;
		do {
			linked = false;
			for (int i = 0; i < unlinkedBlocks.size(); i++) {
				BitcoinBlock block = unlinkedBlocks[i];
				if (block.length - 1 < blockChain.size()) {
					for (int j = 0; j < blockChain[block.length - 1].size(); j++) {
						if (blockChain[block.length - 1][j].minerId == block.tipMiner) {
							if (block.length > blockChain.size() - 1)
								blockChain.push_back(vector<BitcoinBlock>());

							blockChain[block.length].push_back(block);
							unlinkedBlocks.erase(unlinkedBlocks.begin() + i);
							linked = true;
							break;
						}
					}
					if (linked)
						break;
				}
			}
		} while (linked);
	}

	bool BitcoinPeer::guardSubmitTrans() {
		return rand() % submitRate == 0;
	}

	void BitcoinPeer::submitTrans(int tranID) {
		BitcoinMessage message;
		message.mined = false;
		message.block.trans.id = tranID;
		message.block.trans.roundSubmitted = _counter;
		broadcast(message);
		currentTransaction++;
	}

	bool BitcoinPeer::guardMineBlock() {
		return rand() % mineRate == 0;
	}

	void BitcoinPeer::mineBlock() {
		BitcoinBlock newBlock = findNextTransaction();
		if (newBlock.trans.id != -1) {
			newBlock.minerId = id();
			newBlock.length = blockChain.size();
			newBlock.tipMiner = blockChain[blockChain.size() - 1][0].minerId;

			blockChain.push_back(vector<BitcoinBlock>());
			blockChain[blockChain.size() - 1].push_back(newBlock);

			BitcoinMessage message;
			message.block = newBlock;
			message.mined = true;
			broadcast(message);
		}
	}

	BitcoinBlock BitcoinPeer::findNextTransaction() {
		BitcoinBlock nextTransaction;
		for (int i = 0; i < transactions.size(); ++i) {
			bool next = true;
			BitcoinBlock block = blockChain[blockChain.size() - 1][0];
			if (transactions[i].trans.id == block.trans.id) {
				continue;
			}
			for (int k = blockChain.size() - 2; k > 0; --k) {
				for (int j = 0; j < blockChain[k].size(); j++) {
					if (blockChain[k][j].minerId == block.tipMiner) {
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

	ostream& BitcoinPeer::printTo(ostream& out)const {
		Peer<BitcoinMessage>::printTo(out);

		out << id() << endl;
		out << "counter:" << _counter << endl;

		return out;
	}

	ostream& operator<< (ostream& out, const BitcoinPeer& peer) {
		peer.printTo(out);
		return out;
	}
}