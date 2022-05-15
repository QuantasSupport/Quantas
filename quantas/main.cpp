/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <fstream>
#include <set>
#include <chrono>
#include <random>

#include "Common/Network.hpp"
#include "Common/NetworkInterface.hpp"
#include "Common/Simulation.hpp"
#include "Common/Json.hpp"
#include "ExamplePeer.hpp"
#include "BitcoinPeer.hpp"
#include "EthereumPeer.hpp"
#include "PBFTPeer.hpp"
#include "LinearChordPeer.hpp"
#include "RaftPeer.hpp"
#include "KademliaPeer.hpp"
#include "AltBitPeer.hpp"
#include "StableDataLinkPeer.hpp"
#include "ChangRobertsPeer.hpp"

using quantas::Simulation;
using nlohmann::json;
using quantas::ExamplePeer;
using quantas::ExampleMessage;
using quantas::BitcoinPeer;
using quantas::BitcoinMessage;
using quantas::EthereumPeer;
using quantas::EthereumPeerMessage;
using quantas::PBFTPeer;
using quantas::PBFTPeerMessage;
using quantas::RaftPeer;
using quantas::RaftPeerMessage;
using quantas::LinearChordPeer;
using quantas::LinearChordMessage;
using quantas::KademliaPeer;
using quantas::KademliaMessage;
using quantas::AltBitPeer;
using quantas::AltBitMessage;
using quantas::StableDataLinkPeer;
using quantas::StableDataLinkMessage;
using quantas::ChangRobertsPeer;
using quantas::ChangRobertsMessage;

int main(int argc, const char* argv[]) {
	
	srand((float)time(NULL));
	if (argc < 2) {
		std::cerr << "Error: need an input file" << std::endl;
		return 0;
	}
	std::string fileName = argv[1];
	std::ifstream inFile(fileName);
	
	if (inFile.fail()) {
		std::cerr << "Error: need a valid input file" << std::endl;
		return 0;
	}

	json config;
	inFile >> config;

	for (int i = 0; i < config["experiments"].size(); i++) {
		json input = config["experiments"][i];
		std::string algorithm = input["algorithm"];

		if (algorithm == "example") {
			Simulation<ExampleMessage, ExamplePeer> sim;
			sim.run(input);
		}
		else if (algorithm == "bitcoin") {
			Simulation<BitcoinMessage, BitcoinPeer> sim;
			sim.run(input);
		}
		else if (algorithm == "Ethereum") {
			Simulation<EthereumPeerMessage, EthereumPeer> sim;
			sim.run(input);
		}
		else if (algorithm == "PBFT") {
			Simulation<PBFTPeerMessage, PBFTPeer> sim;
			sim.run(input);
		}
		else if (algorithm == "Raft") {
			Simulation<RaftPeerMessage, RaftPeer> sim;
			sim.run(input);
		}
		else if (algorithm == "LinearChord") {
			Simulation<LinearChordMessage, LinearChordPeer> sim;
			sim.run(input);
		}
		else if (algorithm == "Kademlia") {
			Simulation<KademliaMessage, KademliaPeer> sim;
			sim.run(input);
		}
		else if (algorithm == "AltBit") {
			Simulation<AltBitMessage, AltBitPeer> sim;
			sim.run(input);
		}
		else if (algorithm == "StableDataLink") {
			Simulation<StableDataLinkMessage, StableDataLinkPeer> sim;
			sim.run(input);
		}
		else if (algorithm == "changroberts") {
			Simulation<ChangRobertsMessage, ChangRobertsPeer> sim;
			sim.run(input);
		}
		else {
			std::cout << algorithm << " not recognized" << std::endl;
		}
	}

	return 0;
}