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
	clang++ -std=c++14 ./BlockGuard/*.cpp -o ./BlockGuard.out

test: ExamplePeer
	clang++ -std=c++14 ./BlockGuard_Test/*.cpp ./BlockGuard_Test/*.o --debug -o ./BlockGuard_Test.out

ExamplePeer: 
	clang++ -std=c++14 BlockGuard/ExamplePeer.cpp -c --debug -o ./BlockGuard_Test/ExamplePeer.o
