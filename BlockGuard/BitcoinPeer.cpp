//
//  BitcoinPeer.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/15/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include "BitcoinPeer.hpp"
#include "./Common/Peer.hpp"
#include "./Common/Packet.hpp"
#include <iostream>

namespace blockguard {

	using std::cout;
	using std::to_string;
	using std::ostream;
	using std::string;
	using std::endl;

	int BitcoinPeer::currentTransaction = 1;

	BitcoinPeer::~BitcoinPeer() {

	}

	BitcoinPeer::BitcoinPeer(const BitcoinPeer& rhs) : Peer<BitcoinMessage>(rhs) {
		_counter = rhs._counter;
		_numberOfMessagesSent = rhs._numberOfMessagesSent;
	}

	BitcoinPeer::BitcoinPeer(long id) : Peer(id) {
		_counter = 0;
		_numberOfMessagesSent = 0;
	}

	void BitcoinPeer::performComputation() {
		if (true)
			checkInStrm();

		if (guardSubmitTrans())
			submitTrans(currentTransaction);

		if (guardMineBlock())
			mineBlock();

		_counter++;
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
				Block block = unlinkedBlocks[i];
				if (block.length - 1 < blockChain.size()) {
					for (int j = 0; j < blockChain[block.length - 1].size(); j++) {
						if (blockChain[block.length - 1][j].minerId == block.tipMiner) {
							if (block.length > blockChain.size() - 1)
								blockChain.push_back(vector<Block>());
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
		message.block.trans = tranID;
		broadcast(message);
		currentTransaction++;
	}

	bool BitcoinPeer::guardMineBlock() {
		return rand() % mineRate == 0;
	}

	void BitcoinPeer::mineBlock() {
		Block newBlock = findNextTransaction();
		if (newBlock.trans != -1) {
			newBlock.minerId = id();
			newBlock.length = blockChain.size();
			newBlock.tipMiner = blockChain[blockChain.size() - 1][0].minerId;

			blockChain.push_back(vector<Block>());
			blockChain[blockChain.size() - 1].push_back(newBlock);

			BitcoinMessage message;
			message.block = newBlock;
			message.mined = true;
			broadcast(message);
		}
	}

	Block BitcoinPeer::findNextTransaction() {
		Block nextTransaction;
		for (int i = 0; i < transactions.size(); ++i) {
			bool next = true;
			Block block = blockChain[blockChain.size() - 1][0];
			if (transactions[i].trans == block.trans) {
				continue;
			}
			for (int k = blockChain.size() - 2; k > 0; --k) {
				for (int j = 0; j < blockChain[k].size(); j++) {
					if (blockChain[k][j].minerId == block.tipMiner) {
						block = blockChain[k][j];
						if (transactions[i].trans == block.trans)
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