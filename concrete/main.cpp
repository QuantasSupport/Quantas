#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include "../quantas/Common/Json.hpp"
#include "../quantas/Common/LogWriter.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <set>
#include <mutex>
#include <condition_variable>

using json = nlohmann::json;
using quantas::LogWriter;


std::set<int> received_peer_ids;
std::mutex mtx;
std::condition_variable cv;

struct PeerConfig {
    int id;
    int port;
    std::string ip;
};

std::vector<PeerConfig> read_config(const std::string& path) {
    std::ifstream in(path);
    json j;
    in >> j;

    std::vector<PeerConfig> peers;
    for (const auto& item : j["config"]) {
        peers.push_back({
            item["id"],
            item["port"],
            item["ip"]
        });
    }
    return peers;
}

void start_listener(int port, int my_id, int total_peers) {

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 8);

    LogWriter::pushValue("info", "Listening on port " + std::to_string(port));

    while (true) {
        sockaddr_in client_addr{};
        socklen_t addrlen = sizeof(client_addr);
        int new_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);

        char buffer[1024] = {0};
        int bytes = read(new_socket, buffer, 1024);
        std::string raw_msg(buffer, bytes);

        LogWriter::pushValue("info", "[RECV RAW] " + raw_msg);

        try {
            json msg_json = json::parse(raw_msg);
            int sender_id = msg_json.value("from", -1);

            if (sender_id != -1 && sender_id != my_id) {
                std::lock_guard<std::mutex> lock(mtx);
                if (received_peer_ids.insert(sender_id).second) {
                    LogWriter::pushValue("info", "[RECV JSON] from peer " + std::to_string(sender_id));
                    cv.notify_all();
                }
            }
        } catch (const std::exception& e) {
            LogWriter::pushValue("warn", "Failed to parse JSON: " + std::string(e.what()));
        }

        close(new_socket);

        if (received_peer_ids.size() >= total_peers - 1) {
            break;
        }
    }

    LogWriter::pushValue("info", "All expected messages received. Exiting.");
    close(server_fd);
}

void send_message(const PeerConfig& peer, int my_id) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(peer.port);
    inet_pton(AF_INET, peer.ip.c_str(), &serv_addr.sin_addr);

    while (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        LogWriter::pushValue("warn", "Could not connect to " + peer.ip + ":" + std::to_string(peer.port) + " retrying...");
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    json msg_json = {
        {"from", my_id},
        {"type", "greeting"}
    };

    std::string msg = msg_json.dump();
    send(sock, msg.c_str(), msg.size(), 0);
    LogWriter::pushValue("info", "[SEND] " + msg + " to peer " + std::to_string(peer.id));
    close(sock);
}


int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: ./peer <config.json> <id>\n";
        return 1;
    }

    std::string config_path = argv[1];
    int my_id = std::stoi(argv[2]);

    std::string log_path = "peer_" + std::to_string(my_id) + "_log.txt";

    LogWriter::setLogFile(log_path);
    LogWriter::setTest(0);

    auto peers = read_config(config_path);

    // Find my config
    PeerConfig* my_config = nullptr;
    for (auto& p : peers) {
        if (p.id == my_id) {
            my_config = &p;
            break;
        }
    }

    if (!my_config) {
        LogWriter::pushValue("info","ID " + std::to_string(my_id) + " not found in config");
        return 1;
    }

    std::thread listener([&] {
        start_listener(my_config->port, my_id, peers.size());
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));

    for (const auto& peer : peers) {
        if (peer.id != my_id) {
            send_message(peer, my_id);
        }
    }

    // Wait until all expected messages are received
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&] { return received_peer_ids.size() >= peers.size() - 1; });
    }

    LogWriter::pushValue("info", "[DONE] Received messages from all peers. Exiting.");
    listener.join();
    LogWriter::print();
    
    return 0;
}