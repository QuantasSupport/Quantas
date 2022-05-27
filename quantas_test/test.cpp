/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include "NetworkTests.hpp"
#include "Peer_Test.hpp"

#include <string>

int main(int argc, const char * argv[]){
    if(argc < 2){
        std::cerr << "Error: need test to run and output path" << std::endl;
    }

    std::string testOption = argv[1];
    std::string filePath = argv[2];

    if(testOption == "all"){
        runNetworkTests(filePath);
        RunPeerTests(filePath);
    }else if(testOption == "network"){
        runNetworkTests(filePath);
    }else if(testOption == "peer"){
        RunPeerTests(filePath);
    }

    return 0;
}


