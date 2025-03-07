/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "KSMPeer.hpp"

namespace quantas {
	KSMPeer::~KSMPeer() {}

	KSMPeer::KSMPeer(const KSMPeer& rhs) : Peer<KSMMessage>(rhs) {}

	KSMPeer::KSMPeer(long id) : Peer(id) {
		KSMBlock genesis;
		genesis.depth = 0;
		blockChain.push_back(genesis);

		KSMBlockLabel blockLabel;
		blockLabel.block = genesis;
		blockLabel.label = "accepted";
		perBlockLabels.push_back(blockLabel);
	}

	void KSMPeer::performComputation() {
		if (getRound() == 0) {
			for (int i = 0; i < getSourcePoolSize(); ++i) {
				sourcePoolIds.push_back(i);
			}
		}

		checkInStrm();
		
		updateBlockLabels();

		if (std::find(sourcePoolIds.begin(), sourcePoolIds.end(), id()) != sourcePoolIds.end() && guardMineBlock()) {
			mineBlock();
		}

		else {
			sendBlockChain();
		}
	}

	void KSMPeer::endOfRound(const vector<Peer<KSMMessage>*>& _peers) {
		const vector<KSMPeer*> peers = reinterpret_cast<vector<KSMPeer*> const&>(_peers);

		auto minAcceptedBlocks = std::min_element(peers.begin(), peers.end(),
			[](const KSMPeer* peer1, const KSMPeer* peer2) {
				return (peer1->acceptedBlocks < peer2->acceptedBlocks);
			});

		if (minAcceptedBlocks != peers.end()) {
			cout << "Round: " << getRound() << ";    Number of accepted blocks: " << (*minAcceptedBlocks)->acceptedBlocks << endl;
		}
	}

	void KSMPeer::checkInStrm() {
		while (!inStreamEmpty()) {
			Packet<KSMMessage> newMsg = popInStream();

			if (blockChain.size() < newMsg.getMessage().blockChain.size()) {
				createBranch(blockChain);
				blockChain = newMsg.getMessage().blockChain;

				if (std::find(sourcePoolIds.begin(), sourcePoolIds.end(), id()) != sourcePoolIds.end()) {
					auto found = sourcePoolPositions.find(id());
					if (found != sourcePoolPositions.end()) {
						found->second = blockChain[blockChain.size() - 1];
					}

					else {
						sourcePoolPositions.insert(std::pair<int, KSMBlock>(id(), blockChain[blockChain.size() - 1]));
					}
				}
			}

			else {
			    createBranch(newMsg.getMessage().blockChain);
			}

			for (auto& branch : newMsg.getMessage().branches) {
				createBranch(branch);
			}

			for (auto& position : newMsg.getMessage().sourcePoolPositions) {
				auto found = sourcePoolPositions.find(position.first);
				if (found != sourcePoolPositions.end()) {
					if (found->second.depth < position.second.depth) {
						found->second = position.second;
					}
				}

				else {
					sourcePoolPositions.insert(std::pair<int, KSMBlock>(position.first, position.second));
				}
			}
		}
	}

	void KSMPeer::createBranch(const vector<KSMBlock>& bc) {
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

	void KSMPeer::updateBlockLabels() {
		if (sourcePoolIds.size() == sourcePoolPositions.size()) {
			auto branch = branches.begin();
			while (branch != branches.end()) {
				if (*branch != blockChain) {
					bool remove = true;
					for (auto position = sourcePoolPositions.begin(); position != sourcePoolPositions.end(); ++position) {
						if (std::find(branch->begin(), branch->end(), position->second) != branch->end()) {
							remove = false;
						}
					}
					
					if (remove) {
						branch = branches.erase(branch);
					}
					
					else {
						++branch;
					}
				}

				else {
					branch = branches.erase(branch);
				}
			}

			int index = acceptedBlocks + 1;
			for ( ; index < blockChain.size(); ++index) {
				bool flag    = false;
				int  counter = 0;
				for (auto position = sourcePoolPositions.begin(); position != sourcePoolPositions.end(); ++position) {
					if (position->second.depth >= blockChain[index].depth) {
						if (std::find(blockChain.begin(), blockChain.end(), position->second) == blockChain.end()) {
							branch = branches.begin();
							for ( ; branch != branches.end(); ++branch) {
								if (std::find(branch->begin(), branch->end(), position->second) != branch->end()) {
									if (std::find(branch->begin(), branch->end(), blockChain[index]) == branch->end()) {
										flag = true;
									}

									else {
										++counter;
									}

									break;
								}
							}
						}

						else {
							++counter;
						}

						if (flag) {
							break;
						}
					}
				}

				if (counter == sourcePoolIds.size()) {
					updatePerBlockLabels(blockChain[index], "accepted");
				}
			}
		}
	}

	void KSMPeer::updatePerBlockLabels(const KSMBlock& block, const string& label) {
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
				KSMBlockLabel blockLabel;
				blockLabel.block = block;
				blockLabel.label = label;
				perBlockLabels.push_back(blockLabel);
			}
		}
	}

	bool KSMPeer::guardMineBlock() {
		return uniformInt(1, mineRate) == 1;   // interval: [1, mineRate]
	}

	void KSMPeer::sendBlockChain() {
		KSMMessage message;
		message.blockChain          = blockChain;
		message.branches            = branches;
		message.sourcePoolPositions = sourcePoolPositions;
		randomMulticast(message);
	}

	void KSMPeer::mineBlock() {
		KSMBlock block;
		block.minerId    = id();
		block.tipMiner   = blockChain[blockChain.size() - 1].minerId;
		block.depth      = blockChain.size();
		block.roundMined = getRound();
		blockChain.push_back(block);

		auto found = sourcePoolPositions.find(id());

		if (found != sourcePoolPositions.end()) {
			found->second = blockChain[blockChain.size() - 1];
		}

		else {
			sourcePoolPositions.insert(std::pair<int, KSMBlock>(id(), blockChain[blockChain.size() - 1]));
		}

		sendBlockChain();
	}

	bool KSMBlock::operator!= (const KSMBlock& block) const {
		return (block.minerId != this->minerId || block.tipMiner != this->tipMiner || block.roundMined != this->roundMined || block.depth != this->depth);
	}

	bool KSMBlock::operator== (const KSMBlock& block) const {
		return (block.minerId == this->minerId && block.tipMiner == this->tipMiner && block.roundMined == this->roundMined && block.depth == this->depth);
	}

	ostream& KSMPeer::printTo(ostream& out) const {
		Peer<KSMMessage>::printTo(out);

		out << id() << endl;
		out << "counter:" << getRound() << endl;

		return out;
	}

	ostream& operator<< (ostream& out, const KSMPeer& peer) {
		peer.printTo(out);
		return out;
	}

	Simulation<quantas::KSMMessage, quantas::KSMPeer>* generateSim() {
        
        Simulation<quantas::KSMMessage, quantas::KSMPeer>* sim = new Simulation<quantas::KSMMessage, quantas::KSMPeer>;
        return sim;
    }

}

