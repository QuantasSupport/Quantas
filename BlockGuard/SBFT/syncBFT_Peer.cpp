//
// Created by srai on 3/15/19.
//

#include "syncBFT_Peer.hpp"
#include <cassert>
#include <utility>
#include "./../Common/Logger.hpp"
//#include "hash.hpp"


syncBFT_Peer::syncBFT_Peer(std::string id) : Peer<syncBFTmessage>(id){
	P.clear();

	acceptedState as1("_","0",{});
	acceptedStates.emplace_back (as1);

	valueFromLeader ="_";
	cc.clear();
	syncBFTstate = 0;
	statusMessageToSelf = nullptr;
	notifyMessagesPacket = {};
	committeeNeighbours = _neighbors;
	messageToSend = {};
	minedBlock = nullptr;
	dag = {};
	leaderId = "";
	terminated = true;
	committeeSize = 0;
	consensusTx = "";
    submissionRound = -1;

	iter = 0;
	syncBFTsystemState = 0;
	dagBlocks = {};

}

syncBFT_Peer::syncBFT_Peer(const syncBFT_Peer &rhs) : Peer(rhs),statusMessageToSelf(new syncBFTmessage(*rhs.statusMessageToSelf)){
	P = rhs.P;
	acceptedStates = rhs.acceptedStates;
	valueFromLeader = rhs.valueFromLeader;
	cc = rhs.cc;
	syncBFTstate = rhs.syncBFTstate;

	notifyMessagesPacket = rhs.notifyMessagesPacket;
	committeeNeighbours = rhs.committeeNeighbours;
	messageToSend = rhs.messageToSend;
	minedBlock = new DAGBlock(*rhs.minedBlock);
	dag = rhs.dag;
	leaderId = rhs.leaderId;
	terminated = rhs.terminated;
	committeeSize = rhs.committeeSize;
	consensusTx = rhs.consensusTx;
    submissionRound = rhs.submissionRound;
    
	iter = rhs.iter;
	syncBFTsystemState = rhs.syncBFTsystemState;
	dagBlocks = rhs.dagBlocks;

}

syncBFT_Peer& syncBFT_Peer::operator=(const syncBFT_Peer &rhs){
	if(this == &rhs)
		return *this;
	P = rhs.P;
	acceptedStates = rhs.acceptedStates;
	valueFromLeader = rhs.valueFromLeader;
	cc = rhs.cc;
	syncBFTstate = rhs.syncBFTstate;

	notifyMessagesPacket = rhs.notifyMessagesPacket;
	committeeNeighbours = rhs.committeeNeighbours;
	messageToSend = rhs.messageToSend;
	minedBlock = new DAGBlock(*rhs.minedBlock);
	dag = rhs.dag;
	leaderId = rhs.leaderId;
	terminated = rhs.terminated;
	committeeSize = rhs.committeeSize;
	consensusTx = rhs.consensusTx;
    submissionRound = rhs.submissionRound;
    
	iter = rhs.iter;
	syncBFTsystemState = rhs.syncBFTsystemState;
	dagBlocks = rhs.dagBlocks;

	return *this;
}

void syncBFT_Peer::populateOutStream(const syncBFTmessage& msg){
	for(auto & _neighbor : committeeNeighbours) {
		Peer<syncBFTmessage> *peer = _neighbor.second;
		Packet<syncBFTmessage> newMessage(std::to_string(iter), peer->id(), _id);
		newMessage.setBody(msg);
		_outStream.push_back(newMessage);
	}
}

void syncBFT_Peer::createBlock(const std::set<std::string>& publishers){
	std::vector<string> hashesToConnectTo = dag.getTips();
	std::string newBlockString;

	for (auto const& s : hashesToConnectTo) { newBlockString += "_" + s;}

	newBlockString += "_" + consensusTx;

//	use this for proper hashing
//	string newBlockHash = sha256(newBlockString);
	string newBlockHash = std::to_string(dag.getSize());

	minedBlock = new DAGBlock(dag.createBlock(dag.getSize(), hashesToConnectTo, newBlockHash, {id()}, consensusTx, faultyBlock));
	minedBlock->setSubmissionRound(_clock);
    minedBlock->setSecruityLevel(committeeNeighbours.size() + 1);
}

