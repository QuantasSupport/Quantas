//
// Created by srai on 3/15/19.
//

#ifndef syncBFT_Peer_hpp
#define syncBFT_Peer_hpp

#include <utility>
#include <cassert>
#include <algorithm>
#include <queue>

#include "./../Common/Peer.hpp"
#include "./../Common/DAG.hpp"

struct proposal{
	std::vector<vector<string>> 					status;
	std::vector<vector<string>> 					notify;

	int 											size									() const{ return (status.size() + notify.size()); }

	void 											clear									() { status.clear(); notify.clear(); }
};

struct commitCertificate{
	std::vector<vector<string>> 					commit;
	void 											clear									() { commit.clear(); }
};

struct syncBFTmessage {
	std::string 									peerId;
	std::string 									type;
	std::string 									value;
	std::string 									iter;
	std::vector<std::string> 						message;
	std::string 									statusCert;
	proposal                                		P;
	commitCertificate                       		cc;
	std::string										ccHash;
	DAGBlock                                		dagBlock;
	bool                                    		txFlag          = false;
	bool                                    		dagBlockFlag    = false;
	bool											faulty 			= false;
    int                                             submissionRound = 0;

	syncBFTmessage() = default;

	syncBFTmessage(const syncBFTmessage& rhs){
		peerId = rhs.peerId;
		type = rhs.type;
		value = rhs.value;
		iter = rhs.iter;
		message = rhs.message;
		statusCert = rhs.statusCert;
		P = rhs.P;
		cc = rhs.cc;
		dagBlock = rhs.dagBlock;
		txFlag = rhs.txFlag;
		dagBlockFlag = rhs.dagBlockFlag;
		faulty = rhs.faulty;
		ccHash = rhs.ccHash;
	}

	syncBFTmessage& operator=(const syncBFTmessage& rhs){
		if(this == &rhs)
			return *this;
		peerId = rhs.peerId;
		type = rhs.type;
		value = rhs.value;
		iter = rhs.iter;
		message = rhs.message;
		statusCert = rhs.statusCert;
		P = rhs.P;
		cc = rhs.cc;
		dagBlock = rhs.dagBlock;
		txFlag = rhs.txFlag;
		dagBlockFlag = rhs.dagBlockFlag;
		faulty = rhs.faulty;
		ccHash = rhs.ccHash;
		return *this;
	}

	~syncBFTmessage() = default;

};


class syncBFT_Peer : public Peer<syncBFTmessage> {
	struct acceptedState {
		std::string 								value;
		std::string 								valueAcceptedAt;
		commitCertificate                       	cc;

		acceptedState(string v, string vAt , commitCertificate c){
			value = std::move(v);
			valueAcceptedAt = std::move(vAt);
			cc = std::move(c);
		}

		acceptedState(const acceptedState& rhs){
			value = rhs.value;
			valueAcceptedAt = rhs.valueAcceptedAt;
			cc = rhs.cc;
		};

		acceptedState& operator=(const acceptedState &rhs){
			if(this ==&rhs)
				return *this;
			value = rhs.value;
			valueAcceptedAt = rhs.valueAcceptedAt;
			cc = rhs.cc;
			return *this;
		}

		~acceptedState() = default;
	};


	proposal                                   		P;
	std::vector<acceptedState>           			acceptedStates ;
	std::string 									valueFromLeader;
	commitCertificate                               cc;
	int 											syncBFTstate ;
	std::unique_ptr<syncBFTmessage>           		statusMessageToSelf;
	std::vector<Packet<syncBFTmessage>> 			notifyMessagesPacket;
	std::map<std::string, Peer<syncBFTmessage>* >	committeeNeighbours;
	syncBFTmessage                                  messageToSend           = {};
	DAGBlock										*minedBlock;
	DAG												dag;
	std::string 									leaderId                = "";
	bool 											terminated              = true;
	int 											committeeSize           = 0;
	std::string                           			consensusTx             = "";
    int                                             submissionRound         = -1;
	bool											faultyBlock 			= false;

public:
	int 											iter;
	int												syncBFTsystemState      = 0;
	std::vector<DAGBlock>							dagBlocks               = {};

	syncBFT_Peer																				(std::string);
	syncBFT_Peer																				(const syncBFT_Peer &rhs);
	~syncBFT_Peer																				()                                                  { delete minedBlock; }
	syncBFT_Peer&									operator=									(const syncBFT_Peer &rhs);

    // setters
	void 											setDAG										(const DAG &dagChain)                               { this->dag = dagChain; }
	void 											setSyncBFTState								(int status)                                        { syncBFTstate = status; }
	void 											setCommitteeNeighbours						(std::map<std::string, Peer<syncBFTmessage>* > n)   { committeeNeighbours = std::move(n); }
	void 											setTerminated								(bool flag)                                         { this->terminated = flag;}
	void		 									setLeaderId									(std::string id)                                    { this->leaderId = std::move(id); }
	void		 									setCommitteeSize							(int size)                                          { this->committeeSize = std::move(size); }

    // getters
	DAG                             				getDAG                           			()                                                  { return this->dag; }
    DAG                                             getDAG                                      ()const                                             { return this->dag; }
	std::map<std::string, Peer<syncBFTmessage>* > 	getCommitteeNeighbours						()                                                  { return committeeNeighbours; }
	std::string 									getLeaderId									()                                                  { return leaderId; }
	bool 											getTerminationFlag							()const                                             { return terminated; }
	std::string 									getConsensusTx								()                                                  { return consensusTx; }
	bool 											isTerminated								()                                                  { return terminated; }
    bool                                            isValidProposal                             (const syncBFTmessage &message) const               { return message.peerId == leaderId; }
    bool                                            isValidNotify                               (const syncBFTmessage &message) const               { return true; }
    bool                                            isValidStatus                               (const syncBFTmessage &message) const               { return true; }
    bool                                            isLeader                                    ()const                                             { return leaderId == _id; }
    
    // mutators
	void 											createBlock									(const std::set<std::string>&);
	void 											run											();
	void 											currentStatusSend							();
	void 											propose										();
	void 											commitFromLeader							();
	void 											commit										();
	void 											notify										();
	void 											refreshSyncBFT								();
	void 											refreshInstream								();
	void 											preformComputation							() override;
	void                                    		populateOutStream                       	(const syncBFTmessage& msg);
	void                                    		makeRequest                             	() override;
	void                                    		makeRequest                             	(const std::vector<syncBFT_Peer*>&, const std::string&);
	void                                    		sendBlock									();
	void											updateDAG									();
	void 											receiveTx									();

};


#endif
