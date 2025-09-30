# TODO

add more to the tests to verify the results instead of just making sure the program runs

add ability to "skip" ahead a large number of rounds delivering all the necessary messages. This would help to simulate more realistic conditions in some algorithms. Such as in bitcoin where it takes 10 minutes to mine a block but only 1s to send a round trip message. So 3600 rounds need to pass between blocks being mined when compared to sending messages.

move to init parameters and end of round running for each peer individually and then compute the results after for graphing instead of while running to make more compatible with distrubted simulation

Allow the input files to be used the same way for concrete and abstract simulation to allow an algorithm to do both