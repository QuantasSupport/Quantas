//
// Created by srai on 3/31/19.
//

#ifndef bitcoinPeer_hpp
#define bitcoinPeer_hpp

#include <random>

#include "./../Common/Peer.hpp"
#include "./../Common/Blockchain.hpp"

struct bCoinMessage {
    std::string 							peerId;
    std::string 							iter;
    std::vector<std::string> 				message;
    int                                     length = 0;
    Block                                   block;

    bCoinMessage(const bCoinMessage& rhs){
        peerId = rhs.peerId;
        iter = rhs.iter;
        message = rhs.message;
        length = rhs.length;

    }
    bCoinMessage() = default;

    bCoinMessage& operator=(const bCoinMessage& rhs){
        peerId = rhs.peerId;
        iter = rhs.iter;
        message = rhs.message;
        length = rhs.length;
        return *this;
    }

    ~bCoinMessage() = default;

};


class bCoin_Peer : public Peer<bCoinMessage> {

    int 											counter;
    Blockchain*			 							blockchain;
    int                                             mineNextAt;

public:
    static std::default_random_engine       generator;
    static std::binomial_distribution<int>  distribution;

    bCoin_Peer																	    (std::string);
    bCoin_Peer                                                                      (const bCoin_Peer &rhs);
    void                                    setMineNextAt                           (int iter)                  { mineNextAt = iter; }
    int                                     getMineNextAt                           ()                          { return mineNextAt; }
    void 									setBlockchain							(const Blockchain &bChain)  { *(this->blockchain) = bChain; }
    Blockchain*                             getBlockchain                           ()                          { return this->blockchain; }

    void 									preformComputation						() override;

    bool 									mineBlock                               ();
    void                                    receiveBlock                            ();
    void                                    sendBlock                               ();

    ~bCoin_Peer                                                                     ()                          { delete blockchain; }
    void                                    makeRequest                             () override                 {}

};


#endif
