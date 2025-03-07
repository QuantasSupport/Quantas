/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "KPTPeer.hpp"

namespace quantas {
    /*
    __________________________________________________________________________________________________________
    | PT VALUES FOR A NETWORK SIZE OF 100 AND 75 NODES IN SOURCE POOL                                        |
    __________________________________________________________________________________________________________
    | Max Number Of Neighbors A Node Can Message  |  5  |  7  |  9  |  10 |  20  |  40  |  60  |  80  |  99  |
    __________________________________________________________________________________________________________
    | Corresponding PT Values (~rounds)           | 666 | 529 | 324 | 151 |  94  |  58  |  54  |  52  |  36  |
    __________________________________________________________________________________________________________
    */

	const static int PT = 36;

	KPTPeer::~KPTPeer() {}

	KPTPeer::KPTPeer(const KPTPeer& rhs) : Peer<KPTMessage>(rhs) {}

	KPTPeer::KPTPeer(long id) : Peer(id) {
		KPTBlock genesis;
		genesis.depth = 0;
		blockChain.push_back(genesis);

		KPTBlockLabel blockLabel;
		blockLabel.block = genesis;
		blockLabel.label = "accepted";
		perBlockLabels.push_back(blockLabel);
	}

	void KPTPeer::performComputation() {
		checkInStrm();
		updateBlockLabels();

		if (guardMineBlock()) {
			mineBlock();
		}

		else {
			sendBlockChain();
		}
	}

	void KPTPeer::endOfRound(const vector<Peer<KPTMessage>*>& _peers) {
		const vector<KPTPeer*> peers = reinterpret_cast<vector<KPTPeer*> const&>(_peers);

		auto minAcceptedBlocks = std::min_element(peers.begin(), peers.end(),
			[](const KPTPeer* peer1, const KPTPeer* peer2) {
				return (peer1->acceptedBlocks < peer2->acceptedBlocks);
			});
    
		if (minAcceptedBlocks != peers.end()) {
			cout << "Round: " << getRound() << "; Accepted Blocks: " << (*minAcceptedBlocks)->acceptedBlocks << endl;
		}
	}

	void KPTPeer::checkInStrm() {
		while (!inStreamEmpty()) {
			Packet<KPTMessage> newMsg  = popInStream();
			if (blockChain.size() < newMsg.getMessage().blockChain.size()) {
				createBranch(blockChain);
				blockChain = newMsg.getMessage().blockChain;
			}
				
			else {
				createBranch(newMsg.getMessage().blockChain);
			}
			
			for (auto& branch : newMsg.getMessage().branches) {
				createBranch(branch);
			}
		}
	}

	void KPTPeer::createBranch(const vector<KPTBlock>& bc) {
		auto found = std::find(branches.begin(), branches.end(), bc);
		if (found == branches.end()) {
			if (bc != blockChain) {
				int  numberOfBranches  = 0;
				int  numberOfNoMatches = 0;
				bool insertBranch      = false;
				auto branch            = branches.begin();
				while (branch != branches.end()) {
					bool doesNotMatch      = false;
					bool branchIsOutDated  = false;
					int  forLoopUpperBound = 0;
					
					++numberOfBranches;

					if (branch->size() < bc.size()) {
						forLoopUpperBound  = branch->size();
						branchIsOutDated   = true;
					}

					else {
						forLoopUpperBound = bc.size();
					}

					for (int j = 0; j < forLoopUpperBound; ++j) {						
						if ((*branch)[j] != bc[j]) {
							++numberOfNoMatches;
							doesNotMatch = true;
							break;
						}
					}

					if (!doesNotMatch) {
						if (branchIsOutDated) {
							branch = branches.erase(branch);
							insertBranch = true;
						}

						else{
							++branch;
						}
					}

					else {
						++branch;
					}
				}

				if (numberOfNoMatches == numberOfBranches || insertBranch || branches.empty()) {
					branches.push_back(bc);
				}
			}
		}
	}

