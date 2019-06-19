//
// Created by srai on 4/12/19.
//


#include <utility>

#include "DAGBlock.hpp"

DAGBlock::DAGBlock(int index, std::vector<string> prevHashes, string DAGBlockHash, set<string> publishersId, string data = "", bool byzantine= false) {
	this->publishers = std::move(publishersId);
	this->previousHashes = std::move(prevHashes);
	this->index = index;
	this->hash = std::move(DAGBlockHash);
	this->data=std::move(data);
	this->isByzantine = byzantine;
}

DAGBlock::DAGBlock(const DAGBlock &rhs) {
	this->publishers = rhs.publishers;
	this->previousHashes = rhs.previousHashes;
	this->index = rhs.index;
	this->hash = rhs.hash;
	this->data = rhs.data;
	this->isByzantine = rhs.isByzantine;
}

DAGBlock& DAGBlock::operator=(const DAGBlock & rhs) {
	if(this == &rhs)
		return *this;
	this->publishers = rhs.publishers;
	this->previousHashes = rhs.previousHashes;
	this->index = rhs.index;
	this->hash = rhs.hash;
	this->data = rhs.data;
	this->isByzantine = rhs.isByzantine;
	return *this;
}

int DAGBlock::getIndex() const {
	return this->index;
}

std::vector<string> DAGBlock::getPreviousHashes() const{
	return this->previousHashes;
}

string DAGBlock::getHash() const{
	return this->hash;
}

set<string> DAGBlock::getPublishers() const{
	return this->publishers;
}

string DAGBlock::getData() const{
	return this->data;
}

std::ostream& operator<<(std::ostream& os, const DAGBlock& DAGBlockToPrint){
	os<<std::endl<<"DAGBlock "<<DAGBlockToPrint.getIndex()<<", Hash "<<DAGBlockToPrint.getHash()<<" Previous hashes: ";
	for(auto &hash: DAGBlockToPrint.getPreviousHashes()){
		std::cerr<<hash<<" ";
	}
	return os;
}

bool DAGBlock::isByantine() const{
	return isByzantine;
}

bool DAGBlock::operator==(const DAGBlock &rhs)const{
    return (    (index == rhs.index) &&
                (previousHashes == rhs.previousHashes) &&
                (hash == rhs.hash) &&
                (publishers == rhs.publishers) &&
                (data == rhs.data) &&
                (isByzantine == rhs.isByzantine) &&
                (secruityLevel == rhs.secruityLevel) &&
                (submissionRound == rhs.submissionRound) &&
                (confirmedRound == rhs.confirmedRound)
    )
}

