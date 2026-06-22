#include "BrachaPeer.hpp"
#include "../Common/Faults/EquivocateFault.hpp"

namespace quantas
{

    static bool registerBrachaPeer = []()
    {
        return PeerRegistry::registerPeerType(
            "BrachaPeer",
            [](interfaceId pubId)
            { return new BrachaPeer(new NetworkInterfaceAbstract(pubId)); });
    }();

    BrachaPeer::BrachaPeer(NetworkInterface *interfacePtr)
        : ConsensusPeer(interfacePtr) {}

    BrachaPeer::BrachaPeer(const BrachaPeer &rhs) : ConsensusPeer(rhs), num_peers(rhs.num_peers), num_faulty(rhs.num_faulty), messages_sent(rhs.messages_sent), echo_messages(rhs.echo_messages), ready_messages(rhs.ready_messages), dilivered_messages(rhs.dilivered_messages) {}

    BrachaPeer::~BrachaPeer() {}

    void BrachaPeer::initParameters(const std::vector<Peer *> &_peers, json parameters)
    {
        const std::vector<BrachaPeer *> peers = reinterpret_cast<std::vector<BrachaPeer *> const &>(_peers);

        for (auto peer : peers)
        {
            peer->num_peers = peers.size();
            peer->num_faulty = parameters.value("numFaulty", 0);
            peer->num_tx = parameters.value("numMessages", 1);
        }

        std::set<interfaceId> members;
        for (auto p : peers)
        {
            members.insert(p->publicId());
        }

        std::set<std::string> types = {"echo", "ready", "send"};
        std::set<interfaceId> A, B;
        bool flip = true;
        for (auto id : members)
        {
            (flip ? A : B).insert(id);
            flip = !flip;
        }
        // setting i=0 makes the first node a faulty node but BRB doesn't support that as well
        for (int i = 1; i <= num_faulty; ++i)
        {
            peers[i]->faultManager.addFault(new EquivocateFault(A, B, types));
        }
    }

    void BrachaPeer::endOfRound(std::vector<Peer *> &_peers)
    {
        if (RoundManager::currentRound() >= RoundManager::lastRound())
        {
            const std::vector<BrachaPeer *> peers = reinterpret_cast<std::vector<BrachaPeer *> const &>(_peers);
            int total_msg = 0;
            for (auto peer : peers)
            {
                total_msg += peer->messages_sent;
            }
            OutputWriter::pushValue("total_messages_sent", total_msg);
            // pushing latency/ total dilivered messages
            int total_delivered  = 0;
            int total_echo_sent  = 0;
            int total_ready_sent = 0;
            double total_latency_sum = 0.0;

            for (auto peer : peers)
            {
                total_delivered  += peer->delivered_count;
                total_echo_sent  += peer->echo_sent;
                total_ready_sent += peer->ready_sent;
                total_latency_sum += peer->total_latency;
            }
            OutputWriter::pushValue("total_delivered",  total_delivered);
            //OutputWriter::pushValue("total_echo_sent",  total_echo_sent);
            //OutputWriter::pushValue("total_ready_sent", total_ready_sent);
            OutputWriter::pushValue("total_latency",    total_latency_sum);
        }
    }

    void BrachaPeer::broadcast_send(const std::string &message)
    {
        json msg =
            {
                {"MessageType", "send"},
                {"fromId", publicId()},
                {"submitterId", publicId()},
                {"roundSubmitted", RoundManager::currentRound()},
                {"proposal", {{"Request", {{"fault_flip", false}}}}},
                {"content", message}};

        messages_sent += neighbors().size();
        multicast(msg, neighbors());
    }

    void BrachaPeer::broadcast_echo(const json &message)
    {
        json echo_msg = message;
        echo_msg["MessageType"] = "echo";
        echo_msg["fromId"] = publicId();

        messages_sent += neighbors().size();
        ++echo_sent;
        // OutputWriter::pushValue("echo_sent_by_" + std::to_string(publicId()), (int)RoundManager::currentRound());
        multicast(echo_msg, neighbors());
    }

    void BrachaPeer::broadcast_ready(const json &message)
    {
        json ready_msg = message;
        ready_msg["MessageType"] = "ready";
        ready_msg["fromId"] = publicId();

        messages_sent += neighbors().size();
        ++ready_sent;
        // OutputWriter::pushValue("ready_sent_by_" + std::to_string(publicId()), (int)RoundManager::currentRound());
        multicast(ready_msg, neighbors());
    }

    void BrachaPeer::performComputation()
    {
        if (publicId() == 0 && tx_id < num_tx)
            {
                if (dilivered_messages.count(
                        std::to_string(publicId()) + "_" + std::to_string(tx_id)) > 0 || tx_id == 0)
                {
                    std::string content = std::to_string(publicId()) + "_" + std::to_string(++tx_id);
                    OutputWriter::pushValue("sent_" + content, (int)RoundManager::currentRound());
                    broadcast_send(content);
                    echo_messages[content].insert(publicId());
                    json broadcaster_echo_msg =
                    {
                        {"MessageType", "echo"},
                        {"fromId", publicId()},
                        {"submitterId", publicId()},
                        {"roundSubmitted", RoundManager::currentRound()},
                        {"proposal", {{"Request", {{"fault_flip", false}}}}},
                        {"content", content}};
                    broadcast_echo(broadcaster_echo_msg);
                }
            }

        while (!inStreamEmpty())
        {
            Packet packet = popInStream();
            json msg = packet.getMessage();
            // std::cout<<"Received message: "<<msg.dump()<<std::endl;
            string round_str = std::to_string(RoundManager::currentRound());
            if (RoundManager::currentRound() <10) {
                round_str = "0" + round_str;
            }
            OutputWriter::pushValue("received peer " + std::to_string(publicId()) + std::string(" round ") + round_str, msg.dump());
            // skip faulty nodes
            if (msg["proposal"]["Request"]["fault_flip"].get<bool>() == true)
            {
                continue;
            }

            std::string msgContent = msg["content"];
            std::string msgType = msg["MessageType"];

            if (msgType == "echo")
            {
                echo_messages[msgContent].insert(packet.sourceId());
            }
            else if (msgType == "ready")
            {
                ready_messages[msgContent].insert(packet.sourceId());
            }

            int n = num_peers;
            int t = (n-1) / 3; //
            bool echo_condition = echo_messages[msgContent].size() >= (((n + t) / 2) + 1);
            bool ready_condition = ready_messages[msgContent].size() >= t + 1;

            // ECHO
            if ((msgType == "send" || echo_condition || ready_condition) &&
                echo_messages[msgContent].count(publicId()) == 0)
            {
                echo_messages[msgContent].insert(publicId());
                broadcast_echo(msg);
            }

            // READY
            if ((echo_condition || ready_condition) &&
                ready_messages[msgContent].count(publicId()) == 0)
            {
                ready_messages[msgContent].insert(publicId());
                broadcast_ready(msg);
            }

            // 2t+1 readys
            if ((int)ready_messages[msgContent].size() >= 2 * t + 1 &&
                dilivered_messages.count(msgContent) == 0)
            {
                dilivered_messages.insert(msgContent);
                total_latency += (int)RoundManager::currentRound() - msg["roundSubmitted"].get<int>();
                ++delivered_count;
                OutputWriter::pushValue("delivered_" + msgContent + " peer " + std::to_string(publicId()), (int)RoundManager::currentRound());
            }
        }
    }
}