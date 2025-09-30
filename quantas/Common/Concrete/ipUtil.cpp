#include "ipUtil.hpp"

std::string get_local_ip(bool use_ipv6) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) return "";

    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    ULONG bufLen = 15000;
    IP_ADAPTER_ADDRESSES* addresses = (IP_ADAPTER_ADDRESSES*)malloc(bufLen);

    if (GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, addresses, &bufLen) != NO_ERROR) {
        free(addresses);
        WSACleanup();
        return "";
    }

    for (IP_ADAPTER_ADDRESSES* adapter = addresses; adapter; adapter = adapter->Next) {
        if (adapter->OperStatus != IfOperStatusUp) continue;

        for (IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress; unicast; unicast = unicast->Next) {
            char buf[INET6_ADDRSTRLEN];
            void* addr_ptr = nullptr;

            int family = unicast->Address.lpSockaddr->sa_family;
            if (!use_ipv6 && family == AF_INET) {
                addr_ptr = &((struct sockaddr_in*)unicast->Address.lpSockaddr)->sin_addr;
            } else if (use_ipv6 && family == AF_INET6) {
                addr_ptr = &((struct sockaddr_in6*)unicast->Address.lpSockaddr)->sin6_addr;
            }

            if (addr_ptr) {
                inet_ntop(family, addr_ptr, buf, sizeof(buf));
                std::string candidate(buf);
                if (candidate != "127.0.0.1" && candidate != "::1") {
                    free(addresses);
                    WSACleanup();
                    return candidate;
                }
            }
        }
    }

    free(addresses);
    WSACleanup();
    return "";

#else
    struct ifaddrs* ifaddr;
    if (getifaddrs(&ifaddr) == -1) return "";

    for (struct ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;

        int family = ifa->ifa_addr->sa_family;
        if ((!use_ipv6 && family == AF_INET) || (use_ipv6 && family == AF_INET6)) {
            char addr_buf[INET6_ADDRSTRLEN];
            void* addr_ptr = (family == AF_INET)
                ? (void*)&((struct sockaddr_in*)ifa->ifa_addr)->sin_addr
                : (void*)&((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr;

            inet_ntop(family, addr_ptr, addr_buf, sizeof(addr_buf));
            std::string candidate(addr_buf);
            if (candidate != "127.0.0.1" && candidate != "::1") {
                freeifaddrs(ifaddr);
                return candidate;
            }
        }
    }

    freeifaddrs(ifaddr);
    return "";
#endif
}

bool can_bind_port(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) return false;
#endif

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    bool result = (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0);

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    return result;
}

void install_socket_safety() {
    #ifdef __linux__
        signal(SIGPIPE, SIG_IGN);
    #endif
}

int get_unused_port() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = 0;

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        return -1;
    }

    socklen_t len = sizeof(addr);
    getsockname(sock, (struct sockaddr*)&addr, &len);

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    return ntohs(addr.sin_port);
}
