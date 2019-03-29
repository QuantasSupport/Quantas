//
// Created by srai on 3/15/19.
//

#ifndef BCPeer_hpp
#define BCPeer_hpp

#include <cassert>
#include <algorithm>

#include "Peer.hpp"
#include "Blockchain.hpp"


struct syncBFTmessage {
    std::string 							peerId;
    std::string 							type;
    std::string 							info;
    std::string 							value;
    std::string 							iter;
    std::vector<std::string> 				message;
    std::string 							statusCert;
    std::vector<std::unique_ptr<syncBFTmessage>> 			commitCertificate;
    std::vector<std::unique_ptr<syncBFTmessage>> 			P;

    syncBFTmessage(const syncBFTmessage& rhs){
        peerId = rhs.peerId;
        type = rhs.type;
        info = rhs.info;
        value = rhs.value;
        iter = rhs.iter;
        message = rhs.message;
        statusCert = rhs.statusCert;
        for(auto &i:rhs.commitCertificate){
            commitCertificate.push_back (std::make_unique<syncBFTmessage> (*i));
        }
        for(auto &i:rhs.P){
            P.push_back (std::make_unique<syncBFTmessage> (*i));
        }

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
        for(auto &i:rhs.commitCertificate){
            commitCertificate.push_back (std::make_unique<syncBFTmessage> (*i));
        }
        for(auto &i:rhs.P){
            P.push_back (std::make_unique<syncBFTmessage> (*i));
        }
        return *this;
    }

    ~syncBFTmessage() = default;

};


class syncBFT_Peer : public Peer<syncBFTmessage> {

    struct acceptedState {
        std::string 							value;
        std::string 							valueAcceptedAt;
        std::vector<unique_ptr<syncBFTmessage>> 			commitCertificate;

        acceptedState(string v, string vAt , std::vector<unique_ptr<syncBFTmessage>> cc){
            value = v;
            valueAcceptedAt = vAt;
            for(auto &c: cc){
                commitCertificate.push_back (std::make_unique<syncBFTmessage> (*c));
            }
        }
        acceptedState(const acceptedState& rhs){
            value = rhs.value;
            valueAcceptedAt = rhs.valueAcceptedAt;
            for(auto &c: rhs.commitCertificate){
                commitCertificate.push_back (std::make_unique<syncBFTmessage> (*c));
            }
        };

        acceptedState& operator=(const acceptedState&) = delete;

        ~acceptedState() = default;
    };


    int 											counter;
    Blockchain*			 							blockchain;
    std::vector<unique_ptr<syncBFTmessage>> 		P;
    std::vector<unique_ptr<acceptedState>> 			acceptedStates ;
    bool 											terminated ;
    std::string 									valueFromLeader;
    std::vector<unique_ptr<syncBFTmessage>> 		commitCertificate;
    int 											syncBFTstate ;
    unique_ptr<syncBFTmessage> 						statusMessageToSelf;
    bool 											byzantineFlag ;
    int 											syncStatus ;
    std::vector<Packet<syncBFTmessage>> 			notifyMessages;


public:
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

    void 									createBlock								(std::set<std::string>);
    bool 									isLeader								() { return leaderId == _id; }
    void 									run										();
    void 									currentStatusSend						();
    void 									propose									();
    void 									commitFromLeader						();
    void 									commit									();
    void 									notify									();
    void 									receiveNotifyMessage					();
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
    void                                    makeRequest(){}

};


#endif
