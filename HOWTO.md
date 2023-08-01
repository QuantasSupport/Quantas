# QUANTAS Tutorial

This file contains a step-by-step how-to to write and simulate a distributed algorithm with QUANTAS. The use case will be the Chang and Roberts leader election algorithm.

## The Chang and Roberts leader election algorithm

The Chang and Roberts leader election algorithm runs on a ring shaped networks where processes have unique IDs. The algorithm (almost) runs as follows:

	Initially:
		Send my ID to whatever neighbor

	Upon receipt of message M from neighbor N:
		If M.ID = ID Then
			I am the leader
		Else if M.ID > ID Then
			Send M.ID to the other neighbor (not N)

## Step 1: Subclass ``Peer`` for the new algorithm

The new algorithm has no local variables, and the content of every message is a single ID (represented as a ``long``). From the ``ExamplePeer.hpp`` template, we create the ``ChangRobertsPeer.hpp`` file as follows:

	#ifndef ChangRobertsPeer_hpp
	#define ChangRobertsPeer_hpp

	#include "Common/Peer.hpp"

	namespace quantas{

	    using std::string;
	    using std::ostream;

	    // message body type : ID of a process
	    struct ChangRobertsMessage{
	        long aPeerId;
	    };

	    class ChangRobertsPeer : public Peer<ChangRobertsMessage>{
	    public:
	        ChangRobertsPeer                             (long);
	        ChangRobertsPeer                             (const ChangRobertsPeer &rhs);
	        ~ChangRobertsPeer                            ();

	        void                 performComputation ();
	        void                 endOfRound         (const vector<Peer<ChangRobertsMessage>*>& _peers);
	        void                 log()const { printTo(*_log); };
	        ostream&             printTo(ostream&)const;
	        friend ostream& operator<<         (ostream&, const ChangRobertsPeer&);
	    };
	}
	#endif

Similarly, from the ``ExamplePeer.cpp``, we create the following ``ChangRobertsPeer.cpp``file as follows:

	#include <iostream>
	#include "ChangRobertsPeer.hpp"

	namespace quantas {

		ChangRobertsPeer::~ChangRobertsPeer() {
		}

		ChangRobertsPeer::ChangRobertsPeer(const ChangRobertsPeer& rhs) : Peer<ChangRobertsMessage>(rhs) {
		}

		ChangRobertsPeer::ChangRobertsPeer(long id) : Peer(id) {
		}

		void ChangRobertsPeer::performComputation() {
		}

		void ChangRobertsPeer::endOfRound(const vector<Peer<ChangRobertsMessage>*>& _peers) {
			cout << "End of round " << getRound() << endl;
		}

		ostream& ChangRobertsPeer::printTo(ostream& out)const {
			Peer<ChangRobertsMessage>::printTo(out);

			out << id() << endl;
			out << "counter:" << getRound() << endl;

			return out;
		}

		ostream& operator<< (ostream& out, const ChangRobertsPeer& peer) {
			peer.printTo(out);
			return out;
		}
	}

## Step 2: Implement the Algorithm

To implement the algorithm, we need to redefine the behavior of the ``performComputation()``method, that is called at every round of the current test for every simulated peer.

### Initially

The initial part of the algorithm can be implemented testing if the current round is equal to `0`. Send a message to any node is done using the ``unicast()`` method. The following code:

	if(getRound() == 0) {
		ChangRobertsMessage msg;
		msg.aPeerId = id();
		unicast(msg);
	}

thus implemented this part of the algorithm:

	Initially:
		Send my ID to whatever neighbor

### Upon receipt of message M from neighbor N

The regular part of the algorithm can be implemented using the ``inStreamEmpty()`` method to check if the incoming channel contains messages, the ``popInStream()`` to grab the first message in the incoming channel, and the ``sourceId()`` method of the received message to obtain the sender ID of the received packet. Forwarding to the other neighbor is done using the ``broadcastBut()`` that send the message to all neighbors but one (specified in parameter). The following code:

	while (!inStreamEmpty()) {
		Packet<ChangRobertsMessage> newMsg = popInStream();
		long rid = newMsg.getMessage().aPeerId;
		long sid = newMsg.sourceId();
		if( rid == id() ) {
			cout << "Realizing " << id() << " is the leader" << endl;
		}
		else {
			if( rid > id() ) {
				ChangRobertsMessage msg;
				msg.aPeerId = rid;
				broadcastBut(msg,sid);
			}
		}
	}

