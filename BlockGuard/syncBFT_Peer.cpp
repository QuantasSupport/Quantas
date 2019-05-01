#include <utility>

//
// Created by srai on 3/15/19.
//

#include "syncBFT_Peer.hpp"


bool syncBFT_Peer::changeLeader = true;
vector<string> syncBFT_Peer::leaderIdCandidates = {};
string syncBFT_Peer::leaderId;
int syncBFT_Peer::syncBFTsystemState = 0;


syncBFT_Peer::syncBFT_Peer(std::string id) : Peer<syncBFTmessage>(id){
    counter = 0;
    blockchain = new Blockchain(true);

    acceptedState as1("_","0",{});
    acceptedStates.emplace_back (as1);

    terminated = false;
    valueFromLeader ="_";
    syncBFTstate = 0;
    statusMessageToSelf = nullptr;
    byzantineFlag = false;
    notifyMessagesPacket = {};
    iter = 0;
    leaderIdCandidates.push_back(id);

}

void syncBFT_Peer::populateOutStream(syncBFTmessage msg){
    //for(auto & _neighbor : _neighbors) {
    for(auto it = _neighbors.begin(); it != _neighbors.end(); it++){
        Peer<syncBFTmessage> *peer = it->second;
        Packet<syncBFTmessage> newMessage(std::to_string(iter), peer->id(), _id);
        newMessage.setBody(msg);
        _outStream.push_back(newMessage);
    }
}
void syncBFT_Peer::createBlock(std::set<std::string> publishers){
    blockchain->createBlock(blockchain->getChainSize(), blockchain->getLatestBlockHash(), std::to_string(blockchain->getChainSize())+"_"+id(), std::move(publishers));
}
void syncBFT_Peer::currentStatusSend(){

    //std::cerr<<"INSIDE CURRENT STATUS SEND"<<std::endl;
    //std::cerr <<"WORKING REPLICAS:: Sending status to the leader" << std::endl;
    //std::cerr<<"WORKER :: Peer Id is "<<id()<<std::endl;

    //handle notfiy messages if any, save it.
    for(auto & i : _inStream){
        assert(i.getMessage().type == "NOTIFY");
        if(isValidNotify(i.getMessage())){
            notifyMessagesPacket.push_back(i);

            acceptedState as1(valueFromLeader, std::to_string (iter),i.getMessage().cc);
            acceptedStates.push_back (as1);
        }
        //just accept the first notify message
        break;
    }

    _inStream.clear();


    //all peers
    syncBFTmessage sMessage;
//    sMessage.info = "Status Message: " + std::to_string(iter) + " From " + _id;
    sMessage.peerId = _id;
    sMessage.type = "STATUS";
    sMessage.iter = std::to_string(iter);
    assert(!acceptedStates.empty());
    sMessage.message = {std::to_string(iter), "STATUS", acceptedStates.back().value,acceptedStates.back().valueAcceptedAt};
    sMessage.cc = acceptedStates.back().cc;
    sMessage.statusCert = "Cert_"+_id;
    Packet<syncBFTmessage> newMessage(std::to_string(iter), leaderId, _id);
    newMessage.setBody(sMessage);

    if(isLeader()){
        statusMessageToSelf = std::make_unique<syncBFTmessage> (sMessage);
        setSyncBFTState(1);
    }else{

        _outStream.push_back(newMessage);
        setSyncBFTState(2);
    }

}

