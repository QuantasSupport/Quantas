# Copyright 2022

# This file is part of QUANTAS.
# QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
# QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
# You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.

clean:
	rm -f quantas/*.gch
	rm -f quantas/*.tmp
	rm -f quantas/*.o
	rm -f *.out
	rm -f -r *.dSYM
	rm -f *.o
	rm -f quantas_test/*.gch
	rm -f quantas_test/*.tmp
	rm -f quantas_test/*.o
	clear
	clear

prod:
	g++ -O2 -pthread ./quantas/*.cpp -o ./quantas.out

Windows:
	g++ -pthread ./quantas/*.cpp -std=c++11 -o quantas

Clang:
	clang++ -std=c++14 -pthread ./quantas/*.cpp -o ./quantas.out

debug:
	g++ ./quantas/*.cpp -pthread --debug -D_GLIBCXX_DEBUG -o ./quantas.out

AltBitPeer: 
	g++ quantas/AltBitPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./quantas_test/AltBitPeer.o

BitcoinPeer: 
	g++ quantas/BitcoinPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./quantas_test/BitcoinPeer.o

EthereumPeer: 
	g++ quantas/EthereumPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./quantas_test/EthereumPeer.o

ExamplePeer: 
	g++ quantas/ExamplePeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./quantas_test/ExamplePeer.o

KademliaPeer: 
	g++ quantas/KademliaPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./quantas_test/KademliaPeer.o

LinearChordPeer: 
	g++ quantas/LinearChordPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./quantas_test/LinearChordPeer.o

PBFTPeer: 
	g++ quantas/PBFTPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./quantas_test/PBFTPeer.o

RaftPeer: 
	g++ quantas/RaftPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./quantas_test/RaftPeer.o

StableDataLinkPeer: 
	g++ quantas/StableDataLinkPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./quantas_test/StableDataLinkPeer.o
