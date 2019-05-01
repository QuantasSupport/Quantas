//
// Created by srai on 3/31/19.
//

#include "bCoin_Peer.hpp"
std::default_random_engine bCoin_Peer:: generator;
std::binomial_distribution<int> bCoin_Peer::distribution(10, 0.5);

bCoin_Peer::bCoin_Peer(std::string id) : Peer<bCoinMessage>(id){
    counter = 0;
    blockchain = new Blockchain(true);
    mineNextAt = -1;
}

bCoin_Peer::bCoin_Peer(const bCoin_Peer &rhs) {
    counter = rhs.counter;
    *blockchain = *rhs.blockchain;
    mineNextAt = rhs.mineNextAt;
}

bool bCoin_Peer::mineBlock() {
    std::cerr<<"Mining for Block "<<blockchain->getChainSize()<<std::endl;
    if(mineNextAt == 0){
        mineNextAt+= distribution(generator);
        blockchain->createBlock(blockchain->getChainSize(), blockchain->getLatestBlockHash(), std::to_string(blockchain->getChainSize())+"_"+id(), {id()});
        return true;
    }
    return false;
}

void bCoin_Peer::sendBlock(){
    bCoinMessage bCMessage;
    bCMessage.peerId = _id;
    bCMessage.block = blockchain->getBlockAt(blockchain->getChainSize()-1);
    bCMessage.length = blockchain->getChainSize();
    std::cerr<<bCMessage.length<<std::endl;

    std::vector<std::string> listOfTargets = neighbors();
    for(int i = 0; i < listOfTargets.size(); i++) {
        std::string targetId = listOfTargets[i];
        Packet<bCoinMessage> newMessage("", targetId, _id);
        newMessage.setBody(bCMessage);
        _outStream.push_back(newMessage);
    }
}

void bCoin_Peer::preformComputation(){
    std::cerr<<"Performing computation for peer "<<id()<<std::endl;
    mineNextAt--;
    //update own blockchain with the longest chain
    receiveBlock();
    bool mined = mineBlock();
    if (mined){
        std::cerr<<"sending block "<<std::endl;
        sendBlock();
    }
    std::cerr<<"Performed computation for peer "<<id()<<std::endl;
    counter++;

}

void bCoin_Peer::receiveBlock() {
    int maxLength = 0;
    int maxLengthIndex = -1;
    for(int i = 0; i<_inStream.size(); i++){
        if (maxLength < _inStream[i].getMessage().length && _inStream[i].getMessage().length > blockchain->getChainSize() ){
            maxLength = _inStream[i].getMessage().length;
            maxLengthIndex = i;
        }
    }

    if(maxLengthIndex != -1){
        //Select the longest chain
        for (auto it=_neighbors.begin(); it!=_neighbors.end(); ++it){
            auto n = it->second;
            if(n->id() == _inStream[maxLengthIndex].getMessage().peerId){
                bCoin_Peer* asdf= dynamic_cast<bCoin_Peer *>(n);
                setBlockchain(*(asdf)->getBlockchain());
            }
        }
        //set the new mining delay
        mineNextAt = ( distribution(generator) );
    }
    _inStream.clear();
}
