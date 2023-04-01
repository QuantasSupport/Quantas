/*
Copyright 2022
This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/


#include "CycleOfTreesPeer.hpp"

namespace quantas {

	int CycleOfTreesPeer::noOfEdges                = 0;
	int CycleOfTreesPeer::noOfCycleNodes           = 0;
	int                   numberOfNodes            = 0;
	vector<list<int>>     allEdges;
	vector<list<int>>     unusedEdges;
	vector<list<int>>     presentEdges;
	double                avgKnotOutputNumerator   = 0;
	double                avgKnotOutputDenominator = 0;
	bool                  firstDetected            = false;

	CycleOfTreesPeer::~CycleOfTreesPeer() {}

	CycleOfTreesPeer::CycleOfTreesPeer(const CycleOfTreesPeer& rhs) : Peer<CycleOfTreesMessage>(rhs) {

	}

	CycleOfTreesPeer::CycleOfTreesPeer(long id) : Peer(id) {

	}

	void CycleOfTreesPeer::performComputation() {
		checkInStrm();
		sendMessage();
	}

	void CycleOfTreesPeer::initParameters(const vector<Peer<CycleOfTreesMessage>*>& _peers, json parameters) {
		const vector<CycleOfTreesPeer*> peers = reinterpret_cast<vector<CycleOfTreesPeer*> const&>(_peers);

		// numberOfEdges = number of edges per round
		// cycleSize = number of nodes in knot/cycle
		int cycleSize = noOfCycleNodes = parameters["cycleSize"];
		int numberOfEdges = noOfEdges = parameters["numberOfEdges"];

		if (numberOfEdges > peers.size() || numberOfEdges <= 0) {
			cout << "invalid input for number of edges; must be in interval [1, network size]" << endl;
		}

		if (cycleSize > peers.size() || cycleSize <= 1) {
			cout << "invalid input for cycle size; must be in interval [2, network size]" << endl;
		}

		// create cycle
                for (int i = 1; i < cycleSize; ++i) {
			allEdges.push_back(list<int>{(i - 1), i});
                }
		allEdges.push_back(list<int>{(cycleSize - 1), 0});

                // create random trees
		numberOfNodes = _peers.size();
                int positionedPeerID = 0;
                for (int i = cycleSize; i < numberOfNodes; ++i) {
			positionedPeerID = uniformInt(0, (i - 1));    // interval: [0, i - 1]
			allEdges.push_back(list<int>{positionedPeerID, i});
                }

		unusedEdges = allEdges;

		pickEdges();
	}

	void CycleOfTreesPeer::endOfRound(const vector<Peer<CycleOfTreesMessage>*>& _peers) {
		const vector <CycleOfTreesPeer*> peers = reinterpret_cast<vector<CycleOfTreesPeer*> const&>(_peers);

		pickEdges();

		if (firstDetected == false) {
			for(auto it = peers.begin(); it != peers.end(); ++it) {
					if ((*it)->highestID != -1) {
						cout << "Number of allowed edges is " << noOfEdges << " and knot size is " << noOfCycleNodes << endl;
						cout << "\t Number of rounds till the cycle is first detected: " << (getRound() + 1) << endl;    // plus one since QUANTAS starts at round 0
						firstDetected = true;
						break;
					}
			}
		}

		if (lastRound()) {
			if (!firstDetected) {
				cout << "No node detected the knot; try a longer computation length." << endl;
			}

			firstDetected = false;
			
			avgKnotOutputNumerator   = 0;
			avgKnotOutputDenominator = 0;
			numberOfNodes            = 0;

			allEdges.clear();
			unusedEdges.clear();
			presentEdges.clear();

			/*cout << "Highest ID is: ";    // testing every node has detected the same highest ID
			std::for_each(peers.begin(), peers.end(),
				[](const CycleOfTreesPeer* peer) {
					cout << peer->highestID << "  ";
				});*/

			cout << endl << endl;
		}
	}

	void CycleOfTreesPeer::checkInStrm() {    // check messages
		if (highestID == -1) {
			while (!inStreamEmpty()) {
				Packet<CycleOfTreesMessage> newMsg = popInStream();

				set<int> messageSet = newMsg.getMessage().nodesMessageHasReached;

				if (messageSet.find(id()) != messageSet.end()) {    // if message contains the node's ID, then the cycle was found!
					setHighestID(*(messageSet.rbegin()));    // sets are ordered, so the last element is the highest ID
				}

				if (newMsg.getMessage().highestIdInKnot != -1) {    // someone else has detected the knot!
					setHighestID(newMsg.getMessage().highestIdInKnot);
				}

				else if (!messageSet.empty()) {    // add IDs from message to the set of IDs heard from
					std::for_each(messageSet.begin(), messageSet.end(), [&](int id) {
						nodesHeardFrom.insert(id);
						});
				}
			}
		}
	}

	void CycleOfTreesPeer::sendMessage() {
		CycleOfTreesMessage message;

		if (highestID == -1) {   // the cycle has not been detected yet
			message.nodesMessageHasReached = nodesHeardFrom;
			std::for_each(presentEdges.begin(), presentEdges.end(), [=](list<int> edge) {
				if (edge.front() == id()) {
					unicastTo(message, edge.back());
				}
				});
		}

		else {    // the cycle has been detected
			message.highestIdInKnot = highestID;
			std::for_each(presentEdges.begin(), presentEdges.end(), [=](list<int> edge) {
				if (edge.front() == id()) {
					unicastTo(message, edge.back());
				}
				});
		}
	}

	// NOTE: If the total number of edges in the backbone topology, n, is not divisible by the number of allowed edges per state, m,
	// then every [floor(n/m) + 1] rounds there will only be [n – (floor(n/m)*m)] edge(s).
	void CycleOfTreesPeer::pickEdges() {

		presentEdges.clear();

		if (unusedEdges.empty()) {
			unusedEdges = allEdges;
		}

		std::shuffle(unusedEdges.begin(), unusedEdges.end(), RANDOM_GENERATOR);

		int i = 0;
		auto it = unusedEdges.begin();
		while (i < noOfEdges) {
			if (it == unusedEdges.end()) {
				break;
			}

			presentEdges.push_back(std::move(*it));
			it = unusedEdges.erase(it);
			++i;
		}
	}

	void CycleOfTreesPeer::setHighestID(int ID) {
		highestID = ID;
		avgKnotOutputNumerator += (getRound() + 1);    // plus one since QUANTAS starts at round 0

		if (avgKnotOutputDenominator == numberOfNodes - 1) {
			cout << "\t Number of rounds till the cycle is last detected: " << (getRound() + 1) << endl;    // plus one since QUANTAS starts at round 0
		}

		++avgKnotOutputDenominator;

		if (avgKnotOutputDenominator == numberOfNodes) {
			cout << "\t Average number of rounds for node to detect knot: " << avgKnotOutputNumerator / avgKnotOutputDenominator << endl;
		}
	}

	ostream& CycleOfTreesPeer::printTo(ostream& out) const {
		Peer<CycleOfTreesMessage>::printTo(out);

		out << id() << endl;
		out << "counter:" << getRound() << endl;

		return out;
	}

	ostream& operator<< (ostream& out, const CycleOfTreesPeer& peer) {
		peer.printTo(out);
		return out;
	}

	Simulation<quantas::CycleOfTreesMessage, quantas::CycleOfTreesPeer>* generateSim() {
		Simulation<quantas::CycleOfTreesMessage, quantas::CycleOfTreesPeer>* sim = new Simulation<quantas::CycleOfTreesMessage, quantas::CycleOfTreesPeer>;
		return sim;
	}

}
