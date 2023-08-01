/*
Copyright 2023

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version. QUANTAS is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include "TrailPeer.hpp"
#include <iostream>

namespace quantas {
// -- implementation of underlying consensus algorithm --

void PBFTRequest::updateConsensus() {
    if (leader && status == "pre-prepare") {
        status = "prepare";
        TrailMessage message;
        message.sequenceNum = sequenceNum;
        message.messageType = "pre-prepare";
        message.trans = transaction;
        message.senderID = ownID;
        addToConsensus(message, ownID);
        outbox.push_back(message);
    } else if (status == "pre-prepare") {
        if (statusCount["pre-prepare"] > 0) {
            status = "prepare";
            TrailMessage message;
            message.sequenceNum = sequenceNum;
            message.messageType = "prepare";
            message.trans = transaction;
            message.senderID = ownID;
            addToConsensus(message, ownID);
            outbox.push_back(message);
        }
    }
    if (status == "prepare") {
        if (statusCount["prepare"] >= 2 * ((consensusPeers - 1) / 3)) {
            status = "commit";
            TrailMessage message;
            message.sequenceNum = sequenceNum;
            message.messageType = "commit";
            message.trans = transaction;
            message.senderID = ownID;
            addToConsensus(message, ownID);
            outbox.push_back(message);
        }
    }
    if (status == "commit") {
        if (statusCount["commit"] >= 2 * ((consensusPeers - 1) / 3) + 1) {
            status = "committed";
        }
    }
}

void PBFTRequest::addToConsensus(TrailMessage message, int sourceID) {
    // increment the number of times we've seen this type of message for this
    // request
    ++statusCount[message.messageType];
}

bool PBFTRequest::consensusSucceeded() const { return status == "committed"; }

Simulation<quantas::TrailMessage, quantas::TrailPeer> *generateSim() {

    Simulation<quantas::TrailMessage, quantas::TrailPeer> *sim =
        new Simulation<quantas::TrailMessage, quantas::TrailPeer>;
    return sim;
}
void CrossShardPBFTRequest::addToConsensus(TrailMessage message, int sourceID) {
    if (individualMessageCounts.count(
            std::make_pair(message.messageType, sourceID)
        ) == 0) {
        individualMessageCounts[std::make_pair(message.messageType, sourceID)] =
            0;
    }
    individualMessageCounts[std::make_pair(message.messageType, sourceID)]++;
    int f = (neighborhoodSize - 1) / 3;
    if (message.messageType == "pre-prepare" &&
        individualMessageCounts[std::make_pair("pre-prepare", sourceID)] >=
            f + 1) {
        ++statusCount["pre-prepare"];
    } else if (
        (message.messageType == "prepare" || message.messageType == "commit") &&
        individualMessageCounts[std::make_pair(message.messageType, sourceID)] >= 2*f+1){
        ++statusCount[message.messageType];
    }
}

// -- peer class that participates directly in the network --

// gives each peer a neighborhood and gives each neighborhood a set of wallets.
// precondition: "validatorNeighborhoods", "neighborhoodSize", and
// "walletsPerNeighborhood" are set as parameters in the input file
// postcondition: maxNeighborhoodSize is set; neighborhoodCount is set;
// byzantineRound is set; walletsForNeighborhoods contains a vector of
// LocalWallets for each neighborhood; neighborhoodsForPeers maps a peer's id to
// the index of the neighborhood it's in; static members are reset
void TrailPeer::initParameters(
    const vector<Peer<TrailMessage> *> &_peers, json parameters
) {
    // divide peers into neighborhoods: if the neighborhood size is 5, then
    // the first 5 peers are placed in a neighborhood, then the next 5
    // peers, and so on. if the total number of peers is not a multiple of
    // 5, an "extra" neighborhood stores the spares. then, each neighborhood
    // gets a set of wallets to take care of.

    std::cout << "started initParameters" << std::endl;

    corruptNeighborhoods.clear();
    previousSequenceNumber = -1;
    issuedCoins = 0;
    walletsForNeighborhoods = std::vector<std::vector<LocalWallet>>();
    neighborhoodsForPeers = std::unordered_map<long, int>();
    neighborhoods = std::vector<Neighborhood>();

    validatorNeighborhoods = parameters["validatorNeighborhoods"];
    maxNeighborhoodSize = parameters["neighborhoodSize"];
    const int walletsPerNeighborhood = parameters["walletsPerNeighborhood"];
    if (parameters.contains("byzantineRound")) {
        byzantineRound = parameters["byzantineRound"];
        if (!parameters.contains("maliciousNeighborhoods")) {
            std::cout << "WARNING: parameter byzantineRound is set but "
                         "maliciousNeighborhoods is not specified. Defaulting "
                         "to 1 malicious neighborhood\n";
            maliciousNeighborhoods = 1;
        }
    }
    if (parameters.contains("submitRate")) {
        submitRate = parameters["submitRate"];
    }
    if (parameters.contains("maliciousNeighborhoods")) {
        maliciousNeighborhoods = parameters["maliciousNeighborhoods"];
        if (!parameters.contains("byzantineRound")) {
            std::cout
                << "WARNING: parameter maliciousNeighborhoods is set but "
                   "byzantineRound is not specified. Defaulting "
                   "to creating the malicious neighborhoods at round 10\n";
            byzantineRound = 10;
        }
    }
    if (parameters.contains("attemptRollback")) {
        attemptRollback = parameters["attemptRollback"];
    } else {
        attemptRollback = false;
    }

    LogWriter::getTestLog()["roundInfo"]["roundCount"] = getLastRound() + 1;
    LogWriter::getTestLog()["roundInfo"]["byzantineRound"] = byzantineRound;
    LogWriter::getTestLog()["peerInfo"]["peerCount"] = _peers.size();

    neighborhoodCount =
        ceil(static_cast<float>(_peers.size()) / maxNeighborhoodSize);
    walletsForNeighborhoods.resize(neighborhoodCount);
    vector<WalletLocation> allWallets;
    for (int i = 0; i < neighborhoodCount; ++i) {
        Neighborhood newNeighborhood;
        newNeighborhood.id = i;
        newNeighborhood.leader = i * maxNeighborhoodSize;
        int actualNeighborhoodSize = std::min(
            maxNeighborhoodSize,
            static_cast<int>(_peers.size()) - i * maxNeighborhoodSize
        );
        for (int n = 0; n < actualNeighborhoodSize; ++n) {
            const long peerID = i * maxNeighborhoodSize + n;
            newNeighborhood.memberIDs.insert(peerID);
            neighborhoodsForPeers[peerID] = i;
        }
        for (int j = 0; j < walletsPerNeighborhood; ++j) {
            LocalWallet w = {
                static_cast<int>(allWallets.size()), newNeighborhood, {}, {}};
            walletsForNeighborhoods[i].push_back(w);
            allWallets.push_back({w.address, newNeighborhood});
        }
        neighborhoods.push_back(newNeighborhood);
    }

    LogWriter::getTestLog()["walletInfo"]["walletCount"] = allWallets.size();

    const int coinsPerWallet = 5;
    int fakeHistoryLength =
        std::max(4, validatorNeighborhoods + 1 + (attemptRollback ? 1 : 0));
    int neighborhood = 0;
    // temporarily maps wallet addresses to past coins
    std::map<int, std::vector<Coin>> pastCoins;
    for (auto &wallets : walletsForNeighborhoods) {
        for (auto &wallet : wallets) {
            for (int i = 0; i < coinsPerWallet; ++i) {
                // create coin with no history
                Coin c = {issuedCoins++, {}};

                // add the coin history
                std::vector<int> nbrhoodIndexes;
                for (int j = 0; j < neighborhoodCount; j++) {
                    if (j != neighborhood) {
                        nbrhoodIndexes.push_back(j);
                    }
                }

                if (nbrhoodIndexes.size() < fakeHistoryLength - 1) {
                    std::cerr << "Not enough neighborhoods to create coin "
                                 "history of length "
                              << fakeHistoryLength << std::endl;
                    exit(1);
                }

                std::shuffle(
                    nbrhoodIndexes.begin(), nbrhoodIndexes.end(),
                    RANDOM_GENERATOR
                );
                WalletLocation sender;
                for (int j = 0; j < fakeHistoryLength - 1; j++) {
                    if (j == 0) {
                        sender = walletsForNeighborhoods[nbrhoodIndexes.back(
                        )][randMod(walletsPerNeighborhood)];
                        nbrhoodIndexes.pop_back();
                    }
                    WalletLocation reciever =
                        walletsForNeighborhoods[nbrhoodIndexes.back(
                        )][randMod(walletsPerNeighborhood)];
                    nbrhoodIndexes.pop_back();
                    c.history.push_back({sender, reciever});
                    sender = c.history.back().receiver;
                }
                c.history.push_back(TransactionRecord{
                    fakeHistoryLength > 1 ? c.history.back().receiver
                                          : allWallets[nbrhoodIndexes.back()],
                    wallet});

                for (const auto &t : c.history) {
                    pastCoins[t.sender.address].push_back(c);
                }
                wallet.coins.insert({c.id, c});
            }
        }
        ++neighborhood;
    }

    for (auto &n : _peers) {
        // it would be less weird to set these in the constructor, but
        // because this function (initParameters) is not static, at least
        // one Peer needs to be constructed *before* this function can run
        TrailPeer *e = dynamic_cast<TrailPeer *>(n);
        assert(e); // cast should have succeeded
        e->neighborhoodID = neighborhoodsForPeers[e->id()];
        e->heldWallets = walletsForNeighborhoods[e->neighborhoodID];
        for (LocalWallet &w : e->heldWallets) {
            for (auto &c : w.coins) {
                e->coinDB[c.first] = &c.second;
            }
            for (Coin &c : pastCoins[w.address]) {
                auto insertion = w.pastCoins.insert({c.id, c});
                e->coinDB[c.id] = &(insertion.first->second);
            }
        }
    }

    // make sure there is no pointer re-use
    std::unordered_set<long> ptrs;
    for (auto &n : _peers) {
        TrailPeer *e = dynamic_cast<TrailPeer *>(n);
        assert(e); // cast should have succeeded
        for (auto coinPair : e->coinDB) {
            assert(ptrs.count((long)coinPair.second) == 0);
            ptrs.insert((long)coinPair.second);
        }
    }
    std::cout << "finished initParameters" << std::endl;
}

TrailPeer::TrailPeer(long id) : Peer<TrailMessage>(id) {}

bool TrailPeer::validateTransaction(OngoingTransaction t) {
    // for now, just checking to see if the coin was last known to be in the
    // sending neighborhood (because internal transactions in the neighborhood
    // aren't visible and need signatures to be checked)
    if (corrupt) {
        return true;
    } else {
        if (coinDB.count(t.coin.id) == 0) {
            std::cout << "asked to validate transaction with unknown coin\n";
            return false;
        }
        return (
            t.sender.storedBy.id ==
            coinDB[t.coin.id]->history.back().receiver.storedBy.id
        );
    }
}

void TrailPeer::performComputation() {
    // retrieve messages for requests that are in progress:
    while (!inStreamEmpty()) {
        Packet<TrailMessage> p = popInStream();
        TrailMessage message = p.getMessage();
        int sourceNeighborhood = neighborhoodsForPeers[p.sourceId()];
        int seqNum = message.sequenceNum;
        typename std::unordered_map<int, PBFTRequest>::iterator s;
        typename std::unordered_map<int, CrossShardPBFTRequest>::iterator t;
        if ((s = localRequests.find(seqNum)) != localRequests.end()) {
            if (!validateTransaction(message.trans)) {
                // std::cout << "invalid transaction message ignored\n";
                continue;
            }
            s->second.addToConsensus(message);
        } else if ((s = initRequests.find(seqNum)) != initRequests.end()) {
            if (!validateTransaction(message.trans)) {
                continue;
            }
            s->second.addToConsensus(message);
        } else if ((t = superRequests.find(seqNum)) != superRequests.end()) {
            if (!validateTransaction(message.trans)) {
                // std::cout << "invalid transaction message ignored\n";
                continue;
            }
            t->second.addToConsensus(message, sourceNeighborhood);
        } else {
            // message about previously unknown transaction
            if (message.messageType == "pre-prepare") {
                if (!validateTransaction(message.trans)) {
                    // std::cout << "invalid transaction message ignored\n";
                    continue;
                }
                if (coinDB.count(message.trans.coin.id) > 0) {
                    if (message.trans.sender.storedBy ==
                        message.trans.receiver.storedBy) {
                        ConsensusContacts newContacts =
                            chooseContactsFor(message.trans, true);
                        PBFTRequest request(
                            message.trans, newContacts.getValidatorNodeCount(),
                            message.sequenceNum, false
                        );
                        request.addToConsensus(message);
                        if (errantMessages.count(message.sequenceNum) > 0) {
                            for (const auto &m :
                                 errantMessages[message.sequenceNum]) {
                                request.addToConsensus(m);
                            }
                        }
                        errantMessages.erase(message.sequenceNum);
                        contacts.insert({message.sequenceNum, newContacts});
                        localRequests.insert(
                            {request.getSequenceNumber(), request}
                        );
                    } else {
                        // if you receive a pre-prepare message about a new
                        // cross-shard transaction and you're outside the source
                        // shard, you are part of the cross-shard consensus to
                        // verify the transaction. if you're inside the source
                        // shard, you must be starting the consensus process to
                        // determine what the pre-prepare message that everyone
                        // else gets should be.
                        if (message.trans.sender.storedBy.memberIDs.count(id()
                            ) == 0) {
                            ConsensusContacts newContacts =
                                chooseContactsFor(message.trans, false);
                            CrossShardPBFTRequest request(
                                message.trans, validatorNeighborhoods,
                                message.sequenceNum, false,
                                neighborhoodsForPeers[id()], maxNeighborhoodSize
                            );
                            request.addToConsensus(message, sourceNeighborhood);
                            if (errantMessages.count(message.sequenceNum) > 0) {
                                for (const auto &m :
                                     errantMessages[message.sequenceNum]) {
                                    request.addToConsensus(
                                        m, neighborhoodsForPeers[m.senderID]
                                    );
                                }
                            }
                            errantMessages.erase(message.sequenceNum);
                            contacts.insert({message.sequenceNum, newContacts});
                            superRequests.insert(
                                {request.getSequenceNumber(), request}
                            );
                        } else {
                            ConsensusContacts newContacts =
                                chooseContactsFor(message.trans, true);
                            PBFTRequest request(
                                message.trans, maxNeighborhoodSize,
                                message.sequenceNum, false
                            );
                            request.addToConsensus(message, sourceNeighborhood);
                            if (errantMessages.count(message.sequenceNum) > 0) {
                                for (const auto &m :
                                     errantMessages[message.sequenceNum]) {
                                    request.addToConsensus(m);
                                }
                            }
                            errantMessages.erase(message.sequenceNum);
                            contacts.insert({message.sequenceNum, newContacts});
                            initRequests.insert(
                                {request.getSequenceNumber(), request}
                            );
                        }
                    }
                } else {
                    // std::cout
                    //     << "received pre-prepare message about unknown
                    //     coin\n";
                }
            } else if (message.messageType == "reply") {
                // TODO: robustness. should check if coin sender is part of
                // this. should count incoming messages by neighborhood instead
                // of just checking if there are enough
                receivedPostCommits.insert(message.sequenceNum);
                if (receivedPostCommits.count(message.sequenceNum) >
                    2. / 3. * validatorNeighborhoods * maxNeighborhoodSize) {
                    auto localReceiver = std::find_if(
                        heldWallets.begin(), heldWallets.end(),
                        [message](LocalWallet w) {
                            return w.address == message.trans.receiver.address;
                        }
                    );
                    if (coinDB.count(message.trans.coin.id) > 0) {
                        for (auto &wallet : heldWallets) {
                            wallet.remove(message.trans.coin);
                        }
                    }
                    coinDB[message.trans.coin.id] =
                        localReceiver->add(message.trans.coin);
                }
            } else {
                errantMessages[message.sequenceNum].push_back(message);
            }
        }
    }
    // update consensus on in-progress requests:
    for (auto s = localRequests.begin(); s != localRequests.end();) {
        PBFTRequest &r = s->second;
        r.updateConsensus();
        if (r.consensusSucceeded()) {
            OngoingTransaction trans = r.getTransaction();
            // local transaction; commit changes, update wallets
            auto localSender = std::find_if(
                heldWallets.begin(), heldWallets.end(),
                [trans](LocalWallet w) {
                    return w.address == trans.sender.address;
                }
            );
            auto localReceiver = std::find_if(
                heldWallets.begin(), heldWallets.end(),
                [trans](LocalWallet w) {
                    return w.address == trans.receiver.address;
                }
            );
            if (localReceiver != heldWallets.end() &&
                localSender != heldWallets.end()) {
                Coin moving = trans.coin;
                moving.history.push_back(trans);
                coinDB[moving.id] = localReceiver->add(moving);
                localSender->remove(moving);
            }
            validations.push_back(
                {{"round", getRound()},
                 {"seqNum", r.getSequenceNumber()},
                 {"peer", id()}}
            );

            contacts.erase(r.getSequenceNumber());
            s = localRequests.erase(s);
        } else {
            while (!r.outboxEmpty()) {
                TrailMessage m = r.getMessage();
                broadcastTo(m, r.getTransaction().sender.storedBy);
                localMessagesThisRound +=
                    r.getTransaction().sender.storedBy.size();
            }
            ++s;
        }
    }

    for (auto s = initRequests.begin(); s != initRequests.end();) {
        PBFTRequest &r = s->second;
        r.updateConsensus();
        if (r.consensusSucceeded()) {
            OngoingTransaction trans = r.getTransaction();
            contacts.erase(r.getSequenceNumber());
            ConsensusContacts newContacts = chooseContactsFor(trans, false);
            CrossShardPBFTRequest request(
                trans, validatorNeighborhoods, r.getSequenceNumber(), true,
                neighborhoodsForPeers[id()], maxNeighborhoodSize
            );
            contacts.insert({r.getSequenceNumber(), newContacts});
            superRequests.insert({request.getSequenceNumber(), request});
            s = initRequests.erase(s);
        } else {
            while (!r.outboxEmpty()) {
                TrailMessage m = r.getMessage();
                broadcastTo(m, r.getTransaction().sender.storedBy);
                superMessagesThisRound +=
                    r.getTransaction().sender.storedBy.size();
            }
            ++s;
        }
    }

    for (auto s = superRequests.begin(); s != superRequests.end();) {
        CrossShardPBFTRequest &r = s->second;
        r.updateConsensus();
        if (r.consensusSucceeded()) {
            int seqNum = r.getSequenceNumber();
            int peerID = id();
            OngoingTransaction trans = r.getTransaction();
            if (trans.sender.storedBy.has(id())) {
                // if we are in the sending committee:
                auto localSender = std::find_if(
                    heldWallets.begin(), heldWallets.end(),
                    [trans](LocalWallet w) {
                        return w.address == trans.sender.address;
                    }
                );
                Coin moving = trans.coin;
                moving.history.push_back(trans);
                coinDB[moving.id] = localSender->moveToHistory(moving);
                assert(coinDB[moving.id]->history.size() > 0);
            } else {
                // if we are a validator:
                if (coinDB.count(trans.coin.id) > 0) {
                    coinDB[trans.coin.id]->history.push_back(trans);
                    TrailMessage m;
                    m.trans = trans;
                    m.messageType = "reply";
                    m.sequenceNum = seqNum;
                    m.senderID = id();
                    broadcastTo(m, trans.receiver.storedBy);
                    superMessagesThisRound += trans.receiver.storedBy.size();
                } else {
                    std::cout << "reached consensus on an unknown coin?\n";
                }
            }
            validations.push_back(
                {{"round", getRound()},
                 {"seqNum", r.getSequenceNumber()},
                 {"peer", id()}}
            );
            contacts.erase(r.getSequenceNumber());
            s = superRequests.erase(s);
        } else {
            while (!r.outboxEmpty()) {
                TrailMessage m = r.getMessage();
                std::vector<Neighborhood> recipients =
                    contacts.at(r.getSequenceNumber()).participants;
                for (auto &recipient : recipients) {
                    if (!(m.messageType == "pre-prepare" &&
                          recipient.memberIDs.count(id()) > 0)) {
                        // don't send a pre-prepare message to own shard;
                        // consensus was reached on it earlier
                        broadcastTo(m, recipient);
                        superMessagesThisRound += recipient.size();
                    }
                }
            }
            ++s;
        }
    }

    if (heldWallets[0].storedBy.leader == id()) {
        if (byzantineRound != -1 && getRound() == byzantineRound + 1 &&
            !corrupt && attemptRollback) {
            initiateRollbacks();
        }

        bool submit = false;
        if (submitRate == -1) {
            const int inProgressTxs = localRequests.size() +
                                      superRequests.size() +
                                      initRequests.size();
            submit = inProgressTxs < 10;
        } else {
            submit = oneInXChance(submitRate) ||
                     (corrupt && oneInXChance(submitRate / 2));
        }
        if (submit) {
            initiateTransaction(!(corrupt || oneInXChance(4)));
        }
    }
}

void TrailPeer::broadcastTo(TrailMessage m, Neighborhood n) {
    // if (m.sequenceNum == x) {
    //     std::cout << "sending message " << m.messageID
    //               << " about sequence number 148:\n";
    //     std::cout << "Type: " << m.messageType << "\n";
    //     std::cout << "Sender: " << id() << "\n";
    //     std::cout << "Receiver: neighborhood " << n.id << ", led by "
    //               << n.leader << "\n\n";
    // }
    for (const long &i : n.memberIDs) {
        unicastTo(m, i);
    }
}

void TrailPeer::endOfRound(const vector<Peer<TrailMessage> *> &_peers) {
    messages.push_back(
        {{"round", getRound()},
         {"transactionType", "local"},
         {"batchSize", localMessagesThisRound}}
    );
    localMessagesThisRound = 0;
    messages.push_back(
        {{"round", getRound()},
         {"transactionType", "non-local"},
         {"batchSize", superMessagesThisRound}}
    );
    superMessagesThisRound = 0;
    const vector<TrailPeer *> peers =
        reinterpret_cast<vector<TrailPeer *> const &>(_peers);
    // for (const auto &peer : peers) {
    //     if (peer->initRequests.size() + peer->localRequests.size() +
    //             peer->superRequests.size() >=
    //         10) {
    //         std::cout << "peer " << peer->id()
    //                   << " is full of in-progress transactions" <<
    //                   std::endl;
    //     }
    // }
    if (lastRound()) {
        LogWriter::getTestLog()["transactions"] = json::array();
        LogWriter::getTestLog()["validations"] = json::array();
        LogWriter::getTestLog()["messages"] = json::array();
        for (const auto &peer : peers) {
            std::copy(
                peer->transactions.begin(), peer->transactions.end(),
                std::back_inserter(LogWriter::getTestLog()["transactions"])
            );
            std::copy(
                peer->validations.begin(), peer->validations.end(),
                std::back_inserter(LogWriter::getTestLog()["validations"])
            );
            std::copy(
                peer->messages.begin(), peer->messages.end(),
                std::back_inserter(LogWriter::getTestLog()["messages"])
            );
            for (const auto &t : peer->superRequests) {
                const int age =
                    getRound() - t.second.getTransaction().roundSubmitted;
            }
        }
    } else if (byzantineRound == getRound()) {
        std::vector<int> shuffledNBHs(neighborhoods.size());
        std::iota(shuffledNBHs.begin(), shuffledNBHs.end(), 0);
        std::shuffle(
            shuffledNBHs.begin(), shuffledNBHs.end(), RANDOM_GENERATOR
        );

        for (int indexIntoShuffle = 0;
             indexIntoShuffle < maliciousNeighborhoods; ++indexIntoShuffle) {
            int corruptNeighborhood = shuffledNBHs[indexIntoShuffle];
            corruptNeighborhoods.insert(corruptNeighborhood);
            for (int i = 0;
                 i < walletsForNeighborhoods[corruptNeighborhood].size(); i++) {
                LogWriter::getTestLog()["corruptWallets"].push_back(
                    walletsForNeighborhoods[corruptNeighborhood][i].address
                );
            }
            for (int i = 0; i < peers.size(); i++) {
                if (peers[i]->neighborhoodID == corruptNeighborhood) {
                    std::cout << "corrupting " << peers[i]->id() << std::endl;
                    peers[i]->corrupt = true;
                    if (walletsForNeighborhoods[corruptNeighborhood][0]
                            .storedBy.leader == peers[i]->id()) {
                        // using the leader of the byzantine neighborhoods
                        // as a reference for what coins they hold
                        for (const auto &w : peers[i]->heldWallets) {
                            for (const auto &[cid, c] : w.coins) {
                                LogWriter::getTestLog()["lostCoins"].push_back(
                                    cid
                                );
                            }
                        }
                    }
                }
            }
        }
    }
    std::cout << "completed round " << (getRound() + 1) << "/"
              << (getLastRound() + 1) << "\n";
}

bool TrailPeer::transactionInProgressFor(Coin c) {
    for (auto &r : localRequests) {
        if (r.second.getTransaction().coin == c) {
            return true;
        }
    }
    for (auto &r : superRequests) {
        if (r.second.getTransaction().coin == c) {
            return true;
        }
    }
    for (auto &r : initRequests) {
        if (r.second.getTransaction().coin == c) {
            return true;
        }
    }
    return false;
}

ConsensusContacts
TrailPeer::chooseContactsFor(OngoingTransaction t, bool local) {
    ConsensusContacts result;
    if (local) {
        result = ConsensusContacts(t.coin, 1);
    } else if (!corrupt && corruptNeighborhoods.count(t.sender.storedBy.id) > 0) {
        // assuming a rollback transaction. corrupt senders should be
        // stopped by legit validation in message retrieval in
        // performComputation
        result = ConsensusContacts(t.coin, validatorNeighborhoods, true);
    } else {
        result = ConsensusContacts(t.coin, validatorNeighborhoods);
    }
    if (corrupt) {
        result.sabotageWithOwn(neighborhoods[neighborhoodID]);
    }
    return result;
}

void TrailPeer::initiateRollbacks() {
    std::unordered_set<int> rollingBack;
    for (const auto &w : heldWallets) {
        for (const auto &[cid, c] : w.pastCoins) {
            const auto lastTransaction = c.history.back();
            if (lastTransaction.sender.storedBy.leader == id()) {
                const auto inWallet = lastTransaction.receiver;
                if (corruptNeighborhoods.count(inWallet.storedBy.id) > 0) {
                    if (rollingBack.count(cid) > 0) {
                        std::cerr << "already rolling that one back!"
                                  << std::endl;
                    }
                    rollingBack.insert(cid);
                    auto sender = inWallet;
                    auto receiver = lastTransaction.sender;
                    OngoingTransaction t = {
                        inWallet, lastTransaction.sender, c, getRound(), -1};
                    int seqNum = ++previousSequenceNumber;
                    ConsensusContacts newContacts(
                        c, validatorNeighborhoods, true
                    );
                    PBFTRequest request(
                        t, validatorNeighborhoods, seqNum, true,
                        neighborhoodsForPeers[id()]
                    );
                    initRequests.insert({seqNum, request});
                    contacts.insert({seqNum, newContacts});
                    transactions.push_back(
                        {{"round", getRound()},
                         {"seqNum", seqNum},
                         {"validatorsNeeded",
                          newContacts.getValidatorNodeCount()},
                         {"coin", c.id},
                         {"sender", sender.address},
                         {"honest", true},
                         {"receiver", receiver.address},
                         {"rollback", true}}
                    );
                }
            }
        }
    }
}

// Precondition: peer is a leader in their (single) neighborhood
// Postcondition: PBFTRequest created UNLESS there are no coins to send in
// this neighborhood's wallets
void TrailPeer::initiateTransaction(bool withinNeighborhood) {
    LocalWallet sender;
    Coin c;
    bool honest;

    if (corrupt) {
        vector<LocalWallet> haveCoins;
        std::copy_if(
            heldWallets.begin(), heldWallets.end(),
            std::back_inserter(haveCoins),
            [](const LocalWallet &w) { return w.pastCoins.size() > 0; }
        );
        if (haveCoins.size() != 0) {
            sender = haveCoins[randMod(haveCoins.size())];
            // hacky way to get random element from unordered map
            auto pcIter = std::next(
                sender.pastCoins.begin(), randMod(sender.pastCoins.size())
            );
            c = pcIter->second;
        } else {
            std::cout
                << "unable to find past coin for corrupt transaction :(\n";
        }
        honest = false;
    }
    if (c.id == -1) {
        vector<LocalWallet> haveCoins;
        std::copy_if(
            heldWallets.begin(), heldWallets.end(),
            std::back_inserter(haveCoins),
            [this](const LocalWallet &w) {
                for (const auto &c : w.coins) {
                    if (!transactionInProgressFor(c.second)) {
                        return true;
                    }
                }
                return false;
            }
        );
        if (haveCoins.size() == 0) {
            return;
        }
        sender = haveCoins[randMod(haveCoins.size())];
        // hacky way to get random element from unordered map
        vector<Coin> availableCoins;
        for (const auto &c : sender.coins) {
            if (!transactionInProgressFor(c.second)) {
                availableCoins.push_back(c.second);
            }
        }
        c = availableCoins[randMod(availableCoins.size())];
        honest = true;
    }

    LocalWallet receiver = sender;
    // obtain a receiver that is not the sender
    if (withinNeighborhood) {
        do {
            receiver = heldWallets[randMod(heldWallets.size())];
        } while (receiver.address == sender.address);
    } else {
        // accessing static data to find receiver wallet in other
        // neighborhood - for real transactions, clients would have to tell
        // us about these
        int receiverNeighborhood = neighborhoodID;
        do {
            receiverNeighborhood = randMod(neighborhoodCount);
        } while (receiverNeighborhood == neighborhoodID);
        // i'm assuming that all neighborhoods store at least one wallet
        std::vector<LocalWallet> potential =
            walletsForNeighborhoods[receiverNeighborhood];
        receiver = potential[randMod(potential.size())];
    }
    OngoingTransaction t = {sender, receiver, c, getRound(), -1};
    int seqNum = ++previousSequenceNumber;

    ConsensusContacts newContacts(c, 1);
    PBFTRequest request(t, newContacts.getValidatorNodeCount(), seqNum, true);
    if (withinNeighborhood) {
        localRequests.insert({seqNum, request});
    } else {
        initRequests.insert({seqNum, request});
    }

    transactions.push_back(
        {{"round", getRound()},
         {"seqNum", seqNum},
         {"validatorsNeeded",
          (withinNeighborhood
               ? newContacts.getValidatorNodeCount()
               : chooseContactsFor(t, false).getValidatorNodeCount())},
         {"coin", c.id},
         {"sender", sender.address},
         {"honest", honest},
         {"receiver", receiver.address},
         {"rollback", false}}
    );
}
} // namespace quantas
