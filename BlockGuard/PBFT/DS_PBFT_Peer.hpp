//
//  BlockGuardSharded.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/28/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef DS_PBFT_Peer_hpp
#define DS_PBFT_Peer_hpp

#include <stdio.h>
#include "PBFT_Peer.hpp"
//#include "../Common/hash.hpp"
#include "../Common/DAG.hpp"
#include "../Common/Logger.hpp"
class DS_PBFT_Peer : public PBFT_Peer{
protected:

	// state vars
	int                                                        _groupId;
	int                                                        _committeeId;
	std::map<std::string, Peer<PBFT_Message>* >                _groupMembers;
	std::map<std::string, Peer<PBFT_Message>* >                _committeeMembers;
	std::vector<int>                                           _committeeSizes;

	// logging vars
	bool                                                       _printCommittee;
	bool                                                       _printGroup;

	// methods from PBFT peer that need to be adjusted for sharding
	void                        commitRequest           () override;
	void                        braodcast               (const PBFT_Message&) override;
	DAG												dag;

public:
	DS_PBFT_Peer                                    (std::string);
	DS_PBFT_Peer                                    (const DS_PBFT_Peer&);
	~DS_PBFT_Peer                                   ()                                              {};

	//setters
	void                        setGroup                (int id)                                        {clearGroup();_groupId = id;};
	void                        setCommittee            (int id)                                        {clearCommittee();_committeeId = id;};
	void                        addGroupMember          (DS_PBFT_Peer &newMember)                   {_groupMembers[newMember.id()] = &newMember;};
	void                        addCommitteeMember      (DS_PBFT_Peer &newMember)                   {_committeeMembers[newMember.id()] = &newMember;};

	void                        printGroupOn            ()                                              {_printGroup = true;};
	void                        printGroupOff           ()                                              {_printGroup = false;};
	void                        printCommitteeOn        ()                                              {_printCommittee = true;};
	void                        printCommitteeOff       ()                                              {_printCommittee = false;};

	// mutators
	void                        clearCommittee          ()                                              {_committeeMembers.clear(); _committeeId = -1; _currentView = 0;}
	void                        clearGroup              ()                                              {_groupMembers.clear(); _groupId = -1;}
	void                        initPrimary             () override                                     {_primary = findPrimary(_committeeMembers);};
	void                        preformComputation      () override;

	// getters
	int                         faultyPeers             ()const override                                {return ceil(double(_committeeMembers.size() + 1) * _faultUpperBound);};
	int                         getGroup                ()const                                         {return _groupId;};
	int                         getCommittee            ()const                                         {return _committeeId;};
	std::vector<std::string>    getGroupMembers         ()const;
	std::vector<std::string>    getCommitteeMembers     ()const;
	std::vector<int>            getCommitteeSizes       ()const                                         {return _committeeSizes;};

	std::ostream&               printTo                 (std::ostream&)const;
	void                        log                     ()const                                         {printTo(*_log);};

	DS_PBFT_Peer&           operator=               (const DS_PBFT_Peer&);
	friend std::ostream&        operator<<              (std::ostream &o, const DS_PBFT_Peer &p)    {p.printTo(o); return o;};



//	new

	void 											setDAG									(const DAG &dagChain) { this->dag = dagChain; }
	DAG                             				getDAG                           		() { return this->dag; }
	void 											setTerminated								(bool flag){ this->terminated = flag;}
	void 											receiveTx									();
	void                                    		sendBlock									();
	void											updateDAG									();
	std::vector<DAGBlock>							dagBlocks={};
	DAGBlock										*minedBlock = nullptr;
	void 											setCommitteeNeighbours					(std::map<std::string, Peer<PBFT_Message>* > n) { _committeeMembers = std::move(n); }
	void 											createBlock();

	bool 											terminated = true;
	bool 											getTerminationFlag							() const { return terminated; }

//	void											refreshPeer();

};