void syncBFT_Peer::currentStatusSend(){
	Logger::instance()->log("---STATUS---\n");

	//handle notfiy messages if any, save it.
    //just accept the first notify message
	if(!_inStream.empty()){
        auto i = _inStream.front();
		assert(i.getMessage().type == "NOTIFY");
		if(isValidNotify(i.getMessage())){
			notifyMessagesPacket.push_back(i);
			acceptedState as1(valueFromLeader, std::to_string(iter), i.getMessage().cc);
			acceptedStates.push_back (as1);
		}
        _inStream.clear();
	}

	syncBFTmessage sMessage;
	sMessage.peerId = _id;
	sMessage.type = "STATUS";
	sMessage.iter = std::to_string(iter);
	assert(!acceptedStates.empty());
	sMessage.message = {std::to_string(iter), "STATUS", acceptedStates.back().value, acceptedStates.back().valueAcceptedAt, std::to_string(isByzantine())};
	sMessage.cc = acceptedStates.back().cc;
	sMessage.statusCert = "Cert_"+_id;
	Packet<syncBFTmessage> newMessage(std::to_string(iter), leaderId, _id);
	newMessage.setBody(sMessage);

	Logger::instance()->log(id() + " BYZANTINTE "+ std::to_string(isByzantine()) + " IS LEADER " + std::to_string(isLeader()) + "\n");
	if(isLeader()){
		statusMessageToSelf = std::make_unique<syncBFTmessage> (sMessage);
		setSyncBFTState(1);
	}else{
		Logger::instance()->log(id() + " SENDING STATUS MESSAGE TO " + leaderId + "\n");
		_outStream.push_back(newMessage);
		setSyncBFTState(2);
	}
}

void syncBFT_Peer::propose(){
	Logger::instance()->log("---PROPOSE---\n");

	assert(isLeader());

	//	form a safe value proof P
	//	get message to self and add it to _instream in a Packet container
	Packet<syncBFTmessage> newMessage(std::to_string(iter),_id,_id);
	newMessage.setBody(*statusMessageToSelf);
	_inStream.push_back(newMessage);

	//	concatenate notify messages
	_inStream.insert(_inStream.end(), notifyMessagesPacket.begin(), notifyMessagesPacket.end());

	//	distinguish honest and faulty messages
	std::unordered_map<string, int> msgMap;
	for(auto & i: _inStream){
		msgMap[i.getMessage().message[4]]++;
	}
	std::string dominantMsg;
	//	find dominant message to propose
	for(const auto& a: msgMap){
		if(a.second>=(committeeSize - 1)/2 +1){
			dominantMsg = a.first;
			break;
		}
	}

	assert(!dominantMsg.empty());

	for(auto & i : _inStream){

		Logger::instance()->log(id() + " RECEIVED " + i.getMessage().type + " MESSAGE FROM " + i.getMessage().peerId + "\n");
		assert(i.getMessage().type == "STATUS"||i.getMessage().type == "NOTIFY");
		if(P.size()>= (committeeSize - 1)/2 +1){
			break;
		}
		if(isValidStatus(i.getMessage())){
			if(i.getMessage().message[4]==dominantMsg) {
				if (i.getMessage().type == "STATUS") {
					P.status.push_back(i.getMessage().message);
				} else if (i.getMessage().type == "NOTIFY") {
					P.notify.push_back(i.getMessage().message);
				}
			}
		}
	}

	assert((P.size()>= (committeeSize - 1)/2 +1));

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
			assert(s.size() == 5);
			if(s[3]!="0"){
				prevAcceptedValue = true;
			}
		}
	}

	proposalMessage.peerId = _id;
	proposalMessage.type = "PROPOSE";
	proposalMessage.iter = std::to_string(iter);
	proposalMessage.P.clear();

	if(dominantMsg == "1"){
		proposalMessage.faulty = true;
		//	set this for leader too
		faultyBlock = true;
	}

	if(!prevAcceptedValue){
		/*
		 *		No value accepted in P Leader choosing a value to propose
		 *		choose any value to propose if no value has been accepted in P
		 *		value proposed from here.
		*/
		proposalMessage.message = {std::to_string(iter), "PROPOSE", consensusTx};
		proposalMessage.P = P;
		proposalMessage.value = consensusTx;
		valueFromLeader = consensusTx;

	}else{
		/*
		 *		Some values were Previously accepted
		 *		choose a highest non-zero iteration number value to propose under P
		*/
		int highIter= 0;
		int index = 0;
		string highIterVal;
		for(auto &n: P.notify){
			assert(n.size() == 3);
			if(n[0] != "0") {
				if (std::stoi(n[0]) >= highIter){
					highIterVal = n[2];
					highIter = index;
				}

			}
		}
		for(auto &s: P.status){
			assert(s.size() == 4);
			if(s[3] != "0"){
				if(std::stoi(s[3]) >= highIter){
					highIterVal = s[2];
					highIter = index;
				}
			}
		}

		proposalMessage.message = {std::to_string(iter), "PROPOSE", highIterVal};
		proposalMessage.P = P;
		proposalMessage.value = highIterVal;

	}

	populateOutStream(proposalMessage);
	setSyncBFTState(2);
}

