//
//  Network.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/24/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef Network_hpp
#define Network_hpp

#include <stdio.h>
#include <vector>
#include <string>
#include <random>
#include <limits>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <memory>
#include "Peer.hpp"
#include "DAG.hpp"

static const std::string                POISSON = "POISSON";
static const std::string                RANDOM  = "RANDOM";
static const std::string                ONE     = "ONE";
template<class type_msg, class peer_type>
class Network{
protected:

    std::vector<Peer<type_msg>*>        _peers;
    int                                 _avgDelay;
    int                                 _maxDelay;
    int                                 _minDelay;
    std::string                         _distribution;

    std::ostream                         *_log;

    std::string                         createId            ();
    bool                                idTaken             (std::string);
    std::string                         getUniqueId         ();
    void                                addEdges            (Peer<type_msg>*);
    int                                 getDelay            ();
	peer_type*							getPeerById			(std::string);

public:
    Network                                                 ();
    Network                                                 (const Network<type_msg,peer_type>&);
    ~Network                                                ();

    // setters
    void                                initNetwork         (int); // initialize network with peers
    void                                setMaxDelay         (int d)                                         {_maxDelay = d;};
    void                                setAvgDelay         (int d)                                         {_avgDelay = d;};
    void                                setMinDelay         (int d)                                         {_minDelay = d;};
    void                                setToRandom         ()                                              {_distribution = RANDOM;};
    void                                setToPoisson        ()                                              {_distribution = POISSON;};
    void                                setToOne            ()                                              {_distribution = ONE;};
    void                                setLog              (std::ostream&);

    // getters
    int                                 size                ()const                                         {return (int)_peers.size();};
    int                                 maxDelay            ()const                                         {return _maxDelay;};
    int                                 avgDelay            ()const                                         {return _avgDelay;};
    int                                 minDelay            ()const                                         {return _minDelay;};
    std::string                         distribution        ()const                                         {return _distribution;};


    //mutators
    void                                receive             ();
    void                                preformComputation  ();
    void                                transmit            ();
    void                                makeRequest         (int i)                                         {_peers[i]->makeRequest();};
	void                                shuffleByzantines   (int);

	// logging and debugging
    std::ostream&                       printTo             (std::ostream&)const;
    void                                log                 ()const                                         {printTo(*_log);};

    // operators
    Network&                            operator=           (const Network&);
    peer_type*                          operator[]          (int);
    const peer_type*                    operator[]          (int)const;
    friend std::ostream&                operator<<          (std::ostream &out, const Network &system)      {return system.printTo(out);};
	void 								makeRequest			(Peer<type_msg> * peer)				 { peer->makeRequest(); }
	void 								buildInitialDAG		();
	std::vector<peer_type *>			setPeersForConsensusDAG(Peer<type_msg> *,int);
	int									pickSecurityLevel	(int);

};

template<class type_msg, class peer_type>
Network<type_msg,peer_type>::Network(){
    _peers = std::vector<Peer<type_msg>*>();
    _avgDelay = 1;
    _maxDelay = 1;
    _minDelay = 1;
    _distribution = RANDOM;
    _log = &std::cout;
}

template<class type_msg, class peer_type>
Network<type_msg,peer_type>::Network(const Network<type_msg,peer_type> &rhs){
    if(this == &rhs){
        return;
    }

    for(int i = 0; i < _peers.size(); i++){
        delete _peers[i];
    }

    _peers = std::vector<Peer<type_msg>*>();
    for(int i = 0; i < rhs._peers.size(); i++){
        _peers.push_back(new peer_type(*dynamic_cast<peer_type*>(rhs._peers[i])));
    }
    _avgDelay = rhs._avgDelay;
    _maxDelay = rhs._maxDelay;
    _minDelay = rhs._minDelay;
    _distribution = rhs._distribution;
    _log = rhs._log;
}

template<class type_msg, class peer_type>
Network<type_msg,peer_type>::~Network(){
    for(int i = 0; i < _peers.size(); i++){
        delete _peers[i];
    }
}

template<class type_msg, class peer_type>
void Network<type_msg,peer_type>::setLog(std::ostream &out){
    _log = &out;
    for(int i = 0; i < _peers.size(); i++){
        peer_type *p = dynamic_cast<peer_type*>(_peers[i]);
        p->setLogFile(out);
    }
}

