//
// Created by srai on 6/3/19.
//

#ifndef PBFT_Committee_hpp
#define PBFT_Committee_hpp


#include "DS_PBFT_Peer.hpp"
#include "../Common/committee.hpp"

class PBFT_Committee : public Committee<DS_PBFT_Peer>{
private:
	std::string								status;
	std::string								leaderId;
	int 									PBFTsystemState = 0;
	bool									changeLeader = true;
	std::vector<std::string> 				leaderIdCandidates;
//	just a random peer in the first index
	int 									firstMinerIndex = 0;


//	new
	int sequenceNumber;

public:
	PBFT_Committee															(std::vector<DS_PBFT_Peer *> , DS_PBFT_Peer *, std::string , int, int, int, int);
	PBFT_Committee															(const PBFT_Committee&);
	PBFT_Committee&							operator=						(const PBFT_Committee &rhs);

	std::string								getLeaderId						(){return leaderId;}
	int 									getFirstMinerIndex				(){return firstMinerIndex;}

	void 									preformComputation				() override;
	void									refreshPeers					();
	void									initiate						();
	int										getSecurityLevel				(){return securityLevel;}
	bool 									defeated = false;
	void 									receiveTx						();

//	todo conflicts with string committeeId
	int 									committeeId = -1;
	PBFT_Message*							commitMsg = nullptr;
	int submissionRound = -1;

};

PBFT_Committee::PBFT_Committee(std::vector<DS_PBFT_Peer *> peers, DS_PBFT_Peer *sender, std::string transaction, int sLevel, int seqNumber, int committeeId, int subRound) : Committee<DS_PBFT_Peer>(peers, sender, transaction, sLevel){
	sequenceNumber = seqNumber;
	submissionRound = subRound;
	this->committeeId = committeeId;
	for(auto & committeePeer : committeePeers){
		committeePeer->setTerminated(false);
	}
}

PBFT_Committee::PBFT_Committee(const PBFT_Committee &rhs) : Committee<DS_PBFT_Peer>(rhs){
	status = rhs.status;
	leaderId = rhs.leaderId;
	PBFTsystemState = rhs.PBFTsystemState;
	changeLeader = rhs.changeLeader;
	leaderIdCandidates = rhs.leaderIdCandidates;
	firstMinerIndex = rhs.firstMinerIndex;

	sequenceNumber = rhs.sequenceNumber;
	defeated = rhs.defeated;
	committeeId = rhs.committeeId;
	submissionRound = rhs.submissionRound;
	commitMsg = new PBFT_Message(*rhs.commitMsg);
}

PBFT_Committee& PBFT_Committee::operator=(const PBFT_Committee &rhs) {
	if(this == &rhs)
		return *this;
	Committee<DS_PBFT_Peer>::operator=(rhs);
	status = rhs.status;
	leaderId = rhs.leaderId;
	PBFTsystemState = rhs.PBFTsystemState;
	changeLeader = rhs.changeLeader;
	leaderIdCandidates = rhs.leaderIdCandidates;
	firstMinerIndex = rhs.firstMinerIndex;

	sequenceNumber = rhs.sequenceNumber;
	defeated = rhs.defeated;
	committeeId = rhs.committeeId;
	submissionRound = rhs.submissionRound;
	commitMsg = new PBFT_Message(*rhs.commitMsg);
	return *this;
}

void PBFT_Committee::preformComputation(){
	for(auto & committeePeer : committeePeers){
		committeePeer->preformComputation();
	}

	bool consensus = true;
	for(int peerId = 0; peerId<size(); peerId++ ) {
		if(committeePeers[peerId]->getCommittee()!=-1){
			consensus = false;
			break;
		}
		firstMinerIndex = peerId;
	}
	consensusFlag = consensus;
	if(consensusFlag){
		commitMsg = new PBFT_Message(committeePeers[0]->getLedger().back());
		defeated = commitMsg->defeated;
	}
}

void PBFT_Committee::initiate(){
//	committee initiation done here
	// clear old committee
	for(int i = 0 ; i< committeePeers.size(); i++){
		committeePeers[i]->clearCommittee();
	}

	for(int i = 0 ; i< committeePeers.size(); i++){
		std::map<std::string, Peer<PBFT_Message>* > neighbours;	//previous group is dissolved when new group is selected
		committeePeers[i]->setCommittee(committeeId);

		for(int j = 0; j< committeePeers.size(); j++){
			if(i != j){
				committeePeers[i]->addCommitteeMember(*committeePeers[j]);
			}
		}
	}

	for(auto &committeePeer: committeePeers){
		committeePeer->clearPrimary();
		committeePeer->initPrimary();
	}


	for(auto &committeePeer: committeePeers){
		if(committeePeer->isPrimary()){
			committeePeer->makeRequest(sequenceNumber, submissionRound);
		}
	}
}

void PBFT_Committee::refreshPeers() {
	for (auto & committeePeer : committeePeers) {
		committeePeer->setBusy(false);
	}
}

void PBFT_Committee::receiveTx(){
	for(auto & committeePeer : committeePeers){
		committeePeer->receiveTx();
	}
}

#endif //PBFT_Committee_hpp
