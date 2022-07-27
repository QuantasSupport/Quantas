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


using quantas::Simulation;
using nlohmann::json;


#ifdef EXAMPLE_PEER
#include "ExamplePeer.hpp"
#endif

#ifdef BITCOIN_PEER
#include "BitcoinPeer.hpp"
#endif

#ifdef ETHEREUM_PEER
#include "EthereumPeer.hpp"
#endif

#ifdef PBFT_PEER
#include "PBFTPeer.hpp"
#endif

#ifdef RAFT_PEER
#include "RaftPeer.hpp"
#endif

#ifdef SMARTSHARDS_PEER
#include "SmartShardsPeer.hpp"
#endif

#ifdef LINEARCHORD_PEER
#include "LinearChordPeer.hpp"
#endif

#ifdef KADEMLIA_PEER
#include "KademliaPeer.hpp"
#endif

#ifdef ALTBIT_PEER
#include "AltBitPeer.hpp"
#endif

#ifdef STABLEDATALINK_PEER
#include "StableDataLinkPeer.hpp"
#endif

#ifdef CHANGROBERTS_PEER
#include "ChangRobertsPeer.hpp"
#endif

#ifdef DYNAMIC_PEER
#include "DynamicPeer.hpp"
#endif

#ifdef KPT_PEER
#include "KPTPeer.hpp"
#endif

#ifdef KSM_PEER
#include "KSMPeer.hpp"
#endif


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

#ifdef EXAMPLE_PEER
	Simulation<quantas::ExampleMessage, quantas::ExamplePeer> sim;
#elif BITCOIN_PEER
	Simulation<quantas::BitcoinMessage, quantas::BitcoinPeer> sim;
#elif ETHEREUM_PEER
        Simulation<quantas::EthereumPeerMessage, quantas::EthereumPeer> sim;
#elif PBFT_PEER
	Simulation<quantas::PBFTPeerMessage, quantas::PBFTPeer> sim;
#elif RAFT_PEER
	Simulation<quantas::RaftPeerMessage, quantas::RaftPeer> sim;
#elif SMARTSHARDS_PEER
	Simulation<quantas::SmartShardsMessage, quantas::SmartShardsPeer> sim;
#elif LINEARCHORD_PEER
	Simulation<quantas::LinearChordMessage, quantas::LinearChordPeer> sim;
#elif KADEMLIA_PEER
	Simulation<quantas::KademliaMessage, quantas::KademliaPeer> sim;
#elif ALTBIT_PEER
	Simulation<quantas::AltBitMessage, quantas::AltBitPeer> sim;
#elif STABLEDATALINK_PEER
	Simulation<quantas::StableDataLinkMessage, quantas::StableDataLinkPeer> sim;	 
#elif CHANGROBERTS_PEER
	Simulation<quantas::ChangRobertsMessage, quantas::ChangRobertsPeer> sim;
#elif DYNAMIC_PEER
	Simulation<quantas::DynamicMessage, quantas::DynamicPeer> sim;
#elif KPT_PEER
	Simulation<quantas::KPTMessage, quantas::KPTPeer> sim;
#elif KSM_PEER
	Simulation<quantas::KSMMessage, quantas::KSMPeer> sim;
#endif
	   sim.run(input);
   }

   return 0;
}
