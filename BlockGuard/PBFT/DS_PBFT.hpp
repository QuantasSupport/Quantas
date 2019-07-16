//
// Created by srai on 6/27/19.
//

#ifndef DS_PBFT_hpp
#define DS_PBFT_hpp

#include "../Common/ByzantineNetwork.hpp"
#include "DS_PBFT_Peer.hpp"
#include "PBFT_Committee.hpp"
#include <iostream>
#include <random>
#include <stdio.h>
#include <assert.h>
#include <cassert>


struct txRequest{
	double securityLevel;
	int submissionRound;
	int sequenceNumber;
};

class DS_PBFT {
protected:

	double _securityLevel5;
	double _securityLevel4;
	double _securityLevel3;
	double _securityLevel2;
	double _securityLevel1;

	int                                                             _currentRound;
	int                                                             _groupSize;
	int                                                             _nextCommitteeId;
	int                                                             _nextSquenceNumber;
	double                                                          _faultTolerance;
	ByzantineNetwork<PBFT_Message, DS_PBFT_Peer>                	_peers;
	std::vector<int>                                                _groupIds;
	std::vector<int>                                                _busyGroups;
	std::vector<int>                                                _freeGroups;
	std::vector<txRequest>                                 			_requestQueue;
	std::map<int,aGroup>                                            _groups;

	// logging, metrics and untils
	int                                                             _totalTransactionsSubmitted;
	std::ostream                                                    *_log;
	std::default_random_engine                                      _randomGenerator;
	std::vector<int>                                                _currentCommittees;
	bool                                                            _printNetwork;
	int 															collectInterval = 0;
	int 															txWaitTime = 2 * _peers.maxDelay();

	// util functions
	txRequest                  			generateRequest         	();
	txRequest                  			generateRequest         	(int); // generate a request of a specific security level
	void                                initCommittee           	(std::vector<int>);
	double                              pickSecrityLevel        	();

//	void                                makeGroup               	(std::vector<DS_PBFT_Peer*>,int);
//	void                                makeCommittee           	(std::vector<int>);
//	void                                updateBusyGroup         	();


public:
	DS_PBFT                                      ();
	DS_PBFT                                      (const DS_PBFT&);
	~DS_PBFT                                     ()                                      {
		for(auto committee : currentCommittees){
			delete committee;
		}
	};

	// setters
	void                                setGroupSize            (int g)                                 {_groupSize = g;};
	void                                setFaultTolerance       (double);
	void                                setLog                  (std::ostream &o)                       {_log = &o; _peers.setLog(o);}
	void                                setSquenceNumber        (int s)                                 {_nextSquenceNumber = s;}

	// getters
	int                                 getGroupSize            ()const                                 {return _groupSize;};
	int                                 numberOfGroups          ()const                                 {return (int)_groupIds.size();};
	int                                 size                    ()const                                 {return _peers.size();}
	int                                 getNextSquenceNumber    ()const                                 {return _nextSquenceNumber;};
	int                                 getNextCommitteeId      ()const                                 {return _nextCommitteeId;};
	double                              securityLevel5          ()const                                 {return _securityLevel5;}
	double                              securityLevel4          ()const                                 {return _securityLevel4;}
	double                              securityLevel3          ()const                                 {return _securityLevel3;}
	double                              securityLevel2          ()const                                 {return _securityLevel2;}
	double                              securityLevel1          ()const                                 {return _securityLevel1;}
	int                                 totalSubmissions        ()const                                 {return _totalTransactionsSubmitted;};

	aGroup                              getGroup                (int)const;
	std::vector<int>                    getGroupIds             ()const                                 {return _groupIds;};
	std::vector<DS_PBFT_Peer>       	getPeers                ()const;
	std::vector<int>                    getBusyGroups           ()const                                 {return _busyGroups;};
	std::vector<int>                    getFreeGroups           ()const                                 {return _freeGroups;};
	std::vector<int>                    getCurrentCommittees    ()const                                 {return _currentCommittees;};
	std::vector<txRequest>     			getRequestQueue         ()const                                 {return _requestQueue;}
	std::vector<aGroup>                 getCommittee            (int)const;

