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

build: preBuild
	#clang++ -std=c++14 ./BlockGuard/*.cpp *.o -o ./BlockGuard.out -lssl -lcrypto
    #for MacOS and ssl installed via homebrew
    #clang++ -std=c++14 ./BlockGuard/*.cpp *.o -o ./BlockGuard.out -lm /usr/local/opt/openssl/lib/libssl.dylib /usr/local/opt/openssl/lib/libcrypto.dylib
	clang++ -std=c++14 ./BlockGuard/*.cpp *.o -o ./BlockGuard.out

preBuild:
	clang++ -std=c++14 ./BlockGuard/Common/*.cpp -c

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

run_smart-shards:
	tmux new-session -d -s "1" ./BlockGuard.out smartshard ./../results1/
	tmux new-session -d -s "2" ./BlockGuard.out smartshard ./../results2/
	tmux new-session -d -s "3" ./BlockGuard.out smartshard ./../results3/
	tmux new-session -d -s "4" ./BlockGuard.out smartshard ./../results4/
	tmux new-session -d -s "5" ./BlockGuard.out smartshard ./../results5/
	tmux new-session -d -s "6" ./BlockGuard.out smartshard ./../results6/
	tmux new-session -d -s "7" ./BlockGuard.out smartshard ./../results7/
	tmux new-session -d -s "8" ./BlockGuard.out smartshard ./../results8/

run_pbft:
	tmux new-session -d -s "1" ./BlockGuard.out pbft_s ./../results1/
	tmux new-session -d -s "2" ./BlockGuard.out pbft_s ./../results2/
	tmux new-session -d -s "3" ./BlockGuard.out sbft_s ./../results3/
	tmux new-session -d -s "4" ./BlockGuard.out sbft_s ./../results4/
	tmux new-session -d -s "5" ./BlockGuard.out pow_s ./../results5/
	tmux new-session -d -s "6" ./BlockGuard.out pow_s ./../results6/
#tmux new-session -d -s "7" ./BlockGuard.out pow_s ./../results7/
#tmux new-session -d -s "8" ./BlockGuard.out pow_s ./../results8/

run_sbft:
	tmux new-session -d -s "1" ./BlockGuard.out sbft_s ./../results1/
	tmux new-session -d -s "2" ./BlockGuard.out sbft_s ./../results2/
	tmux new-session -d -s "3" ./BlockGuard.out sbft_s ./../results3/
	tmux new-session -d -s "4" ./BlockGuard.out sbft_s ./../results4/
	tmux new-session -d -s "5" ./BlockGuard.out sbft_s ./../results5/
	tmux new-session -d -s "6" ./BlockGuard.out sbft_s ./../results6/
	tmux new-session -d -s "7" ./BlockGuard.out sbft_s ./../results7/
	tmux new-session -d -s "8" ./BlockGuard.out sbft_s ./../results8/

clean_results:
	rm ./../results1/*
	rm ./../results2/*
	rm ./../results3/*
	rm ./../results4/*
	rm ./../results5/*
	rm ./../results6/*
	rm ./../results7/*
	rm ./../results8/*

collect:
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results1/*.csv /Users/kendrichood/Desktop/pbft_s/results1/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results2/*.csv /Users/kendrichood/Desktop/pbft_s/results2/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results3/*.csv /Users/kendrichood/Desktop/pbft_s/results3/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results4/*.csv /Users/kendrichood/Desktop/pbft_s/results4/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results5/*.csv /Users/kendrichood/Desktop/pbft_s/results5/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results6/*.csv /Users/kendrichood/Desktop/pbft_s/results6/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results7/*.csv /Users/kendrichood/Desktop/pbft_s/results7/
	scp -r khood@jadamek1.cs.kent.edu:/home/khood/results8/*.csv /Users/kendrichood/Desktop/pbft_s/results8/
