/*
  Copyright 2022

  This file is part of QUANTAS.  QUANTAS is free software: you can
  redistribute it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.
  QUANTAS is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.  You should have received a copy of the GNU General
  Public License along with QUANTAS. If not, see
  <https://www.gnu.org/licenses/>.

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

using nlohmann::json;

int main(int argc, const char* argv[]) {

   srand(time(nullptr));
   if (argc < 2) {
      std::cerr << "usage: " << argv[0] << " inputFileName "<< std::endl;
      return 1;
   }
	
   std::ifstream inFile(argv[1]);
	
   if (inFile.fail()) {
      std::cerr << "error: cannot open input file" << std::endl;
      return 1;
   }
	
   json config;
   inFile >> config;

   for (int i = 0; i < config["experiments"].size(); ++i) {
      json input = config["experiments"][i];

// #if defined(ExamplePeer_hpp)
// 	Simulation<quantas::ExampleMessage, quantas::ExamplePeer> sim;
// #elif defined(BitcoinPeer_hpp)
// 	Simulation<quantas::BitcoinMessage, quantas::BitcoinPeer> sim;
// #elif defined(EthereumPeer_hpp)
//    Simulation<quantas::EthereumPeerMessage, quantas::EthereumPeer> sim;
// #elif defined(PBFTPeer_hpp)
// 	Simulation<quantas::PBFTPeerMessage, quantas::PBFTPeer> sim;
// #elif defined(RaftPeer_hpp)
// 	Simulation<quantas::RaftPeerMessage, quantas::RaftPeer> sim;
// #elif defined(SmartShardsPeer_hpp)
// 	Simulation<quantas::SmartShardsMessage, quantas::SmartShardsPeer> sim;
// #elif defined(LinearChordPeer_hpp)
// 	Simulation<quantas::LinearChordMessage, quantas::LinearChordPeer> sim;
// #elif defined(KademliaPeer_hpp)
// 	Simulation<quantas::KademliaMessage, quantas::KademliaPeer> sim;
// #elif defined(AltBitPeer_hpp)
// 	Simulation<quantas::AltBitMessage, quantas::AltBitPeer> sim;
// #elif defined(StableDataLinkPeer_hpp)
// 	Simulation<quantas::StableDataLinkMessage, quantas::StableDataLinkPeer> sim;	 
// #elif defined(ChangRobertsPeer_hpp)
// 	Simulation<quantas::ChangRobertsMessage, quantas::ChangRobertsPeer> sim;
// #elif defined(DynamicPeer_hpp)
// 	Simulation<quantas::DynamicMessage, quantas::DynamicPeer> sim;
// #elif defined(KPTPeer_hpp)
// 	Simulation<quantas::KPTMessage, quantas::KPTPeer> sim;
// #elif defined(KSMPeer_hpp)
// 	Simulation<quantas::KSMMessage, quantas::KSMPeer> sim;
// #endif
      quantas::SimWrapper* sim = quantas::generateSim();
	   sim->run(input);
      delete sim;
   }

   return 0;
}