	// mutators
	void                                initNetwork             (int);
	void                                serveRequest            ();
	void                                makeRequest             ()                                      {queueRequest();serveRequest();};
//	void                                makeRequest             (int securityLevel)                     {queueRequest(securityLevel);serveRequest();};
	void                                makeRequest             (int securityLevel)                     {queueRequest(securityLevel);};
	void                                queueRequest            ()                                      {_requestQueue.push_back(generateRequest()); _totalTransactionsSubmitted++;};
	void                                queueRequest            (int securityLevel)                     {_requestQueue.push_back(generateRequest(securityLevel)); _totalTransactionsSubmitted++;};
	void                                clearQueue              ()                                      {_requestQueue.clear();}
	void                                setMaxSecurityLevel     (int); // used to fix max security as number of groups for debugging
	void                                setMinSecurityLevel     (int); // used to fix min security as number of groups for debugging
	void                                printNetworkOn          ()                                      {_printNetwork = true;};
	void                                printNetworkOff         ()                                      {_printNetwork = false;};

	// pass-through to ByzantineNetwork class
	void                                receive                 ()                                      {_peers.receive();};
	void                                preformComputation      ()                                      {_peers.preformComputation();/*updateBusyGroup()*/; _currentRound++;};
	void                                transmit                ()                                      {_peers.transmit();};
	void                                setMaxDelay             (int d)                                 {_peers.setMaxDelay(d);};
	void                                setAvgDelay             (int d)                                 {_peers.setAvgDelay(d);};
	void                                setMinDelay             (int d)                                 {_peers.setMinDelay(d);};
	void                                setToPoisson            ()                                      {_peers.setToPoisson();};
	void                                setToOne                ()                                      {_peers.setToOne();};
	void                                setToRandom             ()                                      {_peers.setToRandom();};
	void                                shuffleByzantines       (int n);
	std::vector<DS_PBFT_Peer*>      	getByzantine            ()const                                 {return _peers.getByzantine();};
	std::vector<DS_PBFT_Peer*>      	getCorrect              ()const                                 {return _peers.getCorrect();};
	void                                makeByzantines          (int n)                                 {_peers.makeByzantines(n);};
	void                                makeCorrect             (int n)                                 {_peers.makeCorrect(n);};
	void                                makePeerByzantines      (int i)                                 {_peers.makePeerByzantines(i);};
	void                                makePeerCorrect         (int i)                                 {_peers.makePeerByzantines(i);};


	// logging and debugging
	std::ostream&                       printTo                 (std::ostream&)const;
	void                                log                     ()const                                 {printTo(*_log);};

	// metrics
	std::vector<ledgerEntery>           getGlobalLedger         ()const;

	// operators
	DS_PBFT&             				operator=               (const DS_PBFT&);
	DS_PBFT_Peer*                   	operator[]              (int i)                                 {return _peers[i];};
	const DS_PBFT_Peer*             	operator[]              (int i)const                            {return _peers[i];};
	friend std::ostream&                operator<<              (std::ostream &out, // continued on next line
																 const DS_PBFT&system)    {return system.printTo(out);};


	void								run						(int);
	void								setDelay				(int d){delay = d;};

	std::vector<vector<DS_PBFT_Peer*>> 							consensusGroups{};
	std::vector<PBFT_Committee *> 								currentCommittees{};
	std::string 												status;
	int 														delay = 0;
	int 														sequenceNumber = 0;
	int 														byzantineOrNot = 1;

	bool 														shufflePeers = false;


	std::map<int, int> 											securityLevelCount{};
	std::map<int, int> 											defeatedCommittee{};
	std::vector<int> 											consensusGroupCount{};

	std::map<int,std::map<int, int>> 							averageWaitingTimeByIteration{};
	std::map<int,std::vector<PBFT_Message>> 					confirmedMessagesPerIteration{};

	std::vector<int> 											defeatedCommittees{};
	std::vector<int> 											totalCommittees{};

};


