#include <iostream>
#include "../Peer.hpp"
#include "NetworkInterfaceConcrete.hpp"
#include "../RoundManager.hpp"
#include "../memoryUtil.hpp"

using namespace quantas;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./peer <config.json> [port]\n";
        return 1;
    }

    std::chrono::time_point<std::chrono::high_resolution_clock> startTime, endTime; // chrono time points
    std::chrono::duration<double> duration; // chrono time interval
    startTime = std::chrono::high_resolution_clock::now();

    RoundManager::asynchronous();
    RoundManager::setLastRound(RoundManager::currentRound() + 10000);
    
    auto * peer = PeerRegistry::makePeer("AltBitPeerConcrete");
    std::vector<Peer*> peer_vector = {peer};
    if (auto networkInterface = dynamic_cast<NetworkInterfaceConcrete*>(peer->getNetworkInterface())) {
        if (argc == 2) {
            networkInterface->load_config(argv[1]);
        } else {
            networkInterface->load_config(argv[1], std::stoi(argv[2]));
        }
        int counter = 9000;
        while(!networkInterface->getShutdownCondition()) {
            if (!peer->isCrashed()) {
                peer->receive();
                peer->performComputation();
            }
            
            if (int(RoundManager::lastRound()) - int(RoundManager::currentRound()) < counter) {
                std::cout << 10 - (counter / 1000) << " seconds passed." << std::endl;
                counter -= 1000;
            }

            if (RoundManager::lastRound() <= RoundManager::currentRound()) {
                networkInterface->shutDown();
                peer->endOfRound(peer_vector);
            } else {
                peer->endOfRound(peer_vector);
            }
        }
        networkInterface->clearAll();
    }
    
    endTime = std::chrono::high_resolution_clock::now();
   	duration = endTime - startTime;
    LogWriter::setValue("RunTime", double(duration.count()));
    size_t peakMemoryKB = getPeakMemoryKB();
    LogWriter::setValue("Peak Memory KB", peakMemoryKB);

    LogWriter::print();
    return 0;
}
