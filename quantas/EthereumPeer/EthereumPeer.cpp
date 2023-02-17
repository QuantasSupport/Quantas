/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <limits.h>
#include "EthereumPeer.hpp"

namespace quantas {

	int EthereumPeer::currentTransaction = 1;
	mutex EthereumPeer::currentTransaction_mutex;

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
			submitTrans();

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
		LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["throughput"].push_back(length);
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
		return randMod(submitRate) == 0;
	}

	void EthereumPeer::submitTrans() {
		const lock_guard<mutex> lock(currentTransaction_mutex);
		EthereumPeerMessage message;
		message.mined = false;
		message.block.trans.id = currentTransaction++;
		message.block.trans.roundSubmitted = getRound();
		broadcast(message);
	}

	bool EthereumPeer::guardMineBlock() {
		return randMod(mineRate) == 0;
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

	Simulation<quantas::EthereumPeerMessage, quantas::EthereumPeer>* generateSim() {
        
        Simulation<quantas::EthereumPeerMessage, quantas::EthereumPeer>* sim = new Simulation<quantas::EthereumPeerMessage, quantas::EthereumPeer>;
        return sim;
    }
}