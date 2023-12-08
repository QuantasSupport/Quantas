# TODO

1. add tests to verify the results instead of just making sure the program runs


1. clean the code so that it runs with -Wall compiler option, add it to makefile by default

1. allow separate peers to run different code

1. (sometime in the future) make `makefile` generate visual studio project

1. presently, QUANTAS allows to configure maximum delay but not minimum delay. Add minimum delay configuraiton. presently, all channels have the same parametres. 
   Want to have per-channel parameters. 

1. add dynamic networks: configuration, potentially arbitrary topology configuration per round. Or, alternatively, change topology according to user-supplied function

1. add ability to implement varying process speeds. One way of doing it: each round invoke a subset of processes. This way some processes will be "faster" than the other
   related to this: implement reporting which process was executed at which round
   