void syncBFT_Peer::propose(){
    assert(isLeader());
    //std::cerr<<"WORKING  LEADER: Collecting status and proposing"<<std::endl;

    //form a safe value proof P

    //get message to self and add it to _instream in a Packet container
    Packet<syncBFTmessage> newMessage(std::to_string(iter),_id,_id);
    newMessage.setBody(*statusMessageToSelf);
    _inStream.push_back(newMessage);

    //concatenate notify messages
    _inStream.insert(_inStream.end(), notifyMessagesPacket.begin(), notifyMessagesPacket.end());
    //std::cerr<<"Checking for status and notify messages in a total of "<<_inStream.size()<<" messages"<<std::endl;
    for(auto & i : _inStream){
        assert(i.getMessage().type == "STATUS"||i.getMessage().type == "NOTIFY");
        if(P.size()>= (peerCount - 1)/2 +1){
            break;
        }
        if(isValidStatus(i.getMessage())){
            if(i.getMessage().type == "STATUS"){
                P.status.push_back(i.getMessage().message);
            }else if(i.getMessage().type == "NOTIFY"){
                P.notify.push_back(i.getMessage().message);
            }
        }
    }

    _inStream.clear();

    bool prevAcceptedValue = false;
    syncBFTmessage proposalMessage;

    for(auto &n: P.notify){
        assert(n.size() == 3);
        //The accepted state has a value accepted at iter > 0
        if(n[0]!="0"){
                prevAcceptedValue = true;
        }
    }
    if(!prevAcceptedValue){
        for(auto &s: P.status){
            assert(s.size() == 4);
            if(s[3]!="0"){
                prevAcceptedValue = true;
            }
        }
    }

//    proposalMessage.info = "Propose Message: " + std::to_string(iter)  + " From "+_id;
    proposalMessage.peerId = _id;
    proposalMessage.type="PROPOSE";
    proposalMessage.iter = std::to_string(iter);
    proposalMessage.P.clear ();

    if(!prevAcceptedValue){
        //No value accepted in P
        //Leader choosing a value to propose
        //choose any value to propose if no value has been accepted in P

        //value proposed from here.
        proposalMessage.message = {std::to_string(iter),"PROPOSE","VALUE"};
        proposalMessage.P = P;
        proposalMessage.value = "VALUE";

    }else{
        //Some values were Previously accepted
        //choose a highest non-zero iteration number value to propose under P
        int highIter= 0;
        int index = 0;

        string highIterVal;
        for(auto &n: P.notify){
            assert(n.size() == 3);
            if(n[0]!="0") {
                if (std::stoi(n[0]) >= highIter){
                    highIterVal = n[2];
                    highIter = index;
                }

            }
        }
        for(auto &s: P.status){
            assert(s.size() == 4);
            if(s[3]!="0"){
                if(std::stoi(s[3])>=highIter){
                    highIterVal = s[2];
                    highIter = index;
                }
            }
        }

        proposalMessage.message = {std::to_string(iter),"PROPOSE", highIterVal};
        proposalMessage.P = P;
        proposalMessage.value = "VALUE";

    }

    //std::cerr<<"Forwarding the proposal from leader"<<std::endl;

    populateOutStream(proposalMessage);
    setSyncBFTState(2);


}

void syncBFT_Peer::commitFromLeader(){
    //std::cerr<<"WORKING: LEADER COMMITTING"<<std::endl;
    syncBFTmessage commitMessage;
    //searching for notify messages

    _inStream.insert(_inStream.end(), notifyMessagesPacket.begin(), notifyMessagesPacket.end());
    for(auto & i : _inStream){
        //std::cerr<<"Checking for notify messages"<<std::endl;
        if(i.getMessage().type == "NOTIFY"){
            //std::cerr<<"notify message found"<<std::endl;
            commitMessage = i.getMessage();
            if(i.getMessage().value == acceptedStates.back().value){
                populateOutStream(commitMessage);
                //a notify message is a valid commit message
                return;
            }
        }
    }
    //std::cerr<<"notify message not found"<<std::endl;
    commitMessage.P.clear();
    if(isByzantine ()){
        valueFromLeader = "CONFLICTING_VALUE";
    }
    else
        valueFromLeader = "VALUE";
//    commitMessage.info = "Forward Propose Message/ Commit message: " + std::to_string(iter)  + " From "+_id;
    commitMessage.type = "FORWARD_PROPOSAL_AND_COMMIT";
    commitMessage.value = valueFromLeader;
    commitMessage.peerId = _id;
    commitMessage.message = {std::to_string(iter),"COMMIT",valueFromLeader};


    populateOutStream(commitMessage);

    setSyncBFTState(3);
}

