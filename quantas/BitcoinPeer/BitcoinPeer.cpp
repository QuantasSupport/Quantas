/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "BitcoinPeer.hpp"

namespace quantas {

	static bool registerBitcoin = [](){
		PeerRegistry::registerPeerType("BitcoinPeer", 
			[](interfaceId pubId){ return new BitcoinPeer(pubId); });
		return true;
	}();

	int BitcoinPeer::currentTransaction = 1;
	mutex BitcoinPeer::currentTransaction_mutex;

	BitcoinPeer::~BitcoinPeer() {

	}

	BitcoinPeer::BitcoinPeer(const BitcoinPeer& rhs) : Peer(rhs) {
		
	}

	BitcoinPeer::BitcoinPeer(interfaceId id) : Peer(id) {
		
	}

	void BitcoinPeer::performComputation() {
		if (true)
			checkInStrm();

		if (guardSubmitTrans())
			submitTrans();

		if (guardMineBlock())
			mineBlock();
	}

	void BitcoinPeer::endOfRound(vector<Peer*>& _peers) {
		const vector<BitcoinPeer*> peers = reinterpret_cast<vector<BitcoinPeer*> const&>(_peers);
		int length = peers[0]->blockChain.size();
		int index = 0;
		for (int i = 0; i < peers.size(); i++) {
			if  (peers[i]->blockChain.size() < length) {
				length = peers[i]->blockChain.size();
				index = i;
			}
		}
		
		LogWriter::pushValue("throughput", length - 1);
	}

	void BitcoinPeer::checkInStrm() {
		while (!inStreamEmpty()) {
			BitcoinMessage* newMsg = dynamic_cast<BitcoinMessage*>(popInStream().getMessage());
			if (newMsg->mined) {
				unlinkedBlocks.push_back(newMsg->block);
			}
			else {
				transactions.push_back(newMsg->block);
			}
			delete newMsg;
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
		return randMod(submitRate) == 0;
	}

	void BitcoinPeer::submitTrans() {
		const lock_guard<mutex> lock(currentTransaction_mutex);
		BitcoinMessage* message = new BitcoinMessage();
		message->mined = false;
		message->block.trans.id = currentTransaction++;
		message->block.trans.roundSubmitted = RoundManager::currentRound();
		broadcast(message);
	}

	bool BitcoinPeer::guardMineBlock() {
		return randMod(mineRate) == 0;
	}

	void BitcoinPeer::mineBlock() {
		BitcoinBlock newBlock = findNextTransaction();
		if (newBlock.trans.id != -1) {
			newBlock.minerId = publicId();
			newBlock.length = blockChain.size();
			newBlock.tipMiner = blockChain[blockChain.size() - 1][0].minerId;

			blockChain.push_back(vector<BitcoinBlock>());
			blockChain[blockChain.size() - 1].push_back(newBlock);

			BitcoinMessage* message = new BitcoinMessage(newBlock, true);
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
}