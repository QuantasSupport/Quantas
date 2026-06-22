
#ifndef LIGHTNINGPEER_HPP
#define LIGHTNINGPEER_HPP

#include "../Common/Peer.hpp"
#include <utility>
#include <vector>
#include <deque>
#include <list>
#include <algorithm>
#include <cmath>
#include <map>

namespace quantas {
    using std::map;

    struct PaymentChannel {
        int mySideBalance = -1;
        int mySidePending = 0;
        int otherSide = -1;
        int otherSidePending = 0;
        interfaceId otherId = -1;
    };

    // json message
    // type: "txAttempt", "txFailed", "txSucceeded", "newEdge"
    // tx: {
    //     int value = -1;
    //     interfaceId source = -1;
    //     interfaceId target = -1;
    //     map<interfaceId,interfaceId> path = {};
    //     int roundSubmitted = -1;
    // };


    class LightningPeer : public Peer{
    public:
        LightningPeer(NetworkInterface* networkInterface);
        LightningPeer(const LightningPeer&);
        ~LightningPeer() override;


        void initParameters(const std::vector<Peer*>& peers, json parameters) override;
        void performComputation() override;
        void endOfRound(std::vector<Peer*>& peers) override;

        PaymentChannel getChannel(interfaceId otherId) const;
        void addPaymentChannel(PaymentChannel);
        void updateChannel(interfaceId target,
                           int myFunds,
                           int otherFunds,
                           int myPending,
                           int otherPending);

    private:
        struct PendingTransaction {
            json tx;
            bool blockedByRebalance = false;
        };

        // receive messages as necessary
        void checkInStrm();
        bool guardSubmit() const;
        void submitTransaction();
        // build a path from the sorce to the target for the transaction 
        json buildPath(const json& tx) const;

        // returns true if has channel 
        // and enough funds to make the payment. Else false
        bool hasCapacity(interfaceId target, int value) const;
        
        // find/create a cycle going from target -> source
        bool rebalance(interfaceId source, interfaceId target, int value);

        void queueTransaction(json tx, bool blockedByRebalance = false);
        void tryQueuedTransactions();
        PaymentChannel* findMutableChannel(interfaceId otherId);
        const PaymentChannel* findChannel(interfaceId otherId) const;
        bool reserveOutgoing(interfaceId target, int value);
        bool applyCapacityChange(interfaceId target, int value);
        void releaseOutgoing(interfaceId target, int value);
        void settleOutgoing(interfaceId target, int value);
        void settleIncoming(interfaceId source, int value);
        interfaceId randomTarget() const;
        json buildRebalanceTransaction(interfaceId target, int value);

        int submitRate = 0;
        bool torus = true;
        int height = 1;
        int width = 1;
        int averageDegree = 4;
        int minInitialChannelCapacity = 100;
        int maxInitialChannelCapacity = 100;
        int minInitialLocalBalanceRatioPercent = 50;
        int minPaymentAmount = 1;
        int maxPaymentAmount = 1;
        int minPeerWalletBalance = 0;
        int maxPeerWalletBalance = 0;
        int funds = 0;
        int fundsAvailable = 0;
        int localTransactionCounter = 0;

        std::vector<PaymentChannel> channels;
        std::list<PendingTransaction> pendingTransactions;
        std::vector<interfaceId> knownPeerIds;
        std::map<interfaceId, size_t> peerIndexById;
        std::map<interfaceId, std::set<interfaceId>> topologyView;
        int throughput = 0;
        int txWait = 0;
        int rebalancesAttempted = 0;
        int successfulRebalances = 0;
        int capacityChanges = 0;
        int submittedTransactions = 0;
    };

}

#endif // LIGHTNINGPEER_HPP
