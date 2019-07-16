//
// Created by srai on 3/31/19.
//

#ifndef DS_bitcoinPeer_hpp
#define DS_bitcoinPeer_hpp
#include <utility>
#include <random>
#include <deque>
#include "./../Common/Peer.hpp"
#include "./../Common/DAG.hpp"

struct DS_bCoinMessage {
	std::string 							peerId ;
	std::vector<std::string> 				message;
	int                                     length              = 0;
	DAGBlock                                dagBlock;
	bool                                    txFlag              = false;
	bool                                    dagBlockFlag        = false;
	bool									blockByzantineFlag  = false;
    int                                     submissionRound     = -1;
    
	DS_bCoinMessage(const DS_bCoinMessage& rhs){
		peerId = rhs.peerId;
		message = rhs.message;
		length = rhs.length;
		txFlag = rhs.txFlag;
		dagBlock = rhs.dagBlock;
		dagBlockFlag = rhs.dagBlockFlag;
		blockByzantineFlag = rhs.blockByzantineFlag;
        submissionRound = rhs.submissionRound;

	}
	DS_bCoinMessage() = default;

	DS_bCoinMessage& operator=(const DS_bCoinMessage& rhs){
		if(this == &rhs)
			return *this;
		peerId = rhs.peerId;
		message = rhs.message;
		length = rhs.length;
		txFlag = rhs.txFlag;
		dagBlock = rhs.dagBlock;
		dagBlockFlag = rhs.dagBlockFlag;
		blockByzantineFlag = rhs.blockByzantineFlag;
        submissionRound = rhs.submissionRound;
		return *this;
	}

	~DS_bCoinMessage() = default;

};


class DS_bCoin_Peer : public Peer<DS_bCoinMessage> {

	int 											counter;
	int                                             mineNextAt;
    int                                             startedMiningAt;
	std::string			                            consensusTx         = "";
	DS_bCoinMessage                                 messageToSend       = {};
	std::map<std::string, Peer<DS_bCoinMessage>* >	committeeNeighbours;
    bool                                            terminated          = false;
    int                                             submissionRound     = -1;
    
public:
	static std::default_random_engine               generator;
	static std::binomial_distribution<int>          distribution;
	DAG												dag;
	std::vector<DAGBlock>							dagBlocks={};
	DAGBlock										*minedBlock;
	void											updateDAG();

	explicit DS_bCoin_Peer																	(std::string);
	DS_bCoin_Peer                                                                      		(const DS_bCoin_Peer &rhs);
	~DS_bCoin_Peer                                                                     		() override                                                 { delete minedBlock; }

    // mutators
    void                                            makeRequest                             () override;
    void                                            makeRequest                             (const vector<DS_bCoin_Peer *>& committeeMembers, const std::string& tx, int submissionRound);
    void                                            deleteMinedBlock                        ()                                                          { delete minedBlock; minedBlock= nullptr; }
    void                                            resetMiningClock                        ();
    void                                            addToBlocks                             ();
    void                                            clearConsensusTx                        ();
    bool                                            mineBlock                               ();
    void                                            receiveMsg                              ();
    void                                            sendBlock                               ();
    void                                            preformComputation                      () override;
    void                                            resetMineNextAt                         ()                                                          { mineNextAt = DS_bCoin_Peer::distribution(DS_bCoin_Peer::generator); }
    
    // getters
    bool                                            isTerminated                            ()                                                          { return terminated; }
    std::string                                     getConsensusTx                          ()                                                          { return consensusTx; }
    DAG                                             getDAG                                  ()const                                                     { return this->dag; }
    int                                             getMineNextAt                           ()                                                          { return mineNextAt; }
    std::map<std::string, Peer<DS_bCoinMessage>* >  getCommitteeNeighbours                  ()                                                          { return committeeNeighbours ; }
    
    // setters
    void                                            setTerminated                           (bool flag)                                                 { this->terminated = flag;}
    void                                            setDAG                                  (const DAG &dagChain)                                       { this->dag = dagChain; }
    void                                            setMineNextAt                           (int iter)                                                  { mineNextAt = iter; }
    void                                            setCommitteeNeighbours                  (std::map<std::string, Peer<DS_bCoinMessage>* > n)          { committeeNeighbours = std::move(n); }
};


#endif
