//
// Created by srai on 6/3/19.
//

#ifndef syncBFT_Committee_hpp
#define syncBFT_Committee_hpp


#include "syncBFT_Peer.hpp"
#include "./../Common/committee.hpp"

class syncBFT_Committee : public Committee<syncBFT_Peer>{
private:
	std::string								status;
	std::string								leaderId;
	int 									syncBFTsystemState  = 0;
	bool									changeLeader        = true;
	std::vector<std::string> 				leaderIdCandidates;
	int 									firstMinerIndex     = -1;
    int                                     clock               = -1;
	bool									defeated = false;

public:
	syncBFT_Committee														(std::vector<syncBFT_Peer *> , syncBFT_Peer *, std::string , int);
	syncBFT_Committee														(const syncBFT_Committee&);
	syncBFT_Committee&						operator=						(const syncBFT_Committee &rhs);

	std::string								getLeaderId						()const                                                                 {return leaderId;}
	int 									getFirstMinerIndex				()const                                                                 {return firstMinerIndex;}
    int                                     getClock                        ()const                                                                 {return clock;};
    
    void                                    clockTik                        ()                                                                      {clock++;};
	void 									preformComputation				() override;
	void 									receiveTx						();
	int 									incrementSyncBFTsystemState		();
	void 									nextState						(int, int);
	void 									leaderChange					();
	void									refreshPeers					();
	void									initiate						();
	bool 									getDefeated						()																		{return defeated;}

};

#endif //SyncBFT_Committee_hpp
