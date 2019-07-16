//
// Created by srai on 3/28/19.
//

#include "Block.hpp"

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