DS_PBFT_Peer::DS_PBFT_Peer(std::string id) : PBFT_Peer(id) {
	_groupId = -1;
	_committeeId = -1;
	_groupMembers = std::map<std::string, Peer<PBFT_Message>* >();
	_committeeMembers = std::map<std::string, Peer<PBFT_Message>* >();
	_printCommittee = false;
	_printGroup = false;
	_committeeSizes = std::vector<int>();
}

DS_PBFT_Peer::DS_PBFT_Peer(const DS_PBFT_Peer &rhs) : PBFT_Peer(rhs){
	_groupId = rhs._groupId;
	_committeeId = rhs._committeeId;
	_groupMembers = rhs._groupMembers;
	_committeeMembers = rhs._committeeMembers;
	_printCommittee = rhs._printCommittee;
	_printGroup = rhs._printGroup;
	_committeeSizes = rhs._committeeSizes;
}

void DS_PBFT_Peer::braodcast(const PBFT_Message &msg){
	for (auto it=_committeeMembers.begin(); it!=_committeeMembers.end(); ++it){
		std::string neighborId = it->first;
		Packet<PBFT_Message> pck(makePckId());
		pck.setSource(_id);
		pck.setTarget(neighborId);
		pck.setBody(msg);
		_outStream.push_back(pck);
	}
}

void DS_PBFT_Peer::commitRequest(){
	PBFT_Message commit = _currentRequest;
	commit.result = _currentRequestResult;
	commit.commit_round = _clock;

	int numberOfByzantineCommits = 0;
	int correctCommitMsg = 0;

//	new
	bool viewChanged = false;
	std::vector<PBFT_Message> tmp = std::vector<PBFT_Message>();


	for(auto commitMsg = _commitLog.begin(); commitMsg != _commitLog.end(); commitMsg++){
		if(commitMsg->sequenceNumber == _currentRequest.sequenceNumber
		   && commitMsg->view == _currentView
		   && commitMsg->result == _currentRequestResult){
			if(commitMsg->byzantine){
				numberOfByzantineCommits++;
			}else{
				correctCommitMsg++;
			}
		}
	}

	// if we dont have enough honest commits and we have not heard from everyone wait
	if(correctCommitMsg <= faultyPeers()){
		if (correctCommitMsg + numberOfByzantineCommits != _committeeMembers.size() + 1){
			return;
		}
	}

	if(_currentRequest.byzantine && correctCommitMsg >= faultyPeers()){
		// clear messages from old view
		int oldTransaction = _currentRequest.sequenceNumber;
		int view = _currentView;
		auto entry = _prePrepareLog.begin();
		while(entry != _prePrepareLog.end()){
			if(entry->sequenceNumber == oldTransaction && entry->view == view){
				_prePrepareLog.erase(entry++);
			}else{
				entry++;
			}
		}

		entry = _prepareLog.begin();
		while(entry != _prepareLog.end()){
			if(entry->sequenceNumber == oldTransaction && entry->view == view){
				_prepareLog.erase(entry++);
			}else{
				entry++;
			}
		}

		entry = _commitLog.begin();
		while(entry != _commitLog.end()){
			if(entry->sequenceNumber == oldTransaction && entry->view == view){
				_commitLog.erase(entry++);
			}else{
				entry++;
			}
		}
		viewChange(_committeeMembers);
		viewChanged = true;
	}else if(!_currentRequest.byzantine && correctCommitMsg >= faultyPeers()){
		commit.defeated = false;
		_ledger.push_back(commit);
		_committeeSizes.push_back(_committeeMembers.size()+1);// +1 for self
		_currentRequest = PBFT_Message();
		clearCommittee();
		_currentRequest = PBFT_Message();
	}else{
		commit.defeated = true;
		_ledger.push_back(commit);
		_committeeSizes.push_back(_committeeMembers.size()+1);// +1 for self
		_currentRequest = PBFT_Message();
		clearCommittee();
		_currentRequest = PBFT_Message();
	}

	for(auto confirmedTransaction = _ledger.begin(); confirmedTransaction != _ledger.end(); confirmedTransaction++){
		cleanLogs(confirmedTransaction->sequenceNumber);
	}
	_currentPhase = IDEAL; // complete distributed-consensus
	_currentRequestResult = 0;


	//	newly added create a block once consensus is reached

	if(!viewChanged){
		terminated = true;
		createBlock();
	}else{
		Logger::instance()->log("VIEW CHANGED\n");
	}
}

