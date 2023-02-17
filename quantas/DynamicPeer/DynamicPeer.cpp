/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "DynamicPeer.hpp"

namespace quantas {

	int  DynamicPeer::acceptedBlocks  = 0;

	DynamicPeer::~DynamicPeer() {}

	DynamicPeer::DynamicPeer(const DynamicPeer& rhs) : Peer<DynamicMessage>(rhs) {}

	DynamicPeer::DynamicPeer(long id) : Peer(id) {
		DynamicBlock genesis;
		genesis.depth = 1;
		blockChain.push_back(genesis);
	}

	void DynamicPeer::performComputation() {
		checkInStrm();

		if (guardMineBlock()) {
			mineBlock();
                }

		else {
			sendBlockChain();
                }
	}

	void DynamicPeer::endOfRound(const vector<Peer<DynamicMessage>*>& _peers) {
		const vector<DynamicPeer*> peers = reinterpret_cast<vector<DynamicPeer*> const&>(_peers);
		bool  flag  = true;
		int   index = acceptedBlocks + 1;

		for ( ; index < blockChain.size(); ++index) {
			for (int i = 0; i < peers.size(); ++i) {
				if (index < peers[i]->blockChain.size()) { // check range
					if (blockChain[index] != peers[i]->blockChain[index]) {
						flag = false;
						break;
					}
				}

				else {
					flag = false;
					break;
				}
			}

			if (!flag) {
				break;
			}

			else {
				++acceptedBlocks;
			}
		}

                cout << "Round: " << getRound() << "; Accepted Blocks: " << acceptedBlocks << endl;
    
		if (lastRound()) {
			acceptedBlocks = 0;
		}
	}

	void DynamicPeer::checkInStrm() {
		while (!inStreamEmpty()) {
			Packet<DynamicMessage> newMsg = popInStream();
      
			if (newMsg.getMessage().blockChain.size() > blockChain.size()) {
				blockChain = newMsg.getMessage().blockChain;
			}
		}
	}

	bool DynamicPeer::guardMineBlock() {
		return uniformInt(1, mineRate) == 1;
	}

	void DynamicPeer::sendBlockChain() {
		DynamicMessage message;
		message.blockChain = blockChain;
		randomMulticast(message);
	}

	void DynamicPeer::mineBlock() {
		DynamicBlock block;
		block.minerId    = id();
		block.tipMiner   = blockChain[blockChain.size() - 1].minerId;
		block.depth      = blockChain.size() + 1;
		block.roundMined = getRound();

		blockChain.push_back(block);
		sendBlockChain();
	}

	bool DynamicBlock::operator!= (const DynamicBlock& block) const {
		return (block.minerId != this->minerId || block.tipMiner != this->tipMiner || block.roundMined != this->roundMined || block.depth != this->depth);
	}

	ostream& DynamicPeer::printTo(ostream& out) const {
		Peer<DynamicMessage>::printTo(out);

		out << id() << endl;
		out << "counter:" << getRound() << endl;

		return out;
	}

	ostream& operator<< (ostream& out, const DynamicPeer& peer) {
		peer.printTo(out);
		return out;
	}

	Simulation<quantas::DynamicMessage, quantas::DynamicPeer>* generateSim() {
        
        Simulation<quantas::DynamicMessage, quantas::DynamicPeer>* sim = new Simulation<quantas::DynamicMessage, quantas::DynamicPeer>;
        return sim;
    }
}
