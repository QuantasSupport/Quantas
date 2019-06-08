//
// Created by srai on 6/8/19.
//

#ifndef bCoin_Committee_hpp
#define bCoin_Committee_hpp


#include "DS_bCoin_Peer.hpp"
#include "committee.hpp"

std::vector<int> someByzantineCount;
std::vector<int> moreThanHalfByzantineCount;
std::map<int, std::map<int, int>> securityLevelToByznainteFound = {};
class bCoin_Committee : public Committee<DS_bCoin_Peer>{
private:
	int 									firstMinerIndex = -1;

public:
	bCoin_Committee															(std::vector<DS_bCoin_Peer *> , DS_bCoin_Peer *, std::string , int);
	bCoin_Committee															(const bCoin_Committee&);
	bCoin_Committee&						operator=						(const bCoin_Committee &rhs);

	int 									getFirstMinerIndex				(){return firstMinerIndex;}

	void									performComputation				() override ;
	void									initiate						();
	bool									checkForConsensus				();
	void									propagateBlock					();


};

bCoin_Committee::bCoin_Committee(std::vector<DS_bCoin_Peer *> peers, DS_bCoin_Peer *sender, std::string transaction, int sLevel) : Committee<DS_bCoin_Peer>(peers, sender, transaction, sLevel){
	for(auto & committeePeer : committeePeers){
		committeePeer->setTerminated(false);
	}
}


bCoin_Committee::bCoin_Committee(const bCoin_Committee &rhs) : Committee<DS_bCoin_Peer>(rhs){
	firstMinerIndex = rhs.firstMinerIndex;
}

bCoin_Committee& bCoin_Committee::operator=(const bCoin_Committee &rhs) {
	if(this == &rhs)
		return *this;
	Committee<DS_bCoin_Peer>::operator=(rhs);
	firstMinerIndex = rhs.firstMinerIndex;
	return *this;
}

void bCoin_Committee::performComputation(){
	for(auto & committeePeer : committeePeers){
		if(!committeePeer->isTerminated())
			committeePeer->preformComputation();
	}
	propagateBlock();
	checkForConsensus();
}

bool bCoin_Committee::checkForConsensus(){
	//if a single committee member mines a block, consensus is reached.
	consensusFlag = true;
	//	check if consensus is reached in every peer. if all peers are terminated, consensus reached
	for(auto & committeePeer : committeePeers){
		if(!committeePeer->isTerminated()){
			consensusFlag = false;
		}
	}

	if(consensusFlag)
		std::cerr<<"CONSENSUS REACHED FOR TX "<<tx<<std::endl;
	else
		std::cerr<<"NO CONSENSUS FOR TX "<<tx<<std::endl;

	return consensusFlag;
}

void bCoin_Committee::propagateBlock(){
	//	this function decides whether a block will be added to the network or not, be it good or bad.
	for(int i = 0; i< committeePeers.size(); i++){
		if(!committeePeers[i]->isBusy()&&!committeePeers[i]->isTerminated()){
			if(committeePeers[i]->isByzantine()){
				if(byzantineRatio>=0.5){
					//	bad block, allow propogation
					if(firstMinerIndex == -1){
						moreThanHalfByzantineCount.push_back(securityLevel);

						firstMinerIndex = i;
						committeePeers[i]->sendBlock();
						//	terminate the peer
						committeePeers[i]->setTerminated(true);
						committeePeers[i]->clearConsensusTx();
						committeePeers[i]->addToBlocks();

					}else{
						committeePeers[i]->setTerminated(true);
						committeePeers[i]->clearConsensusTx();
					}

				}else if(byzantineRatio <0.5){
					//	reset only if this peer is the first one to mine the block
					if(firstMinerIndex == -1){
						for(auto & committeePeer : committeePeers){
							committeePeer->resetMiningClock();
						}
						someByzantineCount.push_back(securityLevel);
						//	exit the loop
						break;
					}else{
//						std::cerr<<"PEER "<<committeePeers[i]->id()<<" ALREADY MINED A BLOCK IN THIS COMMITTEE"<<std::endl;
					}
				}
			}else{
				if(firstMinerIndex == -1)
				{
					firstMinerIndex = i;
					committeePeers[i]->sendBlock();
					committeePeers[i]->addToBlocks();
					committeePeers[i]->setTerminated(true);
					committeePeers[i]->clearConsensusTx();

				} else{
//					delete the mined block, block will be received from the previous miner
					committeePeers[i]->deleteMinedBlock();
				}
			}
		}
	}
}

void bCoin_Committee::initiate(){
	dynamic_cast<DS_bCoin_Peer *>(senderPeer)->makeRequest(committeePeers, tx);

	for(int i = 0 ; i< committeePeers.size(); i++){
		committeePeers[i]->resetMineNextAt();
		std::map<std::string, Peer<DS_bCoinMessage>* > neighbours;	//	previous group is dissolved when new group is selected

		for(int j = 0; j< committeePeers.size(); j++){
			if(i != j){
				neighbours[committeePeers[j]->id()] = committeePeers[j];
			}
		}
		dynamic_cast<DS_bCoin_Peer *> (committeePeers[i])->setCommitteeNeighbours(neighbours);
	}
}


#endif //bCoin_Committee_hpp
