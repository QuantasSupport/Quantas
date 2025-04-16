#ifndef NETWORK_INTERFACE_CONCRETE_HPP
#define NETWORK_INTERFACE_CONCRETE_HPP

#include <memory>
#include <map>
#include <set>
#include <deque>
#include <string>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <fstream>
#include "../LogWriter.hpp"
#include "../Packet.hpp"
#include "../NetworkInterface.hpp"
#include "ipUtil.hpp"
#include "../Json.hpp"
#include "../BS_thread_pool.hpp"

namespace quantas {

class NeighborInfo {
public:
    NeighborInfo() {};
    NeighborInfo(interfaceId Nid, std::string Nip, int Nport) : id(Nid), ip(Nip), port(Nport) {};
    interfaceId id{NO_PEER_ID};
    std::string ip;
    int port{-1};
    json jsonify() const {
        return {
            {"id", id},
            {"ip", ip},
            {"port", port}
        };
    }
};

class NetworkInterfaceConcrete : public NetworkInterface {
private:

    int my_port = -1;
    int total_peers = -1;
    bool is_leader = false;
    std::atomic<bool> shutdown_condition = false;
    int server_fd = -1;
    std::string my_ip;
    json config;

    std::mutex concrete_mtx;
    std::thread listener; // the thread for receiving messages
    BS::thread_pool pool{1}; // currently only 1 thread is allowed to exist for sending messages

    std::map<interfaceId, NeighborInfo> all_peers;
    // Use a thread pool or message dispatch queue so you don’t spawn hundreds of threads.
    void send_json(const std::string& ip, int port, const json& jmsg, bool async = true) {
        auto task = [=]() {
            if (shutdown_condition) return;
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

            #ifdef __APPLE__
                // macOS: disable SIGPIPE per-socket
                int set = 1;
                setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &set, sizeof(set));
            #endif
    
            while (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0 && !shutdown_condition) {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                // std::cerr << "send to " << ip << " " << port  << " failed, retrying." << std::endl;
            }
    
            std::string msg = jmsg.dump();
            send(sock, msg.c_str(), static_cast<int>(msg.size()), 0);
    
            #ifdef _WIN32
                closesocket(sock);
            #else
                close(sock);
            #endif
        };
    
        if (async) {
            pool.push_task(task);
        } else {
            task();
        }
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
    }

    void wait_for_peer_list() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            std::lock_guard<std::mutex> lock(concrete_mtx);
            if (all_peers.size() == total_peers) break;
        }
    }

    void start_listener() {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);

        if (server_fd < 0) {
            perror("socket creation failed");
            return;
        }
        int opt = 1;

        #ifdef _WIN32
            // On Windows, SO_REUSEADDR allows rebinding if no active listener exists.
            // SO_EXCLUSIVEADDRUSE ensures only one process can bind (strong safety)
            setsockopt(server_fd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char*)&opt, sizeof(opt));
        #else
            // On Linux/macOS, SO_REUSEADDR allows rebinding during TIME_WAIT
            setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        #endif

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(my_port);

        bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
        listen(server_fd, 8);

        // std::cout << _publicId << " listening on ip " << my_ip << " port " << my_port << std::endl;

        while (!shutdown_condition) {
            sockaddr_in client_addr{};
            socklen_t addrlen = sizeof(client_addr);
            int new_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
            if (shutdown_condition) break;  // clean exit
            if (new_socket < 0) {
                perror("accept failed");
                #ifdef _WIN32
                    closesocket(new_socket);
                #else
                    close(new_socket);
                #endif
                continue;  // skip this loop iteration
            }

            char buffer[2048] = {0};

            #ifdef _WIN32
                int bytes = recv(new_socket, buffer, sizeof(buffer), 0);
            #else
                int bytes = read(new_socket, buffer, sizeof(buffer));
            #endif

            if (bytes <= 0) {
                if (bytes == 0) {
                    std::cerr << "[RECV] Connection closed by peer.\n";
                } else {
                    perror("read/recv failed");
                }
                #ifdef _WIN32
                    closesocket(new_socket);
                #else
                    close(new_socket);
                #endif
                continue;
            }
            std::string raw_msg(buffer, bytes);

            try {
                json msg = json::parse(raw_msg);
                std::string type = msg.value("type", "");                

                if (type == "ip_report" && is_leader) {
                    std::string assigned_ip = msg["from_ip"];
                    int port = msg["from_port"];
                    std::lock_guard<std::mutex> lock(concrete_mtx);
                    interfaceId assigned_id = all_peers.size() + 1;
                    all_peers.insert({assigned_id, {assigned_id, assigned_ip, port}});

                    if (all_peers.size() == total_peers - 1) {
                        all_peers.insert({_publicId, {_publicId, my_ip, my_port}});
                        json all_peers_json;
                        for (const auto& peer : all_peers) {
                            all_peers_json["peers"].push_back(peer.second.jsonify());
                        }

                        json newMsg = {
                            {"type", "ip_list"},
                            {"from_id", _publicId},
                            {"peers", all_peers_json["peers"]}
                        };

                        for (const auto& peer : all_peers) {
                            if (peer.first == _publicId) continue;
                            addNeighbor(peer.first);
                            send_json(peer.second.ip, peer.second.port, newMsg);
                        }
                    }
                } else if (type == "ip_list" && !is_leader) {
                    std::lock_guard<std::mutex> lock(concrete_mtx);
                    for (const auto& item : msg["peers"]) {
                        NeighborInfo newNeighbor(item["id"], item["ip"], item["port"]);
                        all_peers.insert({item["id"], newNeighbor});
                        if (item["ip"] == my_ip && item["port"] == my_port) {
                            _publicId = item["id"];
                        } else {
                            addNeighbor(item["id"]);
                        }
                    }
                } else if (type == "message") {
                    std::lock_guard<std::mutex> lock(_inStream_mtx);
                    interfaceId sender = msg.value("from_id", -1);
                    if (sender == -1) continue;
                    Packet arrivedPkt(_publicId, sender, msg["body"]);
                    _inStream.push_back(std::move(arrivedPkt));
                    // std::cout << "Message of type | " << type << " | " << std::endl;
                }
            } catch (...) {}
            #ifdef _WIN32
                closesocket(new_socket);
            #else
                close(new_socket);
            #endif
        }

        // std::cout << _publicId << " stopped listening on ip " << my_ip << " port " << my_port << std::endl;

        #ifdef _WIN32
            closesocket(server_fd);
        #else
            close(server_fd);
        #endif
    }

    void start() {
        #ifdef _WIN32
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2,2), &wsaData);
        #endif

        install_socket_safety();

        listener = std::thread(&NetworkInterfaceConcrete::start_listener, this);
        
        if (!is_leader) {
            send_ip_report();
            wait_for_peer_list();
        } else {
            wait_for_peer_list();
        }

        LogWriter::setLogFile("peer_" + std::to_string(_publicId) + ".log");
        LogWriter::setTest(0);
    }