void syncBFT_Peer::commit(){
    syncBFTmessage messageToForward;
    syncBFTmessage virtualProposal;

    //std::cerr<<"WORKING ALL: Forwarding if valid and committing"<<std::endl;
    bool virtualProposalFound = false;
    //check for notify message from the leader
    _inStream.insert(_inStream.end(), notifyMessagesPacket.begin(), notifyMessagesPacket.end());
    for(int i = 0; i< _inStream.size();i++){
        //std::cerr<<"Checking for notify message"<<std::endl;
        if(_inStream[i].getMessage().type == "NOTIFY"){
            virtualProposal = _inStream[i].getMessage();
            if(_inStream[i].getMessage().value == acceptedStates.back().value){
                valueFromLeader = _inStream[i].getMessage().value;
                //std::cerr<<"VIRTUAL PROPOSAL FOUND"<<std::endl;
                syncBFTmessage virtualProposalMessage = virtualProposal;
                virtualProposalMessage.P.clear ();
//                virtualProposalMessage.info = "Forward Propose Message/ Commit message: " + std::to_string(iter)  + " From "+_id;
                virtualProposalMessage.type = "FORWARD_PROPOSAL_AND_COMMIT";
                virtualProposalMessage.value = valueFromLeader;
                virtualProposalMessage.message = {std::to_string(iter),"COMMIT",valueFromLeader};

                populateOutStream(virtualProposalMessage);
                return;
            }
        }
    }
    //viewing content of _inStream

    //std::cerr<<"Notify message not found, no virtual proposal"<<std::endl;
    assert(_inStream.size()==1);
    assert(_inStream[0].getMessage().type=="PROPOSE");
    if(isValidProposal(_inStream[0].getMessage())){
        //std::cerr<<"The proposal from the leader is valid"<<std::endl;
        valueFromLeader = (_inStream[0].getMessage().value);
        messageToForward = (_inStream[0].getMessage());
    } else{
        //std::cerr<<"The proposal from the leader is not valid: NOTHING TO DO"<<std::endl;
        valueFromLeader = ("_");
    }

    _inStream.clear();

    if(valueFromLeader !="_"){

        //Forward the proposal as well as commit message
        messageToForward.P.clear ();
//        messageToForward.info = "Forward Propose Message/ Commit message: " + std::to_string(iter)  + " From "+_id;
        messageToForward.type = "FORWARD_PROPOSAL_AND_COMMIT";
        messageToForward.value = valueFromLeader;
        messageToForward.message = {std::to_string(iter),"COMMIT",valueFromLeader};
        messageToForward.peerId = _id;

        populateOutStream(messageToForward);

    }

    setSyncBFTState(3);


}

void syncBFT_Peer::notify(){
    bool committed = false;
    //std::cerr<<"WORKING ALL: NOTIFYING EVERYONE IF COMMITTING"<<std::endl;

    _inStream.insert(_inStream.end(), notifyMessagesPacket.begin(), notifyMessagesPacket.end());

    for(auto & i : _inStream){
        assert(i.getMessage().type == "FORWARD_PROPOSAL_AND_COMMIT"||i.getMessage().type == "NOTIFY");
    }

    //traverse through "FORWARD_PROPOSAL" messages and searching for leader equivocation
    //std::cerr<<"Traversing to search for equivocation"<<std::endl;
    for(int i = 0; i<_inStream.size();i++){
        if(isValidProposal(_inStream[i].getMessage())&&_inStream[i].getMessage().value != valueFromLeader){
            //std::cerr<<"EQUIVOCATION FOUND"<<" IN PEER "<<id()<<"!"<<std::endl;
            setSyncBFTState(4);
            _inStream.clear();
            return;
        }
    }

    //std::cerr<<"NO EQUIVOCATION!"<<std::endl;
    std::map<string, int> valueToCommitCount;
    for(auto & i : _inStream){
        if(i.getMessage().type=="FORWARD_PROPOSAL_AND_COMMIT"){
            if(i.getMessage().value==valueFromLeader){
                valueToCommitCount[i.getMessage().value]++;
            }
        }
    }

    //if replica receives f+1 valid commit requests
    for (auto const& pair : valueToCommitCount)
    {
        if (pair.second >= (peerCount - 1)/2 +1){
            committed = true;
        }
    }
    if(committed){
        //create commit certificate, concatenation of valid and value matching commits
        for(auto & i : _inStream){
            if(i.getMessage().type=="FORWARD_PROPOSAL_AND_COMMIT"){
                if(i.getMessage().value==valueFromLeader){
                    cc.commit.push_back((i.getMessage().message));
                }
            }
        }

        acceptedState as1(valueFromLeader, std::to_string (iter),cc);
        acceptedStates.push_back(as1);
    }
    //std::cerr<<"The commit certificate size is "<<std::endl;
    //std::cerr<<commitCertificate.size()<<std::endl;
    _inStream.clear();
    setSyncBFTState(4);

    if(committed){
        syncBFTmessage bcMessage;
//        bcMessage.info = "Notify Message: " + std::to_string(iter)  + " From "+_id;
        bcMessage.peerId = _id;
        bcMessage.type="NOTIFY";
        bcMessage.message = {std::to_string(iter), "NOTIFY",valueFromLeader};
        bcMessage.cc.clear ();
        bcMessage.cc = cc;
        bcMessage.iter = std::to_string(iter);
        bcMessage.value = valueFromLeader;


        populateOutStream(bcMessage);

        if(!isLeader())
            terminated = true;

        //write to blockchain
        createBlock({"All"});

//		std::cerr<<"BLOCKCHAIN SIZE IS "<<blockchain->getChainSize()<<std::endl;
    }else{
//	    std::cerr<<"COMMITTED IS FALSE"<<std::endl;
    }
    setSyncBFTState(4);
}


