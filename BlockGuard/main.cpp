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
#include "./Common/Json.hpp"

using std::cout;
using std::string;
using std::cerr;
using blockguard::ExamplePeer;
using blockguard::ExampleMessage;
using blockguard::Simulation;
using nlohmann::json;

void Example(json inputs);

int main(int argc, const char* argv[]) {
	
	srand((float)time(NULL));
	if (argc < 2) {
		cerr << "Error: need an input file" << std::endl;
		return 0;
	}
	string fileName = argv[1];
	std::ifstream inFile(fileName);
	
	if (inFile.fail()) {
		cerr << "Error: need a valid input file" << std::endl;
		return 0;
	}

	json config;
	inFile >> config;

	for (int i = 0; i < config["experiments"].size(); i++) {
		json input = config["experiments"][i];
		string algorithm = input["algorithm"];

		if (algorithm == "example") {
			Example(input);
		}
		else {
			cout << algorithm << " not recognized" << std::endl;
		}
	}
	

	return 0;
}

void Example(json inputs) {
	Simulation<ExampleMessage, ExamplePeer> sim;

	sim.run(inputs);

}