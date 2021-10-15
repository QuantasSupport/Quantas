//
//  main.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <set>
#include <chrono>
#include <random>

#include "./Common/Network.hpp"
#include "./Common/NetworkInterface.hpp"
#include "./Common/Simulation.hpp"
#include "ExamplePeer.hpp"

using std::cout;
using std::ofstream;
using std::string;
using std::cerr;
using blockguard::ExamplePeer;
using blockguard::ExampleMessage;
using blockguard::Network;
using blockguard::Simulation;

void Example(string logFile);

int main(int argc, const char* argv[]) {
	
	srand((float)time(NULL));
	if (argc < 3) {
		cerr << "Error: need algorithm and output path" << std::endl;
		return 0;
	}


	string algorithm = argv[1];
	string filePath = argv[2];

	if (algorithm == "example") {
		Example(filePath);
	}
	else {
		cout << algorithm << " not recognized" << std::endl;
	}

	return 0;
}

void Example(string logFile) {
	Simulation<ExampleMessage, ExamplePeer> sim;
	sim.setLog(logFile);
	sim.setToUniform();
	sim.setMaxDelay(3);
	sim.setInitialPeers(5);
	sim.setTotalPeers(5);
	sim.run(1, 3);

	sim = Simulation<ExampleMessage, ExamplePeer>(); // clear old setup by creating a fresh object
	sim.setLog("cout");
	sim.setToUniform();
	sim.setMaxDelay(10);
	sim.setInitialPeers(3);
	sim.setTotalPeers(3);
	sim.run(1, 3);

	
}