template<class type_msg, class peer_type>
std::string Network<type_msg,peer_type>::createId(){
    char firstPos = '*';
    char secondPos = '*';
    char thirdPos = '*';
    char fourthPos = '*';
    char fifthPos = '*';

    std::uniform_int_distribution<int> uniformDist(0,25);
    // add 'A' to shift char into rnage of upper case letters
    firstPos = uniformDist(RANDOM_GENERATOR) + 'A';
    secondPos = uniformDist(RANDOM_GENERATOR) + 'A';
    thirdPos = uniformDist(RANDOM_GENERATOR) + 'A';
    fourthPos = uniformDist(RANDOM_GENERATOR) + 'A';
    fifthPos = uniformDist(RANDOM_GENERATOR) + 'A';

    std::string id = "";
    id = id + firstPos + secondPos + thirdPos + fourthPos + fifthPos;
    return id;
}

template<class type_msg, class peer_type>
bool Network<type_msg,peer_type>::idTaken(std::string id){
    for(int i = 0; i < _peers.size(); i++){
        if(_peers[i]->id() == id){
            return true;
        }
    }
    return false;
}

template<class type_msg, class peer_type>
std::string Network<type_msg,peer_type>::getUniqueId(){
    std::string id = createId();

    while(idTaken(id)){
        id = createId();
    }
    
    return id;
}

template<class type_msg, class peer_type>
void Network<type_msg,peer_type>::addEdges(Peer<type_msg> *peer){
    for(int i = 0; i < _peers.size(); i++){
        if(_peers[i]->id() != peer->id()){
            if(!_peers[i]->isNeighbor(peer->id())){
                int delay = getDelay();
                // guard agenst 0 and negative numbers
                while(delay < 1 || delay > _maxDelay || delay < _minDelay){
                    delay = getDelay();
                }
                peer->addNeighbor(*_peers[i], delay);
                _peers[i]->addNeighbor(*peer,delay);
            }
        }
    }
}

template<class type_msg, class peer_type>
int Network<type_msg,peer_type>::getDelay(){
    if(_distribution == RANDOM){
        std::uniform_int_distribution<int> randomDistribution(_minDelay,_maxDelay);
        return randomDistribution(RANDOM_GENERATOR);
    }
    if(_distribution == POISSON){
        std::poisson_distribution<int> poissonDistribution(_avgDelay);
        return poissonDistribution(RANDOM_GENERATOR);
    }
    if(_distribution == ONE){
        return 1;
    }
    return -1;
}

template<class type_msg, class peer_type>
void Network<type_msg,peer_type>::initNetwork(int numberOfPeers){
    for(int i = 0; i < numberOfPeers; i++){
        _peers.push_back(new peer_type(getUniqueId()));
    }
    for(int i = 0; i < _peers.size(); i++){
        addEdges(_peers[i]);
    }
}

template<class type_msg, class peer_type>
void Network<type_msg,peer_type>::receive(){
    for(int i = 0; i < _peers.size(); i++){
        _peers[i]->receive();
    }
}

template<class type_msg, class peer_type>
void Network<type_msg,peer_type>::preformComputation(){
    for(int i = 0; i < _peers.size(); i++){
        _peers[i]->preformComputation();
    }
}

template<class type_msg, class peer_type>
void Network<type_msg,peer_type>::transmit(){
    for(int i = 0; i < _peers.size(); i++){
        _peers[i]->transmit();
    }
}

template<class type_msg, class peer_type>
std::ostream& Network<type_msg,peer_type>::printTo(std::ostream &out)const{
    out<< "--- NETWROK SETUP ---"<< std::endl<< std::endl;
    out<< std::left;
    out<< '\t'<< std::setw(LOG_WIDTH)<< "Number of Peers"<< std::setw(LOG_WIDTH)<< "Distribution"<< std::setw(LOG_WIDTH)<< "Min Delay"<< std::setw(LOG_WIDTH)<< "Average Delay"<< std::setw(LOG_WIDTH)<< "Max Delay"<< std::endl;
    out<< '\t'<< std::setw(LOG_WIDTH)<< _peers.size()<< std::setw(LOG_WIDTH)<< _distribution<< std::setw(LOG_WIDTH)<< _minDelay<< std::setw(LOG_WIDTH)<< _avgDelay<< std::setw(LOG_WIDTH)<< _maxDelay<< std::endl;

    for(int i = 0; i < _peers.size(); i++){
        peer_type *p = dynamic_cast<peer_type*>(_peers[i]);
        p->printTo(out);
    }

    return out;
}

template<class type_msg, class peer_type>
Network<type_msg, peer_type>& Network<type_msg,peer_type>::operator=(const Network<type_msg, peer_type> &rhs){
    if(this == &rhs){
        return *this;
    }

    for(int i = 0; i < _peers.size(); i++){
        delete _peers[i];
    }

    _peers = std::vector<Peer<type_msg>*>();
    for(int i = 0; i < rhs._peers.size(); i++){
        _peers.push_back(new peer_type(*dynamic_cast<peer_type*>(rhs._peers[i])));
    }

    _avgDelay = rhs._avgDelay;
    _maxDelay = rhs._maxDelay;
    _minDelay = rhs._minDelay;
    _distribution = rhs._distribution;

    return *this;
}

