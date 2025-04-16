#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <condition_variable>

#include "../quantas/Common/Concrete/ipUtil.hpp"
#include "../quantas/Common/Json.hpp"
#include "../quantas/Common/LogWriter.hpp"

using json = nlohmann::json;
using quantas::LogWriter;

class PeerInfo {
public:
    int id;
    std::string ip;
    int port;
    json jsonify() const {
        return {
            {"id", id},
            {"ip", ip},
            {"port", port}
        };
    }
};

class PeerNode {
public:
    PeerNode(const std::string& config_path, int port = -1)
        : my_port(port) {
        load_config(config_path);
    }

    void start() {
        #ifdef _WIN32
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2,2), &wsaData);
        #endif

        listener = std::thread(&PeerNode::start_listener, this);
        listener.detach();
        // std::this_thread::sleep_for(std::chrono::seconds(2));

        my_ip = get_local_ip();

        if (!is_leader) {
            send_ip_report();
            wait_for_peer_list();
        } else {
            wait_for_peer_list();
        }

        LogWriter::setLogFile("peer_" + std::to_string(my_id) + ".log");
        LogWriter::setTest(0);
    }


    void run() {
        for (const auto& peer : all_peers) {
            if (peer.id != my_id) {
                send_json(peer.ip, peer.port, {
                    {"type", "hello"},
                    {"from_id", my_id}
                });
                LogWriter::pushValue("info", "[SEND] " + std::to_string(my_id) + " sent hello to " + std::to_string(peer.id));
            }
        }
        
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return shutdown_condition; });

        LogWriter::pushValue("info", "[DONE] Exiting.");
        LogWriter::print();

        #ifdef _WIN32
            WSACleanup();
        #endif
    }

private:
    int my_id = -1;
    int my_port = -1;
    int total_peers = -1;
    bool is_leader = false;
    bool shutdown_condition = false;
    std::string my_ip;
    json config;

    std::mutex mtx;
    std::condition_variable cv;
    std::thread listener;

    std::vector<PeerInfo> all_peers;
    std::set<int> received_hello_from;
    std::set<int> received_ids;

    void load_config(std::string config_path) {
        std::ifstream in(config_path);
        in >> config;
        if (!in) {
            throw std::runtime_error("Failed to open config: " + config_path);
        }
        total_peers = config["total_peers"];
        if (my_port == -1) {
            my_port = config["me"]["port"];
        }
        if (config["leader"]["port"] == my_port) {
            is_leader = true;
            my_id = config["leader"]["id"];
        }
        in.close();
    }

    void send_json(const std::string& ip, int port, const json& jmsg) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

        while (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            LogWriter::pushValue("warn", "Retrying connection to " + ip + ":" + std::to_string(port));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::string msg = jmsg.dump();
        send(sock, msg.c_str(), static_cast<int>(msg.size()), 0);

        #ifdef _WIN32
            closesocket(sock);
        #else
            close(sock);
        #endif
    }

    void send_ip_report() {
        json msg = {
            {"type", "ip_report"},
            {"from_ip", my_ip},
            {"from_id", -1},
            {"from_port", my_port},
            {"to_ids", {config["leader"]["id"]}},
            {"id_count", 1}
        };
        send_json(config["leader"]["ip"], config["leader"]["port"], msg);
        LogWriter::pushValue("info", "[SEND] " + std::to_string(my_id) + " sent report to: " + std::to_string(int(config["leader"]["id"])));
    }

    void wait_for_peer_list() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::lock_guard<std::mutex> lock(mtx);
            if (all_peers.size() == total_peers) break;
        }
    }

    void start_listener() {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(my_port);

        bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
        listen(server_fd, 8);

        while (!shutdown_condition) {
            sockaddr_in client_addr{};
            socklen_t addrlen = sizeof(client_addr);
            int new_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);

            char buffer[2048] = {0};

            #ifdef _WIN32
                int bytes = recv(new_socket, buffer, sizeof(buffer), 0);
            #else
                int bytes = read(new_socket, buffer, 2048);
            #endif

            std::string raw_msg(buffer, bytes);

            try {
                json msg = json::parse(raw_msg);
                std::string type = msg.value("type", "");

                if (type == "ip_report" && is_leader) {
                    std::string ip = msg["from_ip"];
                    int port = msg["from_port"];
                    std::lock_guard<std::mutex> lock(mtx);
                    int assigned_id = received_ids.size() + 1;
                    received_ids.insert(assigned_id);
                    all_peers.push_back({assigned_id, ip, port});
                    LogWriter::pushValue("info", "[REC] " + std::to_string(my_id) + " received report from " + std::to_string(assigned_id));

                    if (all_peers.size() == total_peers - 1) {
                        all_peers.insert(all_peers.begin(), {0, my_ip, my_port});
                        json all_peers_json;
                        for (const auto& peer : all_peers) {
                            all_peers_json["peers"].push_back(peer.jsonify());
                        }

                        json newMsg = {
                            {"type", "ip_list"},
                            {"from_id", my_id},
                            {"peers", all_peers_json["peers"]}
                        };

                        for (const auto& peer : all_peers) {
                            if (peer.id == my_id) continue;
                            send_json(peer.ip, peer.port, newMsg);
                            LogWriter::pushValue("info", "[SEND] " + std::to_string(my_id) + " sent full report to " + std::to_string(peer.id));
                        }
                    }
                } else if (type == "ip_list" && !is_leader) {
                    std::lock_guard<std::mutex> lock(mtx);
                    for (const auto& item : msg["peers"]) {
                        all_peers.push_back({item["id"], item["ip"], item["port"]});
                        if (item["ip"] == my_ip && item["port"] == my_port) {
                            my_id = item["id"];
                        }
                    }
                    LogWriter::pushValue("info", "[REC] " + std::to_string(my_id) + " received full report");
                } else if (type == "hello") {
                    std::lock_guard<std::mutex> lock(mtx);
                    int sender = msg.value("from_id", -1);
                    if (sender != -1) {
                        received_hello_from.insert(sender);
                        LogWriter::pushValue("info", "[REC] " + std::to_string(my_id) + " received hello from " + std::to_string(sender));
                        if (received_hello_from.size() == total_peers - 1) {
                            shutdown_condition = true;
                            cv.notify_all();
                        }
                    }
                }
            } catch (...) {}

            #ifdef _WIN32
                closesocket(new_socket);
            #else
                close(new_socket);
            #endif
        }

        #ifdef _WIN32
            closesocket(server_fd);
        #else
            close(server_fd);
        #endif
    }
};


int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: ./peer <config.json> [port]\n";
            return 1;
        }
        if (argc == 2) {
            PeerNode node(argv[1]);
            node.start();
            node.run();
        } else {
            PeerNode node(argv[1], std::stoi(argv[2]));
            node.start();
            node.run();
        }
    } catch (const std::exception& e) {
        std::cerr << "[FATAL] Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