public:

    inline NetworkInterfaceConcrete() {};
    inline NetworkInterfaceConcrete(interfaceId pubId) : NetworkInterface(pubId, pubId) {};
    inline NetworkInterfaceConcrete(interfaceId pubId, interfaceId internalId) : NetworkInterface(pubId, internalId) {};
    inline ~NetworkInterfaceConcrete() {};

    bool getShutdownCondition() const {
        return shutdown_condition;
    }

    void shutDown() {
        shutdown_condition = true;
        if (server_fd != -1) {
            #ifdef _WIN32
                closesocket(server_fd);
            #else
                close(server_fd);
            #endif

            // Wake up accept() with a dummy connection
            int dummy = socket(AF_INET, SOCK_STREAM, 0);
            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(my_port);
            inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);  // connect to self

            connect(dummy, (sockaddr*)&addr, sizeof(addr));
            #ifdef _WIN32
                closesocket(dummy);
            #else
                close(dummy);
            #endif
        }
    }

    void load_config(std::string config_path, int port = -1) {
        my_ip = get_local_ip();
        std::ifstream in(config_path);
        in >> config;
        total_peers = config["total_peers"];
        if (port == -1) {
            my_port = get_unused_port();
        } else if (my_ip == config["leader"]["ip"] && port == config["leader"]["port"]) {
            my_port = port;
            is_leader = true;
            _publicId = config["leader"]["id"];
        } else {
            my_port = port;
        }

        if (!can_bind_port(my_port)) {
            throw std::runtime_error("Failed to bind to port " + std::to_string(my_port));
        }

        in.close();
        start();
    }

    // Send messages to to others using this
    inline void unicastTo (json msg, const interfaceId& dest) override;
    
    // no need to move from channels to instream
    // moves msgs from the channel to the inStream if they've arrived
    inline void receive() override {};

    inline void clearAll() override {
        std::cout << "ClearAll" << std::endl;
        #ifdef _WIN32
            WSACleanup();
        #endif
        listener.join();
        std::cout << "join" << std::endl;
        std::size_t task_count = pool.get_tasks_total();
        std::cout << "Total active or queued tasks: " << task_count << std::endl;
        pool.wait_for_tasks();
        std::cout << "wait_for_tasks" << std::endl;
        _inStream.clear();
        std::cout << "_inStream" << std::endl;
        _neighbors.clear();
        std::cout << "_neighbors" << std::endl;
        all_peers.clear();
        std::cout << "all_peers" << std::endl;
    }
};

void NetworkInterfaceConcrete::unicastTo(json msg, const interfaceId& nbr) {
    if (_neighbors.find(nbr) == _neighbors.end()) return;
    json newMsg = {
        {"type", "message"},
        {"from_id", _publicId},
        {"body", msg}
    };
    send_json(all_peers[nbr].ip, all_peers[nbr].port, newMsg);
}

}

#endif /* NETWORK_INTERFACE_CONCRETE_HPP */