DS_PBFT::DS_PBFT(){
	_totalTransactionsSubmitted = 0;
	_currentRound = 0;
	_groupSize = -1;
	_peers = ByzantineNetwork<PBFT_Message, DS_PBFT_Peer>();
	_busyGroups = std::vector<int>();
	_freeGroups = std::vector<int>();
	_groupIds = std::vector<int>();
	_nextCommitteeId = 0;
	_nextSquenceNumber = 0;
	_requestQueue = std::vector<txRequest>();
	_groups = std::map<int,aGroup>();
	_faultTolerance = 1;
	_currentCommittees = std::vector<int>();
	_log = nullptr;
	_printNetwork = false;

	int seed = (int)time(nullptr);
	_randomGenerator = std::default_random_engine(seed);
}

DS_PBFT::DS_PBFT(const DS_PBFT &rhs){
	_securityLevel5 = rhs._securityLevel5;
	_securityLevel4 = rhs._securityLevel4;
	_securityLevel3 = rhs._securityLevel3;
	_securityLevel2 = rhs._securityLevel2;
	_securityLevel1 = rhs._securityLevel1;
	_currentRound = rhs._currentRound;
	_groupSize = rhs._groupSize;
	_peers = rhs._peers;
	_busyGroups = rhs._busyGroups;
	_freeGroups = rhs._freeGroups;
	_groupIds = rhs._groupIds;
	_nextCommitteeId = rhs._nextCommitteeId;
	_nextSquenceNumber = rhs._nextSquenceNumber;
	_requestQueue = rhs._requestQueue;
	_groups = rhs._groups;
	_faultTolerance = rhs._faultTolerance;
	_currentCommittees = rhs._currentCommittees;
	_log = rhs._log;
	_totalTransactionsSubmitted = rhs._totalTransactionsSubmitted;
	_printNetwork = rhs._printNetwork;

	int seed = (int)time(nullptr);
	_randomGenerator = std::default_random_engine(seed);
}

/*
void DS_PBFT::makeGroup(std::vector<PBFTPeer_Sharded*> group, int id){
	for(int i = 0; i < group.size(); i++){
		group[i]->setGroup(id);
		for(int j = 0; j < group.size(); j++){
			if(group[i]->id() != group[j]->id()){
				group[i]->addGroupMember(*group[j]);
			}
		}
	}
	_freeGroups.push_back(id);
	_groupIds.push_back(id);
	_groups[id] = group;
}
*/

void DS_PBFT:: initNetwork(int numberOfPeers){

	// 5 is high (all groups )1 is low (4 groups for 1024 peers)
	_securityLevel5 = numberOfPeers;
	_securityLevel4 = _securityLevel5/2;
	_securityLevel3 = _securityLevel4/2;
	_securityLevel2 = _securityLevel3/2;
	_securityLevel1 = _securityLevel2/2;

	_peers.initNetwork(numberOfPeers);
	_peers.buildInitialDAG();
/*
	int groupId = 0;
	std::vector<PBFTPeer_Sharded*> group = std::vector<PBFTPeer_Sharded*>();

	for(int i = 0; i < _peers.size(); i++){
		group.push_back(_peers[i]);

		if(group.size() == _groupSize){
			makeGroup(group,groupId);
			group = std::vector<PBFTPeer_Sharded*>();
			groupId++;
		}
	}

	// connects left over peers into s small group
	if(!group.empty()){
		makeGroup(group,groupId);
	}*/
	setFaultTolerance(_faultTolerance);
	return;
}

double DS_PBFT::pickSecrityLevel(){

	std::uniform_int_distribution<int> coin(0,1);
	int trails = 0;
	int heads = coin(_randomGenerator);
	while(!heads){
		trails++;
		heads = coin(_randomGenerator);
	}

	switch (trails) {
		case 0: return _securityLevel1; break;
		case 1: return _securityLevel2; break;
		case 2: return _securityLevel3; break;
		case 3: return _securityLevel4; break;
		case 4: return _securityLevel5; break;

		default: return _securityLevel5; break;
	}
}

