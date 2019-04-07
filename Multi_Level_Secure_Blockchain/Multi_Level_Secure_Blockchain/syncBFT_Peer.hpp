#include <utility>

#include <utility>

#include <utility>

//
// Created by srai on 3/15/19.
//

#ifndef BCPeer_hpp
#define BCPeer_hpp

#include <cassert>
#include <algorithm>
#include <queue>

#include "Peer.hpp"
#include "Blockchain.hpp"

struct proposal{
    std::vector<vector<string>> status;
    std::vector<vector<string>> notify;

    int size() const{
        return (status.size() + notify.size());
    }

    void clear() {
        status.clear();
        notify.clear();
    }

};

struct commitCertificate{
    std::vector<vector<string>> commit;
    void clear(){
        commit.clear();
    }
};

struct syncBFTmessage {
    std::string 							peerId;
    std::string 							type;
    std::string 							info;
    std::string 							value;
    std::string 							iter;
    std::vector<std::string> 				message;
    std::string 							statusCert;
    proposal                                P;
    commitCertificate                       cc;

    syncBFTmessage(const syncBFTmessage& rhs){
        peerId = rhs.peerId;
        type = rhs.type;
        info = rhs.info;
        value = rhs.value;
        iter = rhs.iter;
        message = rhs.message;
        statusCert = rhs.statusCert;
        cc = rhs.cc;
        P = rhs.P;

    }
    syncBFTmessage() = default;


    syncBFTmessage& operator=(const syncBFTmessage& rhs){
        peerId = rhs.peerId;
        type = rhs.type;
        info = rhs.info;
        value = rhs.value;
        iter = rhs.iter;
        message = rhs.message;
        statusCert = rhs.statusCert;
        cc = rhs.cc;
        P = rhs.P;
        return *this;
    }

    ~syncBFTmessage() = default;

};


class syncBFT_Peer : public Peer<syncBFTmessage> {

    struct acceptedState {
        std::string 							value;
        std::string 							valueAcceptedAt;
        commitCertificate                       cc;

        acceptedState(string v, string vAt , commitCertificate c):value(std::move(v)), valueAcceptedAt(std::move(vAt)), cc(std::move(c)){}
        acceptedState(const acceptedState& rhs){
            value = rhs.value;
            valueAcceptedAt = rhs.valueAcceptedAt;
            cc = rhs.cc;
        };

        acceptedState& operator=(const acceptedState&) = delete;

        ~acceptedState() = default;
    };


    int 											counter;
    Blockchain*			 							blockchain;
    proposal                                   		P;
    std::vector<acceptedState>           			acceptedStates ;
    bool 											terminated ;
    std::string 									valueFromLeader;
    commitCertificate                               cc;
    int 											syncBFTstate ;
    std::unique_ptr<syncBFTmessage>           		statusMessageToSelf;
    bool 											byzantineFlag ;
    std::vector<Packet<syncBFTmessage>> 			notifyMessagesPacket;


public:
    static std::queue<std::string>          txQueue;
    static string                           txToConsensus;
    static std::vector<std::string> 		leaderIdCandidates;
    static int 								syncBFTsystemState;
    static bool 							changeLeader;
    int 									iter;
    static int 								peerCount;
    static std::string 						leaderId;

    syncBFT_Peer																	(std::string);
    void 									setBlockchain							(const Blockchain &bChain) { *(this->blockchain) = bChain; }
    Blockchain*                             getBlockchain() 						{ return this->blockchain; }
    void 									setByzantineFlag						(bool flag) { byzantineFlag = flag; }
    void 									setSyncBFTState							(int status) { syncBFTstate = status; }
    static std::string 						getLeaderId								() { return leaderId; }
    bool 									getTerminationFlag						() const { return terminated; }

    void 									createBlock								(std::set<std::string>, string);
    bool 									isLeader								() { return leaderId == _id; }
    void 									run										();
    void 									currentStatusSend						();
    void 									propose									();
    void 									commitFromLeader						();
    void 									commit									();
    void 									notify									();
    bool 									isValidProposal							(const syncBFTmessage &message) const { return message.peerId == leaderId; }
    bool 									isValidNotify							(const syncBFTmessage &message) const { return true; }
    bool 									isValidStatus							(const syncBFTmessage &message) const { return true; }
    void 									refreshSyncBFT							();
    static int 								incrementSyncBFTsystemState				();
    static bool 							leaderChange							();
    void 									preformComputation						();
    void                                    populateOutStream                       (syncBFTmessage msg);

    bool									isByzantine								() { return byzantineFlag; }

    ~syncBFT_Peer() 						{ delete blockchain; }
    void                                    makeRequest                             ();

};


#endif