void syncBFT_Peer::commitFromLeader(){
	Logger::instance()->log("---COMMIT FROM LEADER---\n");

	syncBFTmessage commitMessage;
	//searching for notify messages

	_inStream.insert(_inStream.end(), notifyMessagesPacket.begin(), notifyMessagesPacket.end());
	for(auto & i : _inStream){
		if(i.getMessage().type == "NOTIFY"){
			commitMessage = i.getMessage();
			if(i.getMessage().value == acceptedStates.back().value){
				populateOutStream(commitMessage);
				//a notify message is a valid commit message
				return;
			}
		}
	}

	commitMessage.P.clear();

	if(isByzantine() && !faultyBlock){
		valueFromLeader = "CONFLICTING_VALUE";
	}

	commitMessage.type = "FORWARD_PROPOSAL_AND_COMMIT";
	commitMessage.value = valueFromLeader;
	commitMessage.peerId = _id;
	commitMessage.message = {std::to_string(iter),"COMMIT",valueFromLeader};

	populateOutStream(commitMessage);
	setSyncBFTState(3);
}

void syncBFT_Peer::commit(){
	Logger::instance()->log("---COMMIT---\n");

	syncBFTmessage messageToForward;
	syncBFTmessage virtualProposal;

	//check for notify message from the leader
	_inStream.insert(_inStream.end(), notifyMessagesPacket.begin(), notifyMessagesPacket.end());
	for(auto & i : _inStream){
		if(i.getMessage().type == "NOTIFY"){
			virtualProposal = i.getMessage();
			if(i.getMessage().value == acceptedStates.back().value){
				valueFromLeader = i.getMessage().value;
				syncBFTmessage virtualProposalMessage = virtualProposal;
				virtualProposalMessage.P.clear ();
				virtualProposalMessage.type = "FORWARD_PROPOSAL_AND_COMMIT";
				virtualProposalMessage.value = valueFromLeader;
				virtualProposalMessage.message = {std::to_string(iter),"COMMIT",valueFromLeader};

				populateOutStream(virtualProposalMessage);
				Logger::instance()->log("RETURNING EARLY \n");
				return;
			}
		}
	}

	Logger::instance()->log("ID IN COMMIT " + id() + "\n");
	assert(_inStream.size()==1);
	assert(_inStream[0].getMessage().type=="PROPOSE");
	if(isValidProposal(_inStream[0].getMessage())){
		Logger::instance()->log("The proposal from the leader is valid.\n");
		valueFromLeader = (_inStream[0].getMessage().value);
		messageToForward = (_inStream[0].getMessage());
	} else{
		Logger::instance()->log("The proposal from the leader is not valid: NOTHING TO DO.\n");
		valueFromLeader = ("_");
	}

	_inStream.clear();

	if(valueFromLeader != "_"){
		//Forward the proposal as well as commit message
		messageToForward.P.clear ();
		messageToForward.type = "FORWARD_PROPOSAL_AND_COMMIT";
		messageToForward.value = valueFromLeader;
		messageToForward.message = {std::to_string(iter),"COMMIT",valueFromLeader};
		messageToForward.peerId = _id;

		populateOutStream(messageToForward);

	}
	if(messageToForward.faulty){
		faultyBlock = true;
	}
	setSyncBFTState(3);
}