txRequest DS_PBFT::generateRequest(){
	txRequest request;

	request.securityLevel = pickSecrityLevel();
	request.submissionRound = _currentRound;
	request.sequenceNumber = ++sequenceNumber;

	return request;
}

txRequest DS_PBFT::generateRequest(int securityLevel){
	assert(false);
	txRequest request;
	request.securityLevel = securityLevel;

	request.submissionRound = _currentRound;

	return request;
}

void DS_PBFT::serveRequest(){
	/*if(_requestQueue.empty()){
		return;
	}
	int groupsNeeded = std::ceil(_requestQueue.front().securityLevel);
	updateBusyGroup();

	// return if there is not enough free groups to make the committee
	if(_freeGroups.size() < groupsNeeded){
		return;
	}
	_requestQueue.erase(_requestQueue.begin());

	std::vector<int> groupsInCommittee = std::vector<int>();
	while(groupsInCommittee.size() < groupsNeeded){
		int groupId = _freeGroups.back();
		_busyGroups.push_back(groupId);
		groupsInCommittee.push_back(groupId);
		_freeGroups.pop_back();
	}

	makeCommittee(groupsInCommittee);
	initCommittee(groupsInCommittee);

	for(int i = 0; i < groupsInCommittee.size(); i++){
		aGroup group = getGroup(groupsInCommittee[i]);
		for(int j = 0; j < group.size(); j++){
			if(group[j]->isPrimary()){
				group[j]->makeRequest(_nextSquenceNumber);
				_nextSquenceNumber++;
				return;
			}
		}
	}*/
}