	void KPTPeer::updateBlockLabels() {
		std::sort(branches.begin(), branches.end(),
			[](const vector<KPTBlock>& branch1, const vector<KPTBlock>& branch2) {
				return (branch1.size() > branch2.size());
			});

		auto branch = branches.begin();
		while (branch != branches.end()) {
			if (blockChain.size() > branch->size()) {
				if ((2*PT) <= (getRound() - (*branch)[branch->size() - 1].roundMined)) {
					for (int i = 0; i < branch->size(); ++i) {
						if (blockChain[i] != (*branch)[i]) {
							updatePerBlockLabels((*branch)[i], "rejected");
						}
					}

					branch = branches.erase(branch);
				}

				else {
					++branch;
				}
			}
			
			else {
				++branch;
			}
		}

		for (int index = acceptedBlocks + 1; index < blockChain.size(); ++index) {
			if ((blockChain[index].roundMined + (2 * PT)) <= getRound()) {
				bool noCompetingBranches    = true;
				branch = branches.begin();
				for ( ; branch != branches.end(); ++branch) {
					if (std::find(branch->begin(), branch->end(), blockChain[index]) == branch->end()) {
						noCompetingBranches = false;
						break;
					}
				}

				if (noCompetingBranches) {
					updatePerBlockLabels(blockChain[index], "accepted");
				}
			}

			else {
				break;
			}
		}
	}

	void KPTPeer::updatePerBlockLabels(const KPTBlock& block, const string& label) {
		auto found = perBlockLabels.begin();
		for ( ; found != perBlockLabels.end(); ++found) {
			if (block == found->block && label == found->label) {
				break;
			}
		}

		if (found == perBlockLabels.end()) {
			bool flag = true;
			if (label == "accepted") {
				auto it = perBlockLabels.begin();
				for ( ; it != perBlockLabels.end(); ++it) {
					if (block.tipMiner == it->block.minerId && (block.depth - 1) == it->block.depth && it->label == "accepted") {
						break;
					}
				}

				if (it == perBlockLabels.end()) {
					flag = false;
				}

				else {
					++acceptedBlocks;
				}
			}

			if (flag) {
				KPTBlockLabel blockLabel;
				blockLabel.block = block;
				blockLabel.label = label;
				perBlockLabels.push_back(blockLabel);
			}
		}
	}

	bool KPTPeer::guardMineBlock() {
		return uniformInt(1, mineRate) == 1;
	}

	void KPTPeer::sendBlockChain() {
		KPTMessage message;
		message.blockChain = blockChain;
		message.branches   = branches;
		randomMulticast(message);
	}

	void KPTPeer::mineBlock() {
		KPTBlock block;
		block.minerId    = id();
		block.tipMiner   = blockChain[blockChain.size() - 1].minerId;
		block.depth      = blockChain.size();
		block.roundMined = getRound();
		blockChain.push_back(block);

		sendBlockChain();
	}

	bool KPTBlock::operator!= (const KPTBlock& block) const {
		return (block.minerId != this->minerId || block.tipMiner != this->tipMiner || block.roundMined != this->roundMined || block.depth != this->depth);
	}

	bool KPTBlock::operator== (const KPTBlock& block) const {
		return (block.minerId == this->minerId && block.tipMiner == this->tipMiner && block.roundMined == this->roundMined && block.depth == this->depth);
	}

	ostream& KPTPeer::printTo(ostream& out) const {
		Peer<KPTMessage>::printTo(out);

		out << id() << endl;
		out << "counter:" << getRound() << endl;

		return out;
	}

	ostream& operator<< (ostream& out, const KPTPeer& peer) {
		peer.printTo(out);
		return out;
	}

	Simulation<quantas::KPTMessage, quantas::KPTPeer>* generateSim() {
        
        Simulation<quantas::KPTMessage, quantas::KPTPeer>* sim = new Simulation<quantas::KPTMessage, quantas::KPTPeer>;
        return sim;
    }

}

