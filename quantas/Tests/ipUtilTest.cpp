#include "../Common/Concrete/ipUtil.hpp"

int main() {
    bool all_passed = true;

    std::string ip = get_local_ip();
    if (ip.empty()) {
        std::cerr << "[X] Failed to detect local IP" << std::endl;
        all_passed = false;
    }

    int test_port = 5050;
    if (!can_bind_port(test_port)) {
        std::cerr << "[X] Port " << test_port << " is NOT available." << std::endl;
        all_passed = false;
    }

    int unused_port = get_unused_port();
    if (unused_port <= 0) {
        std::cerr << "[X] Failed to get an unused port from the OS." << std::endl;
        all_passed = false;
    }

    return all_passed ? 0 : 1;
}
