# TODO

1. add more to the tests to verify the results instead of just making sure the program runs

2. allow separate peers to run different code

4. add ability to "skip" ahead a large number of rounds delivering all the necessary messages. This would help to simulate more realistic conditions in some algorithms. Such as in bitcoin where it takes 10 minutes to mine a block but only 1s to send a round trip message. So 3600 rounds need to pass between blocks being mined when compared to sending messages.

5. change LinearChordPeer to fully implement Chord

6. check the memory management of messages, packets, channels, etc.