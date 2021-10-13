#ifndef Simulation_hpp
#define Simulation_hpp

#include "./../Common/Network.hpp"
#include "./../Common/Peer.hpp"

using std::ofstream;

namespace blockguard {

	template<class type_msg, class peer_type>
	class Simulation {
	public:
		// Name of log file, will have Test number appended
		void setLog(string) { _log = log; };
		void setToRandom() { system.setToRandom(); };
		void setMaxDelay(int max) { system.setMaxDelay(max); };
		void setInitialPeers(int initialPeers) { _initialPeers = initialPeers; };
		void setTotalPeers(int totalPeers) { _totalPeers = totalPeers; };
		void run(int tests, int rounds);

	private:
		Network<ExampleMessage, ExamplePeer> system;
		string _log;
		int _initialPeers = 10;
		int _totalPeers = 10;
	};

	template<class type_msg, class peer_type>
	void Simulation<type_msg, peer_type>::run(int tests, int rounds) {
		for (int i = 0; i < tests; i++) {
			ofstream out;
			string file = _log + "Test" + i + ".log";
			out.open(file);

			system.setLog(out);
			system.initNetwork(_totalPeers);
			for (int j = 0; j < rounds; j++) {
				system.receive(); // do the receive phase of the round
				system.log(); // log the system state
				system.preformComputation();  // do the preform computation phase of the round
				system.log();
				system.transmit(); // do the transmit phase of the round
				system.log();
			}
		}
	}

}

#endif