template<class type_msg, class peer_type>
peer_type* Network<type_msg,peer_type>::operator[](int i){
    return dynamic_cast<peer_type*>(_peers[i]);
}

template<class type_msg, class peer_type>
const peer_type* Network<type_msg,peer_type>::operator[](int i)const{
    return dynamic_cast<peer_type*>(_peers[i]);
}

template<class type_msg, class peer_type>
peer_type* Network<type_msg,peer_type>::getPeerById(std::string id){
	for(int i = 0; i<_peers.size(); i++){
		if(_peers[i]->id() ==  id)
			return dynamic_cast<peer_type*>(_peers[i]);
	}
	return nullptr;
}

std::string sha256(const std::string& str);

template<class type_msg, class peer_type>
void Network<type_msg,peer_type>::buildInitialDAG() {
	std::cerr << "Building initial dag" << std::endl;
	std::shared_ptr<DAG> preBuiltDAG  = std::make_shared<DAG>(true);
//	each peer will create a block initially
	std::vector<int> blockPeers;

	for(int i = 0; i<_peers.size(); i++){
		blockPeers.push_back(i);
	}
	//blockchain index starts from 1;
	int blockCount = 1;
	std::string prevHash = "genesisHash";
	while (blockCount <= _peers.size()) {
		std::set<std::string> publishers;
		int randPeer = rand()%blockPeers.size();
		int index = blockPeers[randPeer];
		blockPeers.erase(blockPeers.begin() + randPeer);

		std::string peerId = _peers[index]->id();
//		std::string blockHash = sha256(prevHash+"_"+peerId);
		std::string blockHash = std::to_string(blockCount);
		publishers.insert(peerId);

		DAGBlock newBlock = preBuiltDAG->createBlock(blockCount, {prevHash}, blockHash, publishers, "Data" ,false);
		//the initial dag does not contain byzantine blocks
		preBuiltDAG->addBlockToDAG(newBlock, {prevHash}, false);
		prevHash = blockHash;
		blockCount++;
	}

	std::cerr<<"Prebuilt dag size is "<<preBuiltDAG->getSize()<<std::endl;
//	preBuiltDAG->printGraph();
	preBuiltDAG->setTips();

	for(int i = 0; i<_peers.size();i++){
		dynamic_cast<peer_type *> (_peers[i])->setDAG(*preBuiltDAG);
	}
}

