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
using quantas::ExamplePeer;
using quantas::ExampleMessage;
#endif

#ifdef BITCOIN_PEER
#include "BitcoinPeer.hpp"
using quantas::BitcoinPeer;
using quantas::BitcoinMessage;
#endif

#ifdef ETHEREUM_PEER
#include "EthereumPeer.hpp"
using quantas::EthereumPeer;
using quantas::EthereumPeerMessage;
#endif

#ifdef PBFT_PEER
#include "PBFTPeer.hpp"
using quantas::PBFTPeer;
using quantas::PBFTPeerMessage;
#endif

#ifdef RAFT_PEER
#include "RaftPeer.hpp"
using quantas::RaftPeer;
using quantas::RaftPeerMessage;
#endif

#ifdef SMARTSHARDS_PEER
#include "SmartShardsPeer.hpp"
using quantas::SmartShardsPeer;
using quantas::SmartShardsMessage;
#endif

#ifdef LINEARCHORD_PEER
#include "LinearChordPeer.hpp"
using quantas::LinearChordPeer;
using quantas::LinearChordMessage;
#endif

#ifdef KADEMLIA_PEER
#include "KademliaPeer.hpp"
using quantas::KademliaPeer;
using quantas::KademliaMessage;
#endif

#ifdef ALTBIT_PEER
#include "AltBitPeer.hpp"
using quantas::AltBitPeer;
using quantas::AltBitMessage;
#endif

#ifdef STABLEDATALINK_PEER
#include "StableDataLinkPeer.hpp"
using quantas::StableDataLinkPeer;
using quantas::StableDataLinkMessage;
#endif


#ifdef CHANGROBERTS_PEER
#include "ChangRobertsPeer.hpp"
using quantas::ChangRobertsPeer;
using quantas::ChangRobertsMessage;
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
	 Simulation<ExampleMessage, ExamplePeer> sim;
#elif BITCOIN_PEER
	 Simulation<BitcoinMessage, BitcoinPeer> sim;
#elif ETHEREUM_PEER
         Simulation<EthereumPeerMessage, EthereumPeer> sim;
#elif PBFT_PEER
	 Simulation<PBFTPeerMessage, PBFTPeer> sim;
#elif RAFT_PEER
	 Simulation<RaftPeerMessage, RaftPeer> sim;
#elif SMARTSHARDS_PEER
	 Simulation<SmartShardsMessage, SmartShardsPeer> sim;
#elif LINEARCHORD_PEER
	 Simulation<LinearChordMessage, LinearChordPeer> sim;
#elif KADEMLIA_PEER
	 Simulation<KademliaMessage, KademliaPeer> sim;
#elif ALTBIT_PEER
	 Simulation<AltBitMessage, AltBitPeer> sim;
#elif STABLEDATALINK_PEER
	 Simulation<StableDataLinkMessage, StableDataLinkPeer> sim;	 
#elif CHANGROBERTS_PEER
	 Simulation<ChangRobertsMessage, ChangRobertsPeer> sim;
#endif
	 sim.run(input);
   }

   return 0;
}

