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

	Blockchain& 						operator=						(const Blockchain&);
	Blockchain															(const Blockchain&);
	~Blockchain															() = default;

	friend std::ostream &operator<<(std::ostream &os, Blockchain &blockchain);
};


#endif