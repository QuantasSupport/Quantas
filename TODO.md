# TODO
0. int maxMsgsRec = topology.value("maxMsgsRec", INT_MAX); Do the maxMsgsRec stuff

1. add more to the tests to verify the results instead of just making sure the program runs

2. allow separate peers to run different code

3. presently, QUANTAS allows to configure maximum delay but not minimum delay. Add minimum delay configuraiton. presently, all channels have the same parameters. 
   Want to have per-channel parameters. 

4. add ability to "skip" ahead a large number of rounds delivering all the necessary messages. This would help to simulate more realistic conditions in some algorithms. Such as in bitcoin where it takes 10 minutes to mine a block but only 1s to send a round trip message. So 3600 rounds need to pass between blocks being mined when compared to sending messages.

5. change LinearChordPeer to fully implement Chord