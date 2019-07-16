//
// Created by srai on 6/8/19.
//

#ifndef bCoin_Committee_hpp
#define bCoin_Committee_hpp


#include "DS_bCoin_Peer.hpp"
#include "./../Common/committee.hpp"

static std::vector<int> someByzantineCount;
static std::vector<int> moreThanHalfByzantineCount;
static std::map<int, std::map<int, int>> securityLevelToByznainteFound = {};

class bCoin_Committee : public Committee<DS_bCoin_Peer>{
private:
	int 									firstMinerIndex = -1;

public:
	bCoin_Committee															(std::vector<DS_bCoin_Peer *> , DS_bCoin_Peer *, std::string , int);
	bCoin_Committee															(const bCoin_Committee&);
	bCoin_Committee&						operator=						(const bCoin_Committee &rhs);

	int 									getFirstMinerIndex				(){return firstMinerIndex;}

	void									preformComputation				();
	void									initiate						(int submissionRound);
	bool									checkForConsensus				();
	void									propagateBlock					();


};
#endif //bCoin_Committee_hpp