void DS_PBFT::run(int iter){
//	todo instead of in preformComputation, placed here
	_currentRound++;
	if(status == WAITING_FOR_TX){
		//	wait for max delay until all committees receive their transactions
		if(txWaitTime>=0 ){
			for(auto & currentCommittee : currentCommittees) {
				currentCommittee->receive();
			}
			txWaitTime--;
		}

		if(txWaitTime == 0){
			for(auto & currentCommittee : currentCommittees) {
				currentCommittee->receiveTx();
			}
			status = MINING;
			txWaitTime = 2*_peers.maxDelay();
		}

	}else if(status == MINING){
		for(auto & currentCommittee : currentCommittees){
			currentCommittee->receive();
			currentCommittee->preformComputation();
			currentCommittee->transmit();
		}

		//	don't erase but check to see if consensus reached in all
		bool consensus = true;
		auto it = currentCommittees.begin();
		while(it != currentCommittees.end()) {
			if(!(*it)->getConsensusFlag()) {
				consensus = false;
				break;
			}

			++it;
		}


		if (consensus){
//				calculate average waiting time
			int waitingTime = 0;
			int numOfConfirmation = 0;
			std::vector<PBFT_Message> confirmedMessages;
			for(auto & currentCommittee : currentCommittees){
				waitingTime+= iter - currentCommittee->commitMsg->submission_round;
				numOfConfirmation++;
				confirmedMessages.push_back(*currentCommittee->commitMsg);
			}

			//				defeated Committee and number of committee formed by security level
			for(auto & currentCommittee : currentCommittees){
				if(currentCommittee->defeated)
					defeatedCommittees.push_back(currentCommittee->getSecurityLevel());
				totalCommittees.push_back(currentCommittee->getSecurityLevel());
			}
			confirmedMessagesPerIteration[iter] = confirmedMessages;
			averageWaitingTimeByIteration[iter][numOfConfirmation] = waitingTime;
			for(auto & currentCommittee : currentCommittees){
				currentCommittee->refreshPeers();
			}
			//	send blocks
			for(auto & currentCommittee : currentCommittees){
				//	propogate the block to whole network except the committee itself.
				//	set neighbours
				std::vector<std::string> peerIds = currentCommittee->getCommitteePeerIds();
//				std::map<std::string, DS_PBFT_Peer* > neighbours;
				std::vector<DS_PBFT_Peer*> neighbors =  std::vector<DS_PBFT_Peer*>();


				DS_PBFT_Peer * minerPeer = currentCommittee->getCommitteePeers()[currentCommittee->getFirstMinerIndex()];

				for(int j = 0; j < _peers.size(); j++) {
					if(minerPeer->id()!=_peers[j]->id()) {
						if (std::find(peerIds.begin(), peerIds.end(), _peers[j]->id()) != peerIds.end()) {
						} else {
//							neighbours[_peers[j]->id()] = _peers[j];
							neighbors.push_back(_peers[j]);
						}
					}
				}

				if(currentCommittee->size()!=_peers.size())
					assert(!neighbors.empty());


				for(auto &neighbour: neighbors){
					minerPeer->addCommitteeMember(*neighbour);
				}



//				minerPeer->setCommitteeNeighbours(neighbours);
				minerPeer->sendBlock();
				minerPeer->transmit();
			}
			for(auto committee: currentCommittees ){
				delete committee;
			}
			currentCommittees.clear();

			status = COLLECTING;

			collectInterval = 2 * delay;
		}

	}else if(status == COLLECTING){
		//	make sure no peer is busy
		for(int a = 0; a< _peers.size();a++){
			assert(!_peers[a]->isBusy());
//			assert(_peers[a]->getConsensusTx().empty());
		}

		assert (currentCommittees.empty());

		if(collectInterval > 0){
			_peers.receive();

			if(--collectInterval == 0){
				Logger::instance()->log("CALLING UPDATE DAG\n");
				for(int index = 0; index< _peers.size();index++){
					_peers[index]->updateDAG();
				}
			}else{
				//	skip to next duration
//				continue;
				return;
			}

		}

		//	shuffling byzantines
		if(byzantineOrNot==1){
			Logger::instance()->log("Shuffling " + std::to_string(size()/10) + " Peers.\n");
			_peers.shuffleByzantines (size()/10);
			//	n.shuffleByzantines (1);
		}

		if(!_requestQueue.empty()){
			//	select a random peer
			int randIndex;
			DS_PBFT_Peer *p;

			std::vector<DS_PBFT_Peer*> consensusGroup;
			int concurrentGroupCount = 0;
			do{
				int securityLevel = std::ceil(_requestQueue.front().securityLevel);
//				int securityLevel = n.pickSecurityLevel(peersCount);
				randIndex = rand()%_peers.size();

				p = _peers[randIndex];
				consensusGroup = _peers.setPeersForConsensusDAG(p, securityLevel);
				if(!consensusGroup.empty())
					//create a committee if only a consensus group is formed.
					if(!consensusGroup.empty()){
						PBFT_Committee *co = new PBFT_Committee(consensusGroup, p, std::to_string(_requestQueue.front().submissionRound), securityLevel, _requestQueue.front().sequenceNumber, _nextCommitteeId++, _requestQueue.front().submissionRound);
						Logger::instance()->log("COMMITTEE FORMED"+co->getCommitteeId()+"\n");
						Logger::instance()->log("BYZANTINE RATION IS " + std::to_string(co->getByzantineRatio())+"\n");
						concurrentGroupCount++;
//						_requestQueue.pop_front();
						_requestQueue.erase(_requestQueue.begin());
						currentCommittees.push_back(co);
						consensusGroups.push_back(consensusGroup);
						securityLevelCount[securityLevel]++;
						if(co->getByzantineRatio()>=0.5){
							defeatedCommittee[securityLevel]++;
						}
					}
			}while(!consensusGroup.empty() && !_requestQueue.empty()); //build committees until a busy peer is jumped on.
			Logger::instance()->log("CONCURRENT COMMITTEE SIZE = " + std::to_string(concurrentGroupCount) + "\n");
			consensusGroupCount.push_back(concurrentGroupCount);
		}

		Logger::instance()->log("DONE WITH CONSENSUS GROUP FORMING.\n");

//		Logger::instance()->log("COLLECTION COMPLETE IN ITERATION " + std::to_string(i) + ":\t START MINING \n");
		if(!currentCommittees.empty()){
			for(auto &committee: currentCommittees){
				committee->initiate();
			}
//			status = MINING;
			status = WAITING_FOR_TX;
		}

		/*for(int i = 0; i < groupsInCommittee.size(); i++){
			aGroup group = getGroup(groupsInCommittee[i]);
			for(int j = 0; j < group.size(); j++){
				if(group[j]->isPrimary()){
					group[j]->makeRequest(_nextSquenceNumber);
					_nextSquenceNumber++;
					return;
				}
			}
		}*/
	}
}
/*void DS_PBFT::updateBusyGroup(){
	std::vector<int> aliveCommittees = std::vector<int>(); // if a group is still bust we need the committee it belongs to
	// check each busy group to see if they are still busy
	auto id = _busyGroups.begin();
	while(id != _busyGroups.end()){
		aGroup group= getGroup(*id);
		bool stillBusy = false;
		for(int i = 0; i < group.size(); i++){
			// if the group is still busy it will still have a committee id and thus we need to track it
			if(group[i]->getCommittee() != -1){
				stillBusy = true;
				// find works in n^2 time and lower bound is log(n) if sorted
				auto found = std::lower_bound(aliveCommittees.begin(),aliveCommittees.end(),group[i]->getCommittee());
				if( found == aliveCommittees.end()){
					aliveCommittees.push_back(group[i]->getCommittee());
					std::sort(aliveCommittees.begin(),aliveCommittees.end()); // sort for next check
				}
				break; // no need to check rest of group
			}
		}
		// if the group is not still alive then we need to move its group id to free and remove from busy
		if(!stillBusy){
			_freeGroups.push_back(*id);
			_busyGroups.erase(id);
			// else we just continue
		}else{
			id++;
		}
	}

	_currentCommittees = aliveCommittees; // update list of currently active committees
}*/

