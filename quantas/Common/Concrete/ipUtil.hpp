#ifndef IP_UTIL_HPP
#define IP_UTIL_HPP

#include <string>
#include <iostream>

#ifdef _WIN32
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <iphlpapi.h>
    #pragma comment(lib, "ws2_32.lib")
    #pragma comment(lib, "iphlpapi.lib")
    typedef int socklen_t;
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <ifaddrs.h>
    #include <cstring>
    #include <signal.h>
#endif

/// Returns the local IP address (IPv4 or IPv6)
std::string get_local_ip(bool use_ipv6 = false);

/// Checks if a port is available for binding
bool can_bind_port(int port);

/// Gets an unused available port by asking the OS
int get_unused_port();

/// Ignore error from sockets being closed on Linux
void install_socket_safety();

#endif // IP_UTIL_HPP
