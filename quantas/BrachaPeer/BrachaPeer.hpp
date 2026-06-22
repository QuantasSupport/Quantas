#ifndef BRACHAPEER_HPP
#define BRACHAPEER_HPP

#include "../Common/OutputWriter.hpp"
#include "../Common/RoundManager.hpp"
#include "../Common/Abstract/NetworkInterfaceAbstract.hpp"
#include "../Common/ConsensusPeer.hpp"

#include <unordered_map>
#include <unordered_set>
#include <string>

namespace quantas
{
    class BrachaPeer : public ConsensusPeer
    {
    public:
        BrachaPeer(NetworkInterface*);
        BrachaPeer(const BrachaPeer &rhs);
        ~BrachaPeer() override;

        void performComputation() override; // core per-round logic
        void endOfRound(std::vector<Peer*> &peers) override;
        void initParameters(const std::vector<Peer *> &peers, json parameters) override;

        // void send_message(interfaceId peer, json message);
        void broadcast_send(const std::string &message);
        void broadcast_echo(const json &message);
        void broadcast_ready(const json &message);

        int num_peers, num_faulty;
        int messages_sent = 0;
        int tx_id = 0;
        int num_tx = 1;

        std::map<std::string, std::set<int>> echo_messages;
        std::map<std::string, std::set<int>> ready_messages;
        std::set<std::string> dilivered_messages;

        int echo_sent = 0;
        int ready_sent = 0;
        double total_latency = 0.0;
        int delivered_count = 0; 

    };
} // namespace quantas
#endif