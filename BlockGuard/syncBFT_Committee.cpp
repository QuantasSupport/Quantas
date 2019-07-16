//
//  syncBFT_Committee.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 7/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <stdio.h>
#include "syncBFT_Committee.hpp"

syncBFT_Committee::syncBFT_Committee(std::vector<syncBFT_Peer *> peers, syncBFT_Peer *sender, std::string transaction, int sLevel) : Committee<syncBFT_Peer>(peers, sender, transaction, sLevel){
    for(auto & committeePeer : committeePeers){
        committeePeer->setTerminated(false);
        committeePeer->setCommitteeSize(committeePeers.size());
        leaderIdCandidates.push_back(committeePeer->id());
    }
}

syncBFT_Committee::syncBFT_Committee(const syncBFT_Committee &rhs) : Committee<syncBFT_Peer>(rhs){
    status = rhs.status;
    leaderId = rhs.leaderId;
    syncBFTsystemState = rhs.syncBFTsystemState;
    changeLeader = rhs.changeLeader;
    leaderIdCandidates = rhs.leaderIdCandidates;
    firstMinerIndex = rhs.firstMinerIndex;
    clock = rhs.clock;
}

syncBFT_Committee& syncBFT_Committee::operator=(const syncBFT_Committee &rhs) {
    if(this == &rhs)
        return *this;
    Committee<syncBFT_Peer>::operator=(rhs);
    status = rhs.status;
    leaderId = rhs.leaderId;
    syncBFTsystemState = rhs.syncBFTsystemState;
    changeLeader = rhs.changeLeader;
    leaderIdCandidates = rhs.leaderIdCandidates;
    firstMinerIndex = rhs.firstMinerIndex;
    clock = rhs.clock;
    return *this;
}

void syncBFT_Committee::receiveTx(){
    for(auto & committeePeer : committeePeers){
        committeePeer->receiveTx();
    }
}

int syncBFT_Committee::incrementSyncBFTsystemState() {
    if(syncBFTsystemState == 3) {
        changeLeader = true;
        syncBFTsystemState=0;
    }
    else
        syncBFTsystemState++;
    
    for(auto & committeePeer : committeePeers){
        committeePeer->syncBFTsystemState = syncBFTsystemState;
    }
    
    return syncBFTsystemState;
}

void syncBFT_Committee::nextState(int iter, int maxDelay){
    //reset sync status of the peers after a notify step is done.
    if(syncBFTsystemState==3){
        for(int i = 0; i<size();i++){
            committeePeers[i]->setSyncBFTState(0);
            committeePeers[i]->iter++;
        }
    }
    incrementSyncBFTsystemState();
}

void syncBFT_Committee::leaderChange() {
    if(leaderId.empty()){
        leaderId = leaderIdCandidates.front();
    }else
    {
        auto it = std::find(leaderIdCandidates.begin(),leaderIdCandidates.end(),leaderId);
        assert(it!=leaderIdCandidates.end());
        std::rotate(it, it+1, leaderIdCandidates.end());
        assert(leaderId!=leaderIdCandidates.front());
        leaderId = leaderIdCandidates.front();
    }
    changeLeader = false;
    
    for(auto & committeePeer : committeePeers){
        committeePeer->setLeaderId(leaderId) ;
    }
}

void syncBFT_Committee::refreshPeers() {
    //    refresh the peers
    for(auto & committeePeer : committeePeers){
        committeePeer->refreshSyncBFT();
    }
}

void syncBFT_Committee::preformComputation(){
    if(syncBFTsystemState == 0 || syncBFTsystemState == 4){
        if(changeLeader){
            leaderChange();
        }
    }
    
    for(auto & committeePeer : committeePeers){
        if(!committeePeer->isTerminated())
            committeePeer->preformComputation();
    }
    
    if(syncBFTsystemState == 3){
        //check if all peers terminated
        bool consensus = true;
        for(int peerId = 0; peerId<size(); peerId++ ) {
            if (!committeePeers[peerId]->getTerminationFlag()) {
                //the leader does not need to terminate
                if (committeePeers[peerId]->id() == getLeaderId()) {
                    continue;
                }
                consensus = false;
                break;
            }
        }
        if (consensus){
            //reset system-wide sync state
            syncBFTsystemState = 0;
        }
        consensusFlag = consensus;
    }
}

void syncBFT_Committee::initiate(){
    dynamic_cast<syncBFT_Peer *>(senderPeer)->makeRequest(committeePeers, tx);
	int byzantineCount = 0;
	for(int i = 0 ; i< committeePeers.size(); i++){
		if(committeePeers[i]->isByzantine()){
			byzantineCount++;
		}
        std::map<std::string, Peer<syncBFTmessage>* > neighbours;    //previous group is dissolved when new group is selected
        for(int j = 0; j< committeePeers.size(); j++){
            if(i != j){
                neighbours[committeePeers[j]->id()] = committeePeers[j];
            }
        }
        dynamic_cast<syncBFT_Peer *> (committeePeers[i])->setCommitteeNeighbours(neighbours);
    }
	if(byzantineCount>= (size() - 1)/2 +1){
		defeated = true;
	}
}