std::vector<aGroup> DS_PBFT::getCommittee(int committeeId)const{
	std::vector<aGroup> committee = std::vector<aGroup>();

	// need to find every group that belongs to committee id
	for(auto id = _groupIds.begin(); id != _groupIds.end(); id++){
		aGroup group = getGroup(*id);
		// check each group member to see if they belong to that committee still
		bool inCommittee = true;
		for(auto peer = group.begin(); peer != group.end(); peer++){
			if((*peer)->getCommittee() != committeeId){
				inCommittee = false;
			}
		}
		if(inCommittee){
			committee.push_back(group);
		}
	}

	return committee;
}

void DS_PBFT::initCommittee(std::vector<int> groupsInCommittee){
	for(auto groupId = groupsInCommittee.begin(); groupId != groupsInCommittee.end(); groupId++){
		aGroup groupInCommittee = getGroup(*groupId);
		for(int peerIndex = 0; peerIndex < groupInCommittee.size(); peerIndex++){
			groupInCommittee[peerIndex]->clearPrimary();
			groupInCommittee[peerIndex]->initPrimary();
		}
	}
}

/*

void DS_PBFT::makeCommittee(std::vector<int> groupsForCommittee){
	// gets list of all committee members
	std::vector<DS_PBFT_Peer*> committeeMembers =  std::vector<DS_PBFT_Peer*>();
	for(auto groupId = groupsForCommittee.begin(); groupId != groupsForCommittee.end(); groupId++){
		aGroup groupInCommitte = getGroup(*groupId);
		for(int peerIndex = 0; peerIndex < groupInCommitte.size(); peerIndex++){
			committeeMembers.push_back(groupInCommitte[peerIndex]);
		}
	}

	// clear old committee
	for(int peer = 0; peer < committeeMembers.size(); peer++){
		committeeMembers[peer]->clearCommittee();
	}

	// joins them togeather
	for(int peer = 0; peer < committeeMembers.size(); peer++){
		committeeMembers[peer]->setCommittee(_nextCommitteeId);
		for(int otherPeer = 0; otherPeer < committeeMembers.size(); otherPeer++){
			if(!(committeeMembers[peer]->id() == committeeMembers[otherPeer]->id())){
				committeeMembers[peer]->addCommitteeMember(*committeeMembers[otherPeer]);
			}
		}
	}
	_currentCommittees.push_back(_nextCommitteeId);
	_nextCommitteeId++;
}
*/

