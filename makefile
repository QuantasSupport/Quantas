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
	clang++ -std=c++14 ./BlockGuard/*.cpp --debug -o ./BlockGuard.out

test: PBFT_Peer PBFTPeer_Sharded PBFTReferenceCommittee
	clang++ -std=c++14 ./BlockGuard_Test/*.cpp ./BlockGuard_Test/*.o --debug -o ./BlockGuard_Test.out

PBFT_Peer: 
	clang++ -std=c++14 BlockGuard/PBFT_Peer.cpp -c --debug -o ./BlockGuard_Test/PBFT_Peer.o

PBFTPeer_Sharded:
	clang++ -std=c++14 BlockGuard/PBFTPeer_Sharded.cpp -c --debug -o ./BlockGuard_Test/PBFTPeer_Sharded.o

PBFTReferenceCommittee:
	clang++ -std=c++14 BlockGuard/PBFTReferenceCommittee.cpp -c --debug -o ./BlockGuard_Test/PBFTReferenceCommittee.o

collect:
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results1 ~/Desktop/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results2 ~/Desktop/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results3 ~/Desktop/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results4 ~/Desktop/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results5 ~/Desktop/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results6 ~/Desktop/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results7 ~/Desktop/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results8 ~/Desktop/
