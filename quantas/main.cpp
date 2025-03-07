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
#include <filesystem>

#include "Common/Network.hpp"
#include "Common/NetworkInterface.hpp"
#include "Common/Simulation.hpp"
#include "Common/Json.hpp"

using nlohmann::json;

int main(int argc, const char* argv[]) {
   if (argc < 2) {
      std::cerr << "usage: " << argv[0] << " inputFileName "<< std::endl;
      return 1;
   }

   std::ifstream inFile(argv[1]);
	
   if (inFile.fail()) {
      std::cerr << "error: cannot open input file: " << argv[1] << std::endl;
      return 1;
   }
   
   json config;
   inFile >> config;

   for (int i = 0; i < config["experiments"].size(); ++i) {
      json input = config["experiments"][i];
      quantas::Simulation sim;
	   sim.run(input);
   }

   return 0;
}
