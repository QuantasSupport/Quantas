//
// Created by srai on 3/28/19.
//

#ifndef Block_hpp
#define Block_hpp

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <set>
using std::vector;
using std::string;
using std::set;

class Block {
	int 							index;
	string 							previousHash;
	string 							hash;
	set<string> 					publishers;

public:
	Block																(int, string, string, set<string> );
	string 							getPreviousHash						() const;
	string 							getHash								() const;
	int 							getIndex							() const;
	set<string> 					getPublishers						() const;
	friend std::ostream &operator<<(std::ostream &os, const Block &blockToPrint);

};

Block::Block(int index, string prevHash, string blockHash, set<string> publishersId) {
	this->publishers = std::move(publishersId);
	this->previousHash = std::move(prevHash);
	this->index = index;
	this->hash = std::move(blockHash);

}

int Block::getIndex() const {
	return this->index;
}

string Block::getPreviousHash() const{
	return this->previousHash;
}

string Block::getHash() const{
	return this->hash;
}

set<string> Block::getPublishers() const{
	return this->publishers;
}
std::ostream& operator<<(std::ostream& os, const Block& blockToPrint){
	os<<std::endl<<"Block "<<blockToPrint.getIndex()<<", Hash "<<blockToPrint.getHash()<<", Previous hash "<<blockToPrint.getPreviousHash()<<std::endl;
	return os;
}


#endif