template<class type_msg, class peer_type>
std::vector<peer_type *> Network<type_msg,peer_type>::setPeersForConsensusDAG(Peer<type_msg> * peer, int securityLevel) {
	std::cerr<<"SECURTIY LEVEL CHOSEN = "<<securityLevel<<std::endl;
	int numOfPeers = (securityLevel);
	std::vector<peer_type*> peersForConsensus;
	if(numOfPeers == size()){
//		check if a peers is busy or not
		for(int i = 0 ; i< size(); i++){
			if(_peers[i]->isBusy()){
				return {};
			}
			peer_type* iPeer = getPeerById(_peers[i]->id());

			peersForConsensus.push_back(iPeer);
		}
	}else{
		std::vector<std::string> sortedDAG = (dynamic_cast<peer_type *> (peer))->getDAG().topologicalSort();
		/*std::cerr<<"THE SORTED DAG IS ";
		for(const auto & i : sortedDAG){
			std::cerr<<i<<" ";
		}*/

		//are all peers busy?
		bool allBusy = true;
		for(int i =0; i< size();i++){
			if(_peers[i]->isBusy()==false){
				allBusy = false;
				break;
			}
		}

		if(allBusy){
			std::cerr<<"Could not find a starting point"<<std::endl;
			return {};
		}
		//	find first size()/2 unique peers
		int uniquePeerCount = 0;
		int uniquePeersIndex = 0 ;
		std::vector<string> uniquePeers;
		do{
			std::string peerId = sortedDAG[uniquePeersIndex];
			if (std::find(uniquePeers.begin(), uniquePeers.end(), peerId) != uniquePeers.end())
			{
				uniquePeersIndex++;
				continue;
			}
			uniquePeers.push_back(peerId);
			uniquePeersIndex++;
			uniquePeerCount++;
		}while (uniquePeerCount < size()/2);

		uniquePeersIndex--;
		std::cerr<<"THE INDEX WHERE "<<size()/2<<" UNIQUE PEERS WERE FOUND IS "<<uniquePeersIndex<<std::endl;
		//	resize the vector upto uniquePeerBracketIndex index
		/*std::cerr<<"BEFORE SORTED DAG, SIZE"<<sortedDAG.size()<<std::endl;
		for(const auto & i : sortedDAG){
			std::cerr<<i<<" ";
		}*/

		sortedDAG.resize(uniquePeersIndex+1);
		std::cerr<<"AFTER SORTED DAG, SIZE"<<sortedDAG.size()<<std::endl;
		/*for(const auto & i : sortedDAG){
			std::cerr<<i<<" ";
		}*/

		std::vector<string> chosen;

		while (peersForConsensus.size() < numOfPeers) {
			if(chosen.size() + sortedDAG.size() < numOfPeers){
				std::cerr<<"Not enough peers to form a committee"<<std::endl;
				return {};
			}
			int randIndex = rand() % sortedDAG.size();
			std::string peerId = sortedDAG[randIndex];
			//random value
			if(std::find(chosen.begin(), chosen.end(), peerId) !=  chosen.end()) {
				std::cerr<<"The jumped on peer is already chosen."<<std::endl;
				sortedDAG.erase(sortedDAG.begin()+randIndex);
				continue;
			} else {
				peer_type* iPeer = getPeerById(peerId);
				if(!iPeer->isBusy()){
					std::cerr<<iPeer->id()<<" IS NOT BUSY"<<std::endl;
					peersForConsensus.push_back(iPeer);
					chosen.push_back(peerId);
					sortedDAG.erase(sortedDAG.begin()+randIndex);

				}else{
//					no busy, if already selected remove and try another
					std::cerr<<"The jumped on peer "<<iPeer->id()<<" is already taken for another committee "<<std::endl;
					sortedDAG.erase(sortedDAG.begin()+randIndex);
					continue;
				}
			}
		}
	}
	for(int i = 0 ; i< peersForConsensus.size(); i++){
		std::map<std::string, Peer<type_msg>* > neighbours;			//previous group is dissolved when new group is selected
		for(int j = 0; j< peersForConsensus.size(); j++){
			if(i != j)
				neighbours[peersForConsensus[j]->id()]=peersForConsensus[j];
		}
		dynamic_cast<peer_type *> (peersForConsensus[i])->setCommitteeNeighbours(neighbours);
	}

	//set peers to busy
	for(auto &peerBusy:peersForConsensus){
		peerBusy->setBusy(true);
	}

	std::cerr<<"FINALISED SECURITY LEVEL CHOSEN = "<<securityLevel<<std::endl;
	return peersForConsensus;
}

template<class type_msg, class peer_type>
void Network<type_msg,peer_type>::shuffleByzantines(int shuffleCount){
	int shuffled = 0;
	//find list of byzantineFlag peers
	vector<int> byzantineIndex;
	vector<int> nonByzantineIndex;

	for(int i = 0; i<_peers.size();i++){
		if(_peers[i]->isByzantine()){
			byzantineIndex.push_back (i);
		}else if(!_peers[i]->isByzantine()){
			nonByzantineIndex.push_back (i);
		}
	}

	while (shuffled<shuffleCount){
		//find list of byzantineFlag peers
		int byzantineShuffleIndex = static_cast<int>(rand() % byzantineIndex.size());
		int nonByzantineShuffleIndex = static_cast<int>(rand() % nonByzantineIndex.size());
		_peers[byzantineIndex[byzantineShuffleIndex]]->setByzantineFlag(false);
		_peers[nonByzantineIndex[nonByzantineShuffleIndex]]->setByzantineFlag(true);
		byzantineIndex.erase(byzantineIndex.begin ()+byzantineShuffleIndex);
		nonByzantineIndex.erase(nonByzantineIndex.begin ()+nonByzantineShuffleIndex);
		shuffled++;
	}
}

template<class type_msg, class peer_type>
int Network<type_msg, peer_type>::pickSecurityLevel(int numberOfPeers){
	std::uniform_int_distribution<int> coin(0,1);
	int trails = 0;
	int heads = coin(RANDOM_GENERATOR);
	while(!heads){
		trails++;
		heads = coin(RANDOM_GENERATOR);
	}
	switch (trails) {
		case 0: return numberOfPeers / 16;
		case 1: return numberOfPeers / 8;
		case 2: return numberOfPeers / 4;
		case 3: return numberOfPeers / 2;
		case 4: return numberOfPeers;
		default: return numberOfPeers / 16;
	}
}
#endif /* Network_hpp */