void syncBFT_Peer::notify(){
	Logger::instance()->log("---NOTIFTY---\n");

	bool committed = false;
	_inStream.insert(_inStream.end(), notifyMessagesPacket.begin(), notifyMessagesPacket.end());
	for(auto & i : _inStream){
		assert(i.getMessage().type == "FORWARD_PROPOSAL_AND_COMMIT"||i.getMessage().type == "NOTIFY");
	}

	//traverse through "FORWARD_PROPOSAL" messages and searching for leader equivocation
	Logger::instance()->log("Traversing to search for equivocation.\n");
	for(int i = 0; i < _inStream.size();i++){
		if(isValidProposal(_inStream[i].getMessage())&&_inStream[i].getMessage().value != valueFromLeader){
			Logger::instance()->log("EQUIVOCATION FOUND IN PEER " + id() + "!\n");
			setSyncBFTState(4);
			_inStream.clear();
			return;
		}
	}

	Logger::instance()->log("NO EQUIVOCATION!\n");
	std::map<string, int> valueToCommitCount;
	for(auto & i : _inStream){
		if(i.getMessage().type == "FORWARD_PROPOSAL_AND_COMMIT"){
			if(i.getMessage().value == valueFromLeader){
				valueToCommitCount[i.getMessage().value]++;
			}
		}
	}

	//if replica receives f+1 valid commit requests
	for (auto const& pair : valueToCommitCount)
	{
		if (pair.second >= (committeeSize - 1)/2 +1){
			committed = true;
            std::cout<< "committed"<< committed << std::endl;
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
	_inStream.clear();
	setSyncBFTState(4);

	if(committed){
		syncBFTmessage bcMessage;
		bcMessage.peerId = _id;
		bcMessage.type="NOTIFY";
		bcMessage.message = {std::to_string(iter), "NOTIFY",valueFromLeader};
		bcMessage.cc.clear ();

        std::stringstream ss;
        for(size_t i = 0; i < cc.commit.size(); ++i)
        {
            for(size_t j = 0; j< cc.commit[i].size();++j){
                if(i != 0){
                    ss<<" ";
                }
                ss<<cc.commit[i][j];
            }
        }
        std::string ccString = ss.str();

//        std::string ccHash = sha256(ccString);
        std::string ccHash = "SAMPLE";

        bcMessage.ccHash = ccHash;

        bcMessage.iter = std::to_string(iter);
		bcMessage.value = valueFromLeader;

		populateOutStream(bcMessage);

		if(!isLeader()){
			terminated = true;
			Logger::instance()->log(id() + " is terminated " + std::to_string(isTerminated()) + "\n");
		}

		//write to blockchain
		createBlock({"All"});

	}else{
		Logger::instance()->log("COMMITTED IS FALSE!\n");
	}
	setSyncBFTState(4);
}


/*
 * 0,1,2,3 represent STATUS_SEND, STATUS_COLLECT_AND_PROPOSE, COMMIT, NOTIFY respectively,
 */
void syncBFT_Peer::run(){

	if(terminated){
		Logger::instance()->log("INSIDE RUN BUT TERMINATED ALREADY!\n");
	}else{
		if(!terminated) {
			if (syncBFTsystemState >= syncBFTstate) {
				switch (syncBFTsystemState) {
					case 0:
					case 4:
						Logger::instance()->log("----------------------------------------------------------------------STATUS SEND\n");
						currentStatusSend();
						break;
					case 1:
						Logger::instance()->log("----------------------------------------------------------------------PROPOSE\n");
						if (isLeader()) {
							propose();
						}
						break;
					case 2:
						Logger::instance()->log("----------------------------------------------------------------------COMMIT\n");
						if (isLeader())
							commitFromLeader();
						else
							commit();
						break;
					case 3:
						Logger::instance()->log("----------------------------------------------------------------------NOTIFY\n");
						notify();
						break;
				}
			} else {
				Logger::instance()->log(" DOING NOTHING, JUST WAITING FOR MESSAGES UNTIL MAX DELAY\n");
			}
		}
	}
}

void syncBFT_Peer::preformComputation(){
	run();
}


void syncBFT_Peer::refreshSyncBFT(){
	iter = 0;
	P.clear();

	acceptedStates.clear();
	acceptedState as1("_","0", {});
	acceptedStates.push_back (as1);
	terminated = true;
	valueFromLeader.clear();
	syncBFTstate = 0;
	syncBFTsystemState = 0;
	_inStream.clear();
	_outStream.clear();
	notifyMessagesPacket.clear();
	cc.clear();
	P.clear();
	_busy = false;
	committeeSize = 0;
	dagBlocks.clear();
	leaderId.clear();
	statusMessageToSelf = nullptr;
	for(auto & _channel : _channels){
		_channel.second.clear();
	}
	consensusTx.clear();
	faultyBlock = false;
}

void syncBFT_Peer::refreshInstream(){
	_inStream.clear();
}

void syncBFT_Peer::sendBlock(){
	for(auto & _neighbor : committeeNeighbours) {
		Logger::instance()->log("Peer " + id() + " SENDING BLOCK TO " + _neighbor.second->id() + "\n");
		Peer<syncBFTmessage> *peer = _neighbor.second;
		Packet<syncBFTmessage> newMessage("", peer->id(), _id);
		messageToSend.peerId = id();
		messageToSend.dagBlockFlag = true;
		messageToSend.dagBlock = *minedBlock;
		newMessage.setBody(messageToSend);
		_outStream.push_back(newMessage);
	}
}

void syncBFT_Peer::makeRequest(){
}

void syncBFT_Peer::makeRequest(const vector<syncBFT_Peer *>& committeeMembers, const std::string& tx) {
	Logger::instance()->log("Peer " + id() + " transmitting the transaction " + tx + "\n");
	syncBFTmessage txMessage;
	txMessage.peerId = id();
	txMessage.message.push_back(tx+"_"+id());
    txMessage.submissionRound = _clock;
	txMessage.txFlag = true;
	for(auto &committeePeer: committeeMembers){
		Logger::instance()->log("Peer " + id() + " transmitting the transaction " + tx + " to " + committeePeer->id() + "\n");
		if(!(committeePeer->id()==id())){
			Packet<syncBFTmessage> newMessage("", committeePeer->id(), id());
			newMessage.setBody(txMessage);
			_outStream.push_back(newMessage);
		}else{
			consensusTx = txMessage.message[0];
		}
	}
	this->transmit();
	messageToSend= {};
}

void syncBFT_Peer::updateDAG() {
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
		Logger::instance()->log("BLOCK FROM " + i.getMessage().peerId + " TO " + id() + "\n");
		//todo what will happen if a mined block is received before or at the same time a transaction is received.
		if(i.getMessage().txFlag){
			//check to see if the transaction has already been added to the dag
			if(dag.transactionInDag(i.getMessage().message[0]))
				continue;
			else{
				//assert(false);
			}
			continue;
		}
		assert(!i.getMessage().txFlag);
		if(i.getMessage().dagBlockFlag){
            DAGBlock newBlock = i.getMessage().dagBlock;
            newBlock.setConfirmedRound(_clock);
            newBlock.setSubmissionRound(submissionRound);
            newBlock.setSecruityLevel(committeeNeighbours.size() + 1);
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

void syncBFT_Peer::receiveTx() {
//	can receive from itself
	assert(_inStream.size() == 1 || !consensusTx.empty());
	if(consensusTx.empty()){
		consensusTx = _inStream[0].getMessage().message[0];
        submissionRound = _inStream[0].getMessage().submissionRound;
        assert(submissionRound != -1);
	}
	else{
		assert(!consensusTx.empty());
	}
	_inStream.clear();
}