### A complete ``performComputation()`` method

The complete code for the ``performComputation()``method should look as follows:

	void ChangRobertsPeer::performComputation() {
		if(getRound() == 0) {
			ChangRobertsMessage msg;
			msg.aPeerId = id();
			unicast(msg)
		}
		while (!inStreamEmpty()) {
			Packet<ChangRobertsMessage> newMsg = popInStream();
			long rid = newMsg.getMessage().aPeerId;
			long sid = newMsg.sourceId();
			if( rid == id() ) {
				cout << "Realizing " << id() << " is the leader" << endl;
			}
			else {
				if( rid > id() ) {
					ChangRobertsMessage msg;
					msg.aPeerId = rid;
					broadcastBut(msg,sid);
				}
			}
		}
	}

## Step 3: Add the algorithm to the QUANTAS library

QUANTAS is not yet aware that the new algorithm is available. One must add it to the ``main.cpp`` file as follows:

- at the end of the includes, add:

		#ifdef CHANGROBERTS_PEER
		#include "ChangRobertsPeer.hpp"
		#endif

- at the end of the elifs, add:

		#elif CHANGROBERTS_PEER
		Simulation<quantas::ChangRobertsMessage, quantas::ChangRobertsPeer> sim;
		#endif

## Step 4: Set up and run an experiment

QUANTAS experiments are planned through a simple JSON file. For this experiment, we want to run the ``changroberts`` algorithm 10 times, lasting each 15 rounds. The network topology will be a ring of 10 nodes. With these parameters, the ``ChangRobertsInput.json`` file should look as follows:

	{
	  "experiments": [
	    {
	      "algorithm": "changroberts",
	      "logFile": "ChangRoberts.txt",
	      "topology": {
	        "type": "ring",
	        "initialPeers": 10,
	      },
	      "tests": 10,
	      "rounds": 15
	    }
	  ]
	}

We shall update the `makefile` to include the new algorithm by adding:

	INPUTFILE := $(PROJECT_DIR)/ChangRobertsInput.json

	ALG := CHANGROBERTS_PEER
	ALGFILE := ChangRobertsPeer

For debugging, on a Unix system, we may compile the program as follows:

	make debug
or just plain:

	make

If there are no errors, we can run the experiment:

	make release
	make run

This particular execution should output:

	Realizing 9 is the leader
	Realizing 9 is the leader
	Realizing 9 is the leader
	Realizing 9 is the leader
	Realizing 9 is the leader
	Realizing 9 is the leader
	Realizing 9 is the leader
	Realizing 9 is the leader
	Realizing 9 is the leader
	Realizing 9 is the leader

So, 10 tests were done and the peer with ID = 9 was elected in all of them, as expected.

The file ``ChangRoberts.txt`` was created in the current directory, and contains (actual numbers may vary depending the running machine) only basic statistics:

	{
		"RunTime": 0.010874242,
	}

## Step 5: Instrumenting the simulation

When simulating to obtain quantitative results, it is often necessary to instrument the code (that is, to output specific variable values at some point in the simulation). Here, we wish to know how long it takes to elect a leader, and how many messages in total are exchanged in each round. Instrumentation in Quantas is done using the ``LogWriter`` class that acts as a JSON dictionary.

For example, to retain how many rounds were necessary to elect a leader in each test, one can simply write:

	LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["election_time"] = getRound();

after realizing that a leader was elected (when the received ID is the same as our own).