std::vector<std::string> DS_PBFT_Peer::getGroupMembers()const{
	std::vector<std::string> groupIds = std::vector<std::string>();
	for (auto it=_groupMembers.begin(); it!=_groupMembers.end(); ++it){
		groupIds.push_back(it->first);
	}
	return groupIds;
};

std::vector<std::string> DS_PBFT_Peer::getCommitteeMembers()const{
	std::vector<std::string> committeeIds = std::vector<std::string>();
	for (auto it=_committeeMembers.begin(); it!=_committeeMembers.end(); ++it){
		committeeIds.push_back(it->first);
	}
	return committeeIds;
};

void DS_PBFT_Peer::preformComputation(){
	if(_primary == nullptr){
		_primary = findPrimary(_committeeMembers);
	}
	//_committeeMembers[_primary->id()];
	collectMessages(); // sorts messages into there repective logs
	prePrepare();
	prepare();
	waitPrepare();
	commit();
	waitCommit();
	for(auto confirmedTransaction = _ledger.begin(); confirmedTransaction != _ledger.end(); confirmedTransaction++){
		cleanLogs(confirmedTransaction->sequenceNumber);
	}
}

DS_PBFT_Peer& DS_PBFT_Peer::operator= (const DS_PBFT_Peer &rhs){
	PBFT_Peer::operator=(rhs);

	_groupId = rhs._groupId;
	_committeeId = rhs._committeeId;
	_groupMembers = rhs._groupMembers;
	_committeeMembers = rhs._committeeMembers;
	_printCommittee = rhs._printCommittee;
	_printGroup = rhs._printGroup;
	_committeeSizes = rhs._committeeSizes;

	return *this;
}

std::ostream& DS_PBFT_Peer::printTo(std::ostream &out)const{
	PBFT_Peer::printTo(out);

	out<< "\t"<< std::setw(LOG_WIDTH)<< "Group Id"<< std::setw(LOG_WIDTH)<< "Committee Id"<< std::setw(LOG_WIDTH)<< "Group Size"<< std::setw(LOG_WIDTH)<< "Committee Size"<<  std::endl;
	out<< "\t"<< std::setw(LOG_WIDTH)<< _groupId<< std::setw(LOG_WIDTH)<< _committeeId<< std::setw(LOG_WIDTH)<< _groupMembers.size() + 1<< std::setw(LOG_WIDTH)<< _committeeMembers.size() + 1<< std::endl<< std::endl;

	std::vector<std::string> groupMembersIds = std::vector<std::string>();
	for (auto it=_groupMembers.begin(); it!=_groupMembers.end(); ++it){
		groupMembersIds.push_back(it->first);
	}

	std::vector<std::string> committeeMembersIds = std::vector<std::string>();
	for (auto it=_committeeMembers.begin(); it!=_committeeMembers.end(); ++it){
		committeeMembersIds.push_back(it->first);
	}

	while(committeeMembersIds.size() > groupMembersIds.size()){
		groupMembersIds.push_back("");
	}
	while(groupMembersIds.size() > committeeMembersIds.size()){
		committeeMembersIds.push_back("");
	}

	if(_printCommittee || _printGroup){
		out<< "\t"<< std::setw(LOG_WIDTH)<< "Group Members"<< std::setw(LOG_WIDTH)<< "Committee Members"<< std::endl;
	}
	for(int i = 0; i < committeeMembersIds.size(); i++){
		if(_printCommittee && _printGroup){
			out<< "\t"<< std::setw(LOG_WIDTH)<< groupMembersIds[i]<< std::setw(LOG_WIDTH)<< committeeMembersIds[i]<< std::endl;
		}else if(_printCommittee){
			out<< "\t"<< std::setw(LOG_WIDTH)<< ""<< std::setw(LOG_WIDTH)<< committeeMembersIds[i]<< std::endl;
		}else if(_printGroup){
			out<< "\t"<< std::setw(LOG_WIDTH)<< groupMembersIds[i]<< std::setw(LOG_WIDTH)<< ""<< std::endl;
		}
	}

	return out;
}

