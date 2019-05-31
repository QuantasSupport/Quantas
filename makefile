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

test: PBFT_Peer PBFTPeer_Sharded PBFTReferenceCommittee ExamplePeer
	clang++ -std=c++14 ./BlockGuard_Test/*.cpp ./BlockGuard_Test/*.o --debug -o ./BlockGuard_Test.out

PBFT_Peer: 
	clang++ -std=c++14 BlockGuard/PBFT_Peer.cpp -c --debug -o ./BlockGuard_Test/PBFT_Peer.o

ExamplePeer: 
	clang++ -std=c++14 BlockGuard/ExamplePeer.cpp -c --debug -o ./BlockGuard_Test/ExamplePeer.o

PBFTPeer_Sharded:
	clang++ -std=c++14 BlockGuard/PBFTPeer_Sharded.cpp -c --debug -o ./BlockGuard_Test/PBFTPeer_Sharded.o

PBFTReferenceCommittee:
	clang++ -std=c++14 BlockGuard/PBFTReferenceCommittee.cpp -c --debug -o ./BlockGuard_Test/PBFTReferenceCommittee.o

collect:
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results1/*.csv /Users/kendrichood/Desktop/pbft_s/results1/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results2/*.csv /Users/kendrichood/Desktop/pbft_s/results2/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results3/*.csv /Users/kendrichood/Desktop/pbft_s/results3/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results4/*.csv /Users/kendrichood/Desktop/pbft_s/results4/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results5/*.csv /Users/kendrichood/Desktop/pbft_s/results5/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results6/*.csv /Users/kendrichood/Desktop/pbft_s/results6/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results7/*.csv /Users/kendrichood/Desktop/pbft_s/results7/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results8/*.csv /Users/kendrichood/Desktop/pbft_s/results8/
