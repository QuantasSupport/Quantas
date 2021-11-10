#ifndef Simulation_hpp
#define Simulation_hpp

#include "Network.hpp"
#include "Peer.hpp"
#include "./Common/Json.hpp"

using std::ofstream;
using nlohmann::json;

namespace blockguard {
	template<class type_msg, class peer_type>
	class Simulation {
	public:
		// Name of log file, will have Test number appended
		void run(json);

	private:
		Network<type_msg, peer_type> system;

		ofstream* getLog() { system.setLog; }
	};

	template<class type_msg, class peer_type>
	void Simulation<type_msg, peer_type>::run(json config) {
		for (int i = 1; i <= config["tests"]; i++) {
			ofstream out;
			
			if (config["logFile"] == "cout") {
				//out = &cout;
				system.setLog(cout);
			}
			else {
				string file = config["logFile"];
				file += "Test" + std::to_string(i) + ".log";
				out.open(file);
				if (out.fail()) {
					cout << "Error: could not open file " << file << std::endl;
					system.setLog(cout);
				}
				else {
					system.setLog(out);
				}
				
			}
			
			system.setDistribution(config["distribution"]);

			system.initNetwork(config["topology"]);
			*system.getLog() << "-- STARTING Test " << i << " --" << std::endl; // write in the log when the test started
			for (int j = 1; j <= config["rounds"]; j++) {
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