void DS_PBFT_Peer::sendBlock(){
	for(auto & _neighbor : _committeeMembers) {
		assert(_neighbor.second->id()!= id());
		PBFT_Message messageToSend;
		Peer<PBFT_Message> *peer = _neighbor.second;
		Packet<PBFT_Message> newMessage("", peer->id(), _id);
		messageToSend.client_id = id();
//todo add block to the message;
		messageToSend.dagBlock = *minedBlock;
		messageToSend.dagBlockMsg = true;
		messageToSend.creator_id = id();
		newMessage.setBody(messageToSend);
		_outStream.push_back(newMessage);
	}
}

void DS_PBFT_Peer::updateDAG() {
	Logger::instance()->log("UPDATING DAG IN PEER " + id() + "\n");
	Logger::instance()->log("BEFORE UPDATE DAG SIZE IS " + std::to_string(dag.getSize()) + "\n");
	//process self mined block
	if(minedBlock!= nullptr){
		dagBlocks.push_back(*minedBlock);
		delete minedBlock;
		minedBlock = nullptr;
	}
	//resolve the dag
	for(auto & i : _inStream){
//		this flag added to distinguish block msg from other messages.
		if(i.getMessage().dagBlockMsg){
			Logger::instance()->log("BLOCK FROM " + i.getMessage().client_id + " TO " + id() + "\n");
			//todo what will happen if a mined block is received before or at the same time a transaction is received.
			/*if(i.getMessage().txFlag){
				//check to see if the transaction has already been added to the dag
				if(dag.transactionInDag(i.getMessage().message[0]))
					continue;
				else{
					consensusQueue.push_back(i.getMessage().message[0]);
					assert(false);
				}
				continue;
			}
			assert(!i.getMessage().txFlag);
			if(i.getMessage().dagBlockFlag){
				dagBlocks.push_back(i.getMessage().dagBlock);
			}*/
//todo no txFLag dagBlockFlag only message
			dagBlocks.push_back(i.getMessage().dagBlock);
		}
	}

	if(!dagBlocks.empty()){
		sort(dagBlocks.begin(), dagBlocks.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs.getData() < rhs.getData();
		});

		for(auto & dagBlock : dagBlocks){
			dag.addVertex(dagBlock, dagBlock.getPreviousHashes(), dagBlock.isByzantine());
		}
	}
	dag.setTips();
	dagBlocks.clear();
	_inStream.clear();
	Logger::instance()->log("AFTER UPDATE DAG SIZE IS " + std::to_string(dag.getSize()) + "\n");
}

void DS_PBFT_Peer::createBlock(){
	std::vector<string> hashesToConnectTo = dag.getTips();
	std::string newBlockString;

	for (auto const& s : hashesToConnectTo) { newBlockString += "_" + s;}
	int seqNumber = _ledger.back().sequenceNumber;
	newBlockString += "_" + std::to_string(seqNumber);
//	string newBlockHash = sha256(newBlockString);
	string newBlockHash = std::to_string(dag.getSize());

	minedBlock = new DAGBlock(dag.createBlock(dag.getSize(), hashesToConnectTo, newBlockHash, {id()}, std::to_string(seqNumber), _byzantine));
}

void DS_PBFT_Peer::receiveTx() {
//	can receive from itself
//	nothing here, transactions are mathematical calculations, just keep delaying until twice max delay.
	_inStream.clear();
}

#endif /* PBFTPeer_Sharded_hpp */