/*
 * 0,1,2,3 represent STATUS_SEND, STATUS_COLLECT_AND_PROPOSE, COMMIT, NOTIFY respectively,
 */
void syncBFT_Peer::run(){


    if(terminated){
        //std::cerr<<"TERMINATED"<<std::endl;
    }else{
//		std::cerr<<"SyncBFTState is " <<syncBFTstate<<std::endl;
//		std::cerr<<"SyncBFTState System is " <<syncBFTsystemState<<std::endl;
        if(!terminated) {

            if (syncBFTsystemState >= syncBFTstate) {
                switch (syncBFTsystemState) {
                    case 0:
                    case 4:
                        if(changeLeader){
                            std::cerr<<"Old leader was :"<<leaderId<<std::endl;
                            leaderChange();
                            std::cerr<<"New leader is  :"<<leaderId<<std::endl;
                        }
//						std::cerr<<"----------------------------------------------------------------------status send"<<std::endl;
                        currentStatusSend();
                        break;
                    case 1:
                        //std::cerr<<"isLeader"<<std::endl;
                        //std::cerr<<isLeader()<<std::endl;
                        if (isLeader()) {
                            propose();
                        }
//						std::cerr<<"----------------------------------------------------------------------PROPOSE"<<std::endl;
                        break;
                    case 2:
                        if (isLeader())
                            commitFromLeader();
                        else
                            commit();
//						std::cerr<<"----------------------------------------------------------------------COMMIT"<<std::endl;
                        break;
                    case 3:
                        notify();
//						std::cerr<<"----------------------------------------------------------------------NOTIFY"<<std::endl;
                        break;

                }
            } else {
//				std::cerr << " DOING NOTHING, JUST WAITING FOR MESSAGES UNTIL MAX DELAY" << std::endl;
            }
        }

    }

}

void syncBFT_Peer::preformComputation(){

    run();
    counter++;

}

bool syncBFT_Peer::leaderChange(){
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
    return changeLeader;
}

void syncBFT_Peer::refreshSyncBFT(){
    iter = 0;
    P.clear();

    acceptedStates.clear();
    acceptedState as1("_","0", {});
    acceptedStates.push_back (as1);

    terminated = false;
    valueFromLeader.clear();
    syncBFTstate = 0;
    _inStream.clear();
    _outStream.clear();
    notifyMessagesPacket.clear();
    cc.clear();
    P.clear();
    statusMessageToSelf = nullptr;
    for(auto & _channel : _channels){
        _channel.second.clear();

    }
}

int syncBFT_Peer::incrementSyncBFTsystemState(){
    if(syncBFTsystemState == 3) {
        changeLeader = true;
        syncBFTsystemState=0;
    }
    else
        syncBFTsystemState++;
    return syncBFTsystemState;
}

