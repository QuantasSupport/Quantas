//
// Created by srai on 6/2/19.
//

#ifndef Committee_hpp
#define Committee_hpp


#include <cstdio>
#include <iostream>
#include <vector>
#include "./../Common/Peer.hpp"

template<class peer_type>
class Committee {
protected:
	std::vector<peer_type *>				committeePeers;
	peer_type *								senderPeer;
	int 									securityLevel = 0;
	std::string								tx;
	std::string								cId;
	bool 									consensusFlag = false;
	double 									byzantineRatio = 0;

public:
	Committee								(std::vector<peer_type *> , peer_type *, const std::string& , int);
	Committee								(const Committee&);
	~Committee								() = default;
	Committee&								operator=						(const Committee&);

    // getters
	int										size							()const	                            {return committeePeers.size();};
	std::vector<peer_type *>				getCommitteePeers				()                                  {return committeePeers;};
	std::vector<std::string>				getCommitteePeerIds				();
	bool 									getConsensusFlag				()                                  {return consensusFlag;};
	double									getByzantineRatio				()                                  {return byzantineRatio;};
	std::string								getCommitteeId					()                                  {return cId;}
	int										getSecurityLevel				()									{return securityLevel;}
    peer_type*                              get                             (int i)                             {return operator[](i);};
    peer_type*                              get                             (int i)const                        {return operator[](i);};
    
    // mutators
	virtual void							preformComputation				() = 0;
	void									receive							();
	void									transmit						();
    
    // operators
    peer_type*                              operator[]                      (int);
    const peer_type*                        operator[]                      (int)const;
};

template <class peer_type>
Committee<peer_type>::Committee(std::vector<peer_type*> peers, peer_type *sender, const std::string& transaction, int sLevel){
	committeePeers = peers;
	tx = transaction;
	senderPeer = sender;
	securityLevel = sLevel;
	consensusFlag = false;
	cId= "C"+sender->id();

	int byzantineCount = 0;
	for(int i =0; i<committeePeers.size(); i++){
		if(committeePeers[i]->isByzantine()){
			byzantineCount++;
		}
	}

	byzantineRatio = double(byzantineCount)/committeePeers.size();
}

template <class peer_type>
Committee<peer_type>::Committee(const Committee &rhs){
	committeePeers = rhs.committeePeers;
	senderPeer = rhs.senderPeer;
	securityLevel = rhs.securityLevel;
	tx = rhs.tx;
	cId = rhs.cId;
	consensusFlag = rhs.consensusFlag;
	byzantineRatio = rhs.byzantineRatio;
}

template<class peer_type>
Committee<peer_type> &Committee<peer_type>::operator=(const Committee &rhs) {
	if(this == &rhs)
		return *this;
	committeePeers = rhs.committeePeers;
	senderPeer = rhs.senderPeer;
	securityLevel = rhs.securityLevel;
	tx = rhs.tx;
	cId = rhs.cId;
	consensusFlag = rhs.consensusFlag;
	byzantineRatio = rhs.byzantineRatio;
	return *this;
}

template <class peer_type>
void Committee<peer_type>::receive(){
	for(int i = 0 ; i< committeePeers.size(); i++){
		committeePeers[i]->receive();
	}
}

template <class peer_type>
void Committee<peer_type>::transmit(){
	for(int i = 0 ; i< committeePeers.size(); i++){
		committeePeers[i]->transmit();
	}
}

template <class peer_type>
std::vector<std::string> Committee<peer_type>::getCommitteePeerIds(){
	std::vector<std::string> ids;
	for(int i =0; i<size();i++){
		ids.push_back(committeePeers[i]->id());
	}
	return ids;
}

template<class peer_type>
peer_type* Committee<peer_type>::operator[](int i){
    return dynamic_cast<peer_type*>(committeePeers[i]);
}

template<class peer_type>
const peer_type* Committee<peer_type>::operator[](int i)const{
    return dynamic_cast<peer_type*>(committeePeers[i]);
}

#endif //Committee_hpp

