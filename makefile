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

build: ExamplePeer
	clang++ -std=c++14 ./BlockGuard/*.cpp --debug -D_GLIBCXX_DEBUG -o -f ./BlockGuard/Common -f ./BlockGuard/Logging ./BlockGuard.out

test: ExamplePeer
	clang++ -std=c++14 ./BlockGuard_Test/*.cpp ./BlockGuard_Test/*.o --debug -D_GLIBCXX_DEBUG -o -f ./BlockGuard/Common -f ./BlockGuard/Logging ./BlockGuard_Test.out

ExamplePeer: 
	clang++ -std=c++14 BlockGuard/ExamplePeer.cpp -c --debug -D_GLIBCXX_DEBUG -o -f ./BlockGuard/Common -f ./BlockGuard/Logging ./BlockGuard_Test/ExamplePeer.o