void DS_PBFT::setFaultTolerance(double f){
	_faultTolerance = f;
	for(int i = 0; i < _peers.size(); i++){
		_peers[i]->setFaultTolerance(_faultTolerance);
	}
}

aGroup DS_PBFT::getGroup(int id)const{
	return _groups.at(id);
}

std::vector<DS_PBFT_Peer> DS_PBFT::getPeers()const{
	std::vector<DS_PBFT_Peer> peers = std::vector<DS_PBFT_Peer>();
	for(int i = 0; i < _peers.size(); i++){
		peers.push_back(*_peers[i]);
	}
	return peers;
}

std::vector<ledgerEntery> DS_PBFT::getGlobalLedger()const{
	std::vector<ledgerEntery> globalLegder;
	for(int i = 0; i < _peers.size(); i++){
		auto localLedger = _peers[i]->getLedger();
		for(auto transaction = localLedger.begin(); transaction != localLedger.end(); transaction++){
			bool found = false;
			for(auto global = globalLegder.begin(); global != globalLegder.end(); global++){
				if(*transaction == global->first){
					found = true;
				}
			}
			if(!found){
				int sizeIndez = std::distance(localLedger.begin(),transaction);
				globalLegder.push_back(ledgerEntery(*transaction,_peers[i]->getCommitteeSizes()[sizeIndez]));
			}
		}
	}
	return globalLegder;
}

void DS_PBFT::setMaxSecurityLevel(int max){
	if(_securityLevel5 > max){
		_securityLevel5 = max;
	}
	if(_securityLevel4 > max){
		_securityLevel4 = max;
	}
	if(_securityLevel3 > max){
		_securityLevel3 = max;
	}
	if(_securityLevel2 > max){
		_securityLevel2 = max;
	}
	if(_securityLevel1 > max){
		_securityLevel1 = max;
	}
}

void DS_PBFT::setMinSecurityLevel(int min){
	if(_securityLevel5 < min){
		_securityLevel5 = min;
	}
	if(_securityLevel4 < min){
		_securityLevel4 = min;
	}
	if(_securityLevel3 < min){
		_securityLevel3 = min;
	}
	if(_securityLevel2 < min){
		_securityLevel2 = min;
	}
	if(_securityLevel1 < min){
		_securityLevel1 = min;
	}
}

void DS_PBFT::shuffleByzantines(int n){
	std::vector<int> correct = std::vector<int>();
	std::vector<int> byz = std::vector<int>();
	for(int i = 0; i < _peers.size(); i++){
		if(_peers[i]->getCommittee() == -1){
			if(_peers[i]->isByzantine()){
				byz.push_back(i);
			}else{
				correct.push_back(i);
			}
		}
	}
	if(correct.size() < n){
		n = correct.size();
	}
	if(byz.size() < n){
		n = byz.size();
	}

	int shuffleCount = 0;
	while(shuffleCount < n){
		int byzantineShuffleIndex = static_cast<int>(rand() % byz.size());
		int nonByzantineShuffleIndex = static_cast<int>(rand() % correct.size());
		_peers[byz[byzantineShuffleIndex]]->makeCorrect();
		_peers[correct[nonByzantineShuffleIndex]]->makeByzantine();
		shuffleCount++;
		byz.erase(byz.begin () + byzantineShuffleIndex);
		correct.erase(correct.begin () + nonByzantineShuffleIndex);
	}

}

DS_PBFT& DS_PBFT::operator=(const DS_PBFT &rhs){
	_securityLevel5 = rhs._securityLevel5;
	_securityLevel4 = rhs._securityLevel4;
	_securityLevel3 = rhs._securityLevel3;
	_securityLevel2 = rhs._securityLevel2;
	_securityLevel1 = rhs._securityLevel1;
	_currentRound = rhs._currentRound;
	_groupSize = rhs._groupSize;
	_peers = rhs._peers;
	_busyGroups = rhs._busyGroups;
	_freeGroups = rhs._freeGroups;
	_groupIds = rhs._groupIds;
	_nextCommitteeId = rhs._nextCommitteeId;
	_nextSquenceNumber = rhs._nextSquenceNumber;
	_requestQueue = rhs._requestQueue;
	_groups = rhs._groups;
	_faultTolerance = rhs._faultTolerance;
	_currentCommittees = rhs._currentCommittees;
	_log = rhs._log;
	_totalTransactionsSubmitted = rhs._totalTransactionsSubmitted;
	_printNetwork = rhs._printNetwork;

	int seed = (int)time(nullptr);
	_randomGenerator = std::default_random_engine(seed);

	return *this;
}

