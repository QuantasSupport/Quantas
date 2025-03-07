/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/
//
// This class handles reading in a configuration file, setting up log files for the simulation, 
// initializing the network class, and repeating a simulation according to the configuration file 
// (i.e., running multiple experiments with the same configuration).  It is templated with a user 
// defined message and peer class, used for the underlaying network instance. 

#ifndef Simulation_hpp
#define Simulation_hpp

#include <chrono>
#include <thread>
#include <fstream>

#include "Network.hpp"
#include "LogWriter.hpp"
#include "BS_thread_pool.hpp"


using std::ofstream;
using std::thread;

namespace quantas {

	class Simulation {
	private:
		Network system;  // use your new non-templated Network
	public:
		inline void run(json config);
	};

	inline void Simulation::run(json config) {
		ofstream out;
		if (config["logFile"] == "cout") {
			LogWriter::instance()->setLog(cout); // Set the log file to the console
		}
		else {
			string file = config["logFile"];
			out.open(file);
			if (out.fail()) {
				cout << "Error: could not open file " << file << ". Writing to console" << endl;
				LogWriter::instance()->setLog(cout); // If the file doesn't open set the log file to the console
			}
			else {
				LogWriter::instance()->setLog(out); // Otherwise set the log file to the user given file
			}
		}

		std::chrono::time_point<std::chrono::high_resolution_clock> startTime, endTime; // chrono time points
   		std::chrono::duration<double> duration; // chrono time interval
		startTime = std::chrono::high_resolution_clock::now();

		int _threadCount = thread::hardware_concurrency(); // By default, use as many hardware cores as possible
		if (config.contains("threadCount") && config["threadCount"] > 0) {
			_threadCount = config["threadCount"];
		}
		if (_threadCount > config["topology"]["initialPeers"]) {
			_threadCount = config["topology"]["initialPeers"];
		}
		int networkSize = static_cast<int>(config["topology"]["initialPeers"]);
		
		BS::thread_pool pool(_threadCount);
		for (int i = 0; i < config["tests"]; i++) {
			LogWriter::instance()->setTest(i);
			RoundManager::instance()->setCurrentRound(0);
			RoundManager::instance()->setLastRound(config["rounds"]);
			// Configure the delay properties and initial topology of the network
			system.setDistribution(config["distribution"]);
			system.initNetwork(config["topology"]);
			if (config.contains("parameters")) {
				system.initParameters(config["parameters"]);
			}
			
			//cout << "Test " << i + 1 << endl;
			for (int j = 0; j < config["rounds"]; j++) {
				//cout << "ROUND " << j << endl;
				RoundManager::instance()->incrementRound();

				// do the receive phase of the round
				BS::multi_future<void> receive_loop = pool.parallelize_loop(networkSize, [this](int a, int b){system.receive(a, b);});
				receive_loop.wait();

				BS::multi_future<void> compute_loop = pool.parallelize_loop(networkSize, [this](int a, int b){system.performComputation(a, b);});
				compute_loop.wait();

				system.endOfRound(); // do any end of round computations
			}
		}
		
		endTime = std::chrono::high_resolution_clock::now();
   		duration = endTime - startTime;
		LogWriter::instance()->data["RunTime"] = duration.count();

		LogWriter::instance()->print();
		out.close();
	}

	
}

#endif /* Simulation_hpp */
