//
// Created by srai on 3/31/19.
//

#include "DS_bCoin_Peer.hpp"
//#include "hash.hpp"
#include <cassert>

std::default_random_engine DS_bCoin_Peer::generator;
std::binomial_distribution<int> DS_bCoin_Peer::distribution(10, 0.5);

DS_bCoin_Peer::DS_bCoin_Peer(const std::string id) : Peer<DS_bCoinMessage>(id){
	counter = 0;
	dag = {};
	mineNextAt = -1;
	consensusQueue = {};
	consensusTx = "";
	committeeNeighbours = _neighbors;
	minedBlock = nullptr;
	_busy = false;
}

DS_bCoin_Peer::DS_bCoin_Peer(const DS_bCoin_Peer &rhs)  : Peer<DS_bCoinMessage>(rhs) {
	counter = rhs.counter;
	dag = rhs.dag;
	mineNextAt = rhs.mineNextAt;
	consensusQueue = rhs.consensusQueue;
	consensusTx = rhs.consensusTx;
	committeeNeighbours = rhs.committeeNeighbours;
	minedBlock = nullptr;
	terminated = rhs.terminated;
}

bool DS_bCoin_Peer::mineBlock() {
	if(mineNextAt == 0){

		std::vector<string> hashesToConnectTo=dag.getTips();
		std::string newBlockString;
		for (auto const& s : hashesToConnectTo) { newBlockString += "_"+s;}

		newBlockString+= "_"+consensusTx;

//		string newBlockHash = sha256(newBlockString);
		string newBlockHash = std::to_string(dag.getSize());


		messageToSend.dagBlock = dag.createBlock(dag.getSize(), hashesToConnectTo, newBlockHash, {id()}, consensusTx, _byzantine);
		minedBlock = new DAGBlock(messageToSend.dagBlock);

		messageToSend.dagBlockFlag = true;

		//	not popping in here pop only if successfully mined
		//	consensusQueue.pop_front();

		return true;
	}
	return false;
}

void DS_bCoin_Peer::preformComputation(){
	receiveMsg();
	if(!_busy||consensusTx.empty())
		return;					//	not busy, not chosen in a committee
	bool mined = mineBlock();
	if (mined){
		_busy = false;
	}
	mineNextAt--;

	counter++;
}

void DS_bCoin_Peer::makeRequest(){}

void DS_bCoin_Peer::receiveMsg(){

	//	DAG blocks are handled here
	for(auto & i : _inStream){
		if(i.getMessage().dagBlockFlag){
			if(_busy){
				//	block not mined yet
				_busy = false;
				minedBlock = new DAGBlock(i.getMessage().dagBlock);
				dagBlocks.push_back(*minedBlock);
				terminated = true;

			}else{

				//	not busy means peer already finished working on this transaction.
				//	a byzantine peer might have mined the block after or at the same time as the honest one and busy flag will be false because he HAS mined.
				delete minedBlock;
				minedBlock = new DAGBlock(i.getMessage().dagBlock);
				dagBlocks.push_back(*minedBlock);
				terminated = true;

			}
			//	removing the transaction
			clearConsensusTx();
		}

		if(i.getMessage().txFlag){
			if(minedBlock){
				//	this means the block for this transaction has already been mined.
				continue;
			}
			consensusTx = i.getMessage().message[0];
		}
	}
	_inStream.clear();
}

void DS_bCoin_Peer::sendBlock(){

	if(!messageToSend.txFlag && messageToSend.dagBlockFlag){
		for(auto & _neighbor : committeeNeighbours) {
			Peer<DS_bCoinMessage> *peer = _neighbor.second;
			Packet<DS_bCoinMessage> newMessage("", peer->id(), _id);

			messageToSend.peerId = id();
			messageToSend.blockByzantineFlag = isByzantine();
			newMessage.setBody(messageToSend);
			_outStream.push_back(newMessage);
		}
	}
}

void DS_bCoin_Peer::makeRequest(const vector<DS_bCoin_Peer *>& committeeMembers, const std::string& tx) {
	DS_bCoinMessage txMessage;
	txMessage.peerId = id();
	txMessage.message.push_back(tx+"_"+id());
	txMessage.txFlag = true;
	for(auto &committeePeer: committeeMembers){

		if(!(committeePeer->id()==id())){
			Packet<DS_bCoinMessage> newMessage("", committeePeer->id(), id());
			newMessage.setBody(txMessage);
			_outStream.push_back(newMessage);
		}else{
			//	need to add to consensusqueue since the peer is in the committee
			consensusTx = txMessage.message[0];
		}
	}
	this->transmit();
	messageToSend= {};
}

void DS_bCoin_Peer::updateDAG() {
	assert(consensusQueue.empty());
	//	process self mined block
	if(minedBlock!= nullptr){
		dag.addVertex(*minedBlock, minedBlock->getPreviousHashes());
		delete minedBlock;
		minedBlock = nullptr;
	}
	//	resolve the dag
	for(auto & i : _inStream){
		if(i.getMessage().txFlag){
			//	check to see if the transaction has already been added to the dag
			if(dag.transactionInDag(i.getMessage().message[0]))
				continue;
			else{
				std::cerr<<i.getMessage().message[0]<<std::endl;
				consensusQueue.push_back(i.getMessage().message[0]);
				assert(false);
			}
			continue;
		}
		assert(!i.getMessage().txFlag);
		assert(i.getMessage().dagBlockFlag);
		if(i.getMessage().dagBlockFlag){
			dagBlocks.push_back(i.getMessage().dagBlock);
		}
	}

	if(!dagBlocks.empty()){
		sort( dagBlocks.begin( ), dagBlocks.end( ), [ ]( const auto& lhs, const auto& rhs )
		{
			return lhs.getData() < rhs.getData();
		});
		for(auto & dagBlock : dagBlocks){
			dag.addVertex(dagBlock, dagBlock.getPreviousHashes(), dagBlock.isByantine());
		}
	}

	dag.setTips();
	dagBlocks.clear();
	_inStream.clear();
}


void DS_bCoin_Peer::resetMiningClock() {
	_busy = true;
	mineNextAt = distribution(generator);
	delete minedBlock;
	minedBlock = nullptr;
}

void DS_bCoin_Peer::addToBlocks(){
	//	add to dagblocks only when sending, receivers will add it from receive msg
	assert(minedBlock != nullptr);
	dagBlocks.push_back(*minedBlock);
}

void DS_bCoin_Peer::clearConsensusTx(){
	if(consensusTx.empty()){
//		std::cerr<<"THE TX HAS NOT BEEN RECEIVED YET"<<std::endl;
	}
	consensusTx.clear();
}