std::ostream& DS_PBFT::printTo(std::ostream& out)const{
	out<< "-- REFERENCE COMMITTEE SETUP --"<< std::endl<< std::endl;
	out<< std::left;
	out<< '\t'<< std::setw(LOG_WIDTH)<< "Current Round" << std::setw(LOG_WIDTH) << "Group Size" << std::setw(LOG_WIDTH) << "Number Of Groups"   << std::setw(LOG_WIDTH) << "Number Of Free Groups"  << std::setw(LOG_WIDTH) << "Number Of Busy Groups" << std::endl;
	out<< '\t'<< std::setw(LOG_WIDTH)<< _currentRound   << std::setw(LOG_WIDTH) << _groupSize   << std::setw(LOG_WIDTH) << _groupIds.size()     << std::setw(LOG_WIDTH) << _freeGroups.size()       << std::setw(LOG_WIDTH) << _busyGroups.size()<< std::endl;
	out<< '\t'<< std::setw(LOG_WIDTH)<<  "Request Queue Size"   << std::setw(LOG_WIDTH)<< "Next Committee Id"   << std::endl;
	out<< '\t'<< std::setw(LOG_WIDTH)<< _requestQueue.size()    << std::setw(LOG_WIDTH)<< _nextCommitteeId      << std::endl;
	out<<std::endl;
	out<< '\t'<<"Current Committees:"<< std::endl;
	for(auto id = _currentCommittees.begin(); id != _currentCommittees.end(); id++){
		std::vector<aGroup> committee = getCommittee(*id);
		out<< '\t'<< std::setw(LOG_WIDTH)<< "Committee ID:" + std::to_string(*id) << std::setw(LOG_WIDTH)<< "Current Committees Sizes:" + std::to_string(committee.size())<< std::endl;
		out<< '\t'<< std::setw(LOG_WIDTH)<< "Peer:"<< std::setw(LOG_WIDTH)<< "Current Phase:"<< std::setw(LOG_WIDTH)<< "Last Commit:"<< std::setw(LOG_WIDTH)<< "Current Committee"<< std::setw(LOG_WIDTH)<< "Current Primary"<< std::endl;
		for(auto group = committee.begin(); group != committee.end(); group++){
			for(auto peer = group->begin(); peer != group->end(); peer++ ){
				if((*peer)->getLedger().size() != 0){
					out<< '\t'<< std::setw(LOG_WIDTH)<< (*peer)->id()<< std::setw(LOG_WIDTH)<< (*peer)->getPhase()<< std::setw(LOG_WIDTH)<< (*peer)->getLedger().end()->commit_round<< std::setw(LOG_WIDTH)<< (*peer)->getCommittee()<< std::setw(LOG_WIDTH)<< (*peer)->getPrimary()<<std::endl;
				}else{
					out<< '\t'<< std::setw(LOG_WIDTH)<< (*peer)->id()<< std::setw(LOG_WIDTH)<< (*peer)->getPhase()<< std::setw(LOG_WIDTH)<< "None"<< std::setw(LOG_WIDTH)<< (*peer)->getCommittee()<<std::endl;
				}
			}
		}
	}

	if(_printNetwork){
		_peers.printTo(out);
	}

	return out;
}

// "Total Request:,Max Ledger:,Ratio Honest To Defeated,Average Waiting Time 1,Average Waiting Time 2,Average Waiting Time 3 ,Average Waiting Time 4,Average Waiting Time 5\n"
/*
void calculateResults(const std::vector<std::pair<PBFT_Message,int> > globalLedger){

}
*/


#endif //MULTI_LEVEL_SECURE_BLOCKCHAIN_DS_PBFT_H
