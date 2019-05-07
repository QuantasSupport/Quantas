clean:
	rm -f BlockGuard/*.gch
	rm -f BlockGuard/*.tmp
	rm -f BlockGuard/*.o
	rm -f a.out
	rm -f -r *.dSYM
	rm -f *.o
	rm -f BlockGuard_Test/*.gch
	rm -f BlockGuard_Test/*.tmp
	rm -f BlockGuard_Test/*.o
	clear
	clear

build:
	clang++ -std=c++14 BlockGuard/*.cpp --debug -o ./BlockGuard.out

test: PBFT_Peer_Test PBFTPeer_Sharded_Test PBFTReferenceCommittee_Test
	clang++ -std=c++14 BlockGuard_Test/*.cpp BlockGuard_Test/*.o --debug -o ./BlockGuard_Test.out

PBFT_Peer_Test: 
	clang++ -std=c++14 BlockGuard/PBFT_Peer.cpp -c --debug -o ./BlockGuard_Test/PBFT_Peer.o

PBFTPeer_Sharded_Test:
	clang++ -std=c++14 BlockGuard/PBFTPeer_Sharded.cpp -c --debug -o ./BlockGuard_Test/PBFTPeer_Sharded.o

PBFTReferenceCommittee_Test:
	clang++ -std=c++14 BlockGuard/PBFTReferenceCommittee.cpp -c --debug -o ./BlockGuard_Test/PBFTReferenceCommittee.o