clean:
	rm -f BlockGuard/*.gch
	rm -f BlockGuard/*.tmp
	rm -f BlockGuard/*.o
	rm -f *.out
	rm -f -r *.dSYM
	rm -f *.o
	rm -f BlockGuard_Test/*.gch
	rm -f BlockGuard_Test/*.tmp
	rm -f BlockGuard_Test/*.o
	clear
	clear

build: 
	clang++ -std=c++14 ./BlockGuard/*.cpp --debug -D_GLIBCXX_DEBUG -o ./BlockGuard.out


test: AltBitPeer
	clang++ -std=c++14 ./BlockGuard_Test/*.cpp ./BlockGuard_Test/*.o --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test.out

Logging:
	clang++ -std=c++14 BlockGuard/Logging/LogWritter.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/LogWritter.o

AltBitPeer: 
	clang++ -std=c++14 BlockGuard/AltBitPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/AltBitPeer.o

BitcoinPeer: 
	clang++ -std=c++14 BlockGuard/BitcoinPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/BitcoinPeer.o

EthereumPeer: 
	clang++ -std=c++14 BlockGuard/EthereumPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/EthereumPeer.o

ExamplePeer: 
	clang++ -std=c++14 BlockGuard/ExamplePeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/ExamplePeer.o

KademliaPeer: 
	clang++ -std=c++14 BlockGuard/KademliaPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/KademliaPeer.o

LinearChordPeer: 
	clang++ -std=c++14 BlockGuard/LinearChordPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/LinearChordPeer.o

PBFTPeer: 
	clang++ -std=c++14 BlockGuard/PBFTPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/PBFTPeer.o

RaftPeer: 
	clang++ -std=c++14 BlockGuard/RaftPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/RaftPeer.o

StableDataLinkPeer: 
	clang++ -std=c++14 BlockGuard/StableDataLinkPeer.cpp -c --debug -D_GLIBCXX_DEBUG -o ./BlockGuard_Test/StableDataLinkPeer.o