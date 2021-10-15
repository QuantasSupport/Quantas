#ifndef Simulation_hpp
#define Simulation_hpp

#include "Network.hpp"
#include "Peer.hpp"

using std::ofstream;

namespace blockguard {
	template<class type_msg, class peer_type>
	class Simulation {
	public:
		// Name of log file, will have Test number appended
		void setLog(string log) { _log = log; };
		ofstream* getLog() { system.setLog; }
		void setToUniform() { system.setToUniform(); };
		void setMaxDelay(int max) { system.setMaxDelay(max); };
		void setInitialPeers(int initialPeers) { _initialPeers = initialPeers; };
		void setTotalPeers(int totalPeers) { _totalPeers = totalPeers; };
		void run(int tests, int rounds);

	private:
		Network<type_msg, peer_type> system;
		string _log;
		int _initialPeers = 10;
		int _totalPeers = 10;
	};

	template<class type_msg, class peer_type>
	void Simulation<type_msg, peer_type>::run(int tests, int rounds) {
		for (int i = 0; i < tests; i++) {
			ofstream out;
			if (_log == "cout") {
				//out = &cout;
				system.setLog(cout);
			}
			else {
				
				string file = _log + "Test" + std::to_string(i) + ".log";
				out.open(file);
				if (out.fail()) {
					cout << "Error: could not open file " << file << std::endl;
					system.setLog(cout);
				}
				else {
					system.setLog(out);
				}
				
			}
			
			system.initNetwork(_totalPeers);
			*system.getLog() << "-- STARTING Test " << i << " --" << std::endl; // write in the log when the test started
			for (int j = 0; j < rounds; j++) {
				out << "-- STARTING ROUND " << j << " --" << std::endl; // write in the log when the round started
				system.receive(); // do the receive phase of the round
				system.log(); // log the system state
				system.performComputation();  // do the perform computation phase of the round
				system.log();
				system.transmit(); // do the transmit phase of the round
				system.log();
				*system.getLog() << "-- ENDING ROUND " << j << " --" << std::endl; // log the end of a round
			}

			*system.getLog() << "Number of Messages: " << system.getMessageCount() << std::endl;
			*system.getLog() << "-- ENDING Test " << i << " --" << std::endl; // write in the log when the test ends
		}
	}

}

#endif