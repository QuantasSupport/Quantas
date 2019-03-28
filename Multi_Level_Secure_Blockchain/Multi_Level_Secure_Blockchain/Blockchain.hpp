//
// Created by srai on 3/28/19.
//

#ifndef Blockchain_hpp
#define Blockchain_hpp

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "Block.hpp"
using std::vector;
using std::string;
using std::unique_ptr;

class Blockchain {
	vector<unique_ptr<Block>> 					chain;

public:
	explicit Blockchain													(bool init);

	int 								createBlock						(int , string , string , set<string> );

	string 								getLatestBlockHash				();
	int 								getChainSize					() const;

	Block 								getBlockAt						(int index);

	Blockchain& operator=(const Blockchain&);
	friend std::ostream &operator<<(std::ostream &os, Blockchain &blockchain);
};


Blockchain::Blockchain(bool init){
	if (init) {
		this->chain.push_back(std::make_unique<Block>(0,"-1_-1","genesisHash", std::set<string>{}));
	}
}

int Blockchain::createBlock(int blockIndex, string prevHash, string blockHash, set<string> publishers) {
	//inserting to the chain without any work
	this->chain.push_back(std::make_unique<Block>(blockIndex,prevHash,blockHash, publishers));
}


Block Blockchain::getBlockAt(int index) {
	return *chain[index];
}

int Blockchain::getChainSize() const{
	return static_cast<int>(this->chain.size());
}


string Blockchain::getLatestBlockHash() {
	return this->chain[chain.size()-1]->getHash();
}

std::ostream& operator<<(std::ostream& os, Blockchain& blockchain){
	for(auto const& node: blockchain.chain) {
		os << *node;
	}
	return os;
}

Blockchain& Blockchain::operator=(const Blockchain& rhs){
	chain.clear();
	for (int i = 0; i < rhs.chain.size(); i++) {
		chain.push_back(std::make_unique<Block>(*rhs.chain[i]));
	}

	return *this;

}
#endif