To collect metrics that are related to all peers at every round, one can find the ``endOfRound()`` method handy. For our example, we want to collect all messages sent since the beginning of the test once the leader is elected. For this purpose, we add a ``messages_sent`` instance variable to each peer that maintains the number of sent messages, and a ``first_elected`` instance variable to each peer that is set to ``true`` the first time a peer is elected the leader.

	class ChangRobertsPeer : public Peer<ChangRobertsMessage>{
		...
		private:
			bool first_elected;
			long messages_sent;
	};

This ``messages_sent`` is initialized to zero, and the ``first_elected`` is initialized to ``false``.

	ChangRobertsPeer::ChangRobertsPeer(const ChangRobertsPeer& rhs) : Peer<ChangRobertsMessage>(rhs),
		messages_sent(0), first_elected(false) {
	}

	ChangRobertsPeer::ChangRobertsPeer(long id) : Peer(id), messages_sent(0), first_elected(false) {
	}

Whenever we call ``unicast()`` or ``broadcastBut()``, we increment the ``messages_sent`` variable. When we first realize a leader is elected, we update the ``first_elected`` variable accordingly.

	void ChangRobertsPeer::performComputation() {
		first_elected = false;
		if(getRound() == 0) {
			ChangRobertsMessage msg;
			msg.aPeerId = id();
			unicast(msg);
			++messages_sent;
		}
		while (!inStreamEmpty()) {
			Packet<ChangRobertsMessage> newMsg = popInStream();
			long rid = newMsg.getMessage().aPeerId;
			long sid = newMsg.sourceId();
			if( rid == id() ) {
				first_elected = true;
				cout << "Realizing " << id() << " is the leader" << endl;
			}
			else {
				if( rid > id() ) {
					ChangRobertsMessage msg;
					msg.aPeerId = rid;
					broadcastBut(msg,sid);
					++messages_sent;
				}
			}
		}
	}

Then, at the end of each round, we check whether a leader has been elected in this round. If so, we simply accumulate all messages and log them, along with the current round number and the elected identifier:

	void ChangRobertsPeer::endOfRound(const vector<Peer<ChangRobertsMessage>*>& _peers) {
		long all_messages_sent = 0;
		bool elected = false;
		long elected_id = -1;
		const vector<ChangRobertsPeer*> peers = reinterpret_cast<vector<ChangRobertsPeer*> const&>(_peers);
		for(auto it = peers.begin(); it != peers.end(); ++it) {
			all_messages_sent += (*it)->messages_sent;
			if((*it)->first_elected) {
				elected = true;
				elected_id = (*it)->id();
			}
		}
		if(elected) {
			LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["nb_messages"] = all_messages_sent;
			LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["election_time"] = getRound();
			LogWriter::instance()->data["tests"][LogWriter::instance()->getTest()]["elected_id"] = elected_id;
		}
	}

Now that the algorithm is instrumented, we run the simulation again (on a Unix-like system):

	make prod
	make run

The file ``ChangRoberts.txt`` now contains more detailed statistics:

	{
	"RunTime": 0.018834804,
	"tests": [
	    {
	        "elected_id": 9,
	        "election_time": 10,
	        "nb_messages": 27
	    },
	    {
	        "elected_id": 9,
	        "election_time": 10,
	        "nb_messages": 30
	    },
	    {
	        "elected_id": 9,
	        "election_time": 10,
	        "nb_messages": 24
	    },
	    {
	        "elected_id": 9,
	        "election_time": 10,
	        "nb_messages": 31
	    },
	    {
	        "elected_id": 9,
	        "election_time": 10,
	        "nb_messages": 26
	    },
	    {
	        "elected_id": 9,
	        "election_time": 10,
	        "nb_messages": 32
	    },
	    {
	        "elected_id": 9,
	        "election_time": 10,
	        "nb_messages": 31
	    },
	    {
	        "elected_id": 9,
	        "election_time": 10,
	        "nb_messages": 25
	    },
	    {
	        "elected_id": 9,
	        "election_time": 10,
	        "nb_messages": 32
	    },
	    {
	        "elected_id": 9,
	        "election_time": 10,
	        "nb_messages": 30
	    }
	]
	}


We can observe that the election took 10 rounds in every of the 10 tests, and that the same identifier was elected.
