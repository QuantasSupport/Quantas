//
//  main.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <set>
#include "Blockchain.hpp"
#include "Peer.hpp"
#include "ExamplePeer.hpp"
#include "PBFT_Peer.hpp"
#include "syncBFT_Peer.hpp"
#include "Network.hpp"
#include "bCoin_Peer.hpp"
#include "PBFTPeer_Sharded.hpp"
#include "PBFTReferenceCommittee.hpp"
#include "syncBFT_Committee.hpp"
#include "Logger.hpp"
#include "DS_bCoin_Peer.hpp"
#include "bCoin_Committee.hpp"
#include <iostream>
#include <chrono>
#include <random>


const int peerCount = 10;
const int blockChainLength = 100;
Blockchain *blockchain;
int shuffleByzantineInterval = 0;
static const int SPBFT_PEER_COUNT = 256;
static const int SPBFT_GROUP_SIZE = 8;
static const double FAULT = 0.3;
static const int NUMBER_OF_ROUNDS = 1000;
static const int NUMBER_OF_RUNS = 2;
const static std::string s_pbft_header = "Total Request:,Max Ledger:,Ratio Defeated To Honest 1,Ratio Defeated To Honest 2,Ratio Defeated To Honest 3,Ratio Defeated To Honest 4,Ratio Defeated To Honest 5,Average Waiting Time 1,Average Waiting Time 2,Average Waiting Time 3 ,Average Waiting Time 4,Average Waiting Time 5, total honest 1, total honest 2, total honest 3, total honest 4, total honest 5, total defeated 1, total defeated 2, total defeated 3, total defeated 4, total defeated 5\n";

// util functions
void buildInitialChain(std::vector<std::string>);
std::set<std::string> getPeersForConsensus(int);
int getNumberOfConfimedTransactionsBGS_PBFT(const std::vector<PBFTPeer_Sharded>&);
int sumMessagesSentBGS(const PBFTReferenceCommittee&);
void calculateResults(const PBFTReferenceCommittee system,std::ofstream &csv);
double ratioOfSecLvl(std::vector<std::pair<PBFT_Message,int> > globalLedger, double secLvl);
double waitTimeOfSecLvl(std::vector<std::pair<PBFT_Message,int> > globalLedger, double secLvl);
double waitTime(std::vector<std::pair<PBFT_Message,int> > globalLedger);
int totalNumberOfDefeatedCommittees(std::vector<std::pair<PBFT_Message,int> > globalLedger, double secLvl);
int totalNumberOfCorrectCommittees(std::vector<std::pair<PBFT_Message,int> > globalLedger, double secLvl);
int defeatedTrnasactions(std::vector<std::pair<PBFT_Message,int> > globalLedger);

void Example(std::ofstream &logFile);
void PBFT(std::ofstream &out,int);
void syncBFT(const char ** argv);
void bitcoin(std::ofstream &,int );
void DS_bitcoin(const char ** argv);

void Sharded_PBFT(std::ofstream &csv, std::ofstream &log,int,double);

void MOTIVATIONAL11_Sharded_PBFT(std::ofstream &csv, std::ofstream &log);
void MOTIVATIONAL12_Sharded_PBFT(std::ofstream &csv, std::ofstream &log);
void PARAMETER1_Sharded_PBFT(std::ofstream &csv, std::ofstream &log);
void PARAMETER2_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, int delay);
void ADAPTIVE11_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, int delay);
void ADAPTIVE12_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, double byzantine);
void ADAPTIVE21_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, int delay);
void ADAPTIVE22_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, double byzantine);
void ADAPTIVE3_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, double byzantine);

int main(int argc, const char * argv[]) {
    srand((float)time(NULL));
    if(argc < 3){
        std::cerr << "Error: need algorithm and output path" << std::endl;
        return 0;
    }


    std::string algorithm = argv[1];
    std::string filePath = argv[2];

    if(algorithm == "example"){
        std::ofstream out;
        std::string file = filePath + "/example.log";
        out.open(file);
        if ( out.fail() ){
            std::cerr << "Error: could not open file"<< file  << std::endl;
        }
        Example(out);
    }
    else if (algorithm== "pbft"){
        for(int delay = 1; delay < 10; delay++){
            std::cout<< "PBFT"<<std::endl;
            std::ofstream out;
            out.open(filePath + "/PBFT_Delay"+std::to_string(delay) + ".log");
            if ( out.fail() ){
                std::cerr << "Error: could not open file" << std::endl;
            }
            for(int run = 0; run < 10; run++){
                PBFT(out,delay);
            }
            out.close();
            if(delay == 1){
                delay = 0;
            }
        }

    }else if (algorithm == "syncBFT") {
		//	Program arguments: syncBFT fileName 2 1 1 128 100 0.3 1
		std::ofstream out;
		int runs = std::stoi(argv[3]);
		for(int i = 0; i< runs; i++){
			syncBFT(argv);
		}
	}else if (algorithm == "pbft_s") {
        std::cout<< "pbft_s"<<std::endl;
        std::ofstream csv;
        std::ofstream log;
        log.open(filePath + "pbft_s.log");
        if ( log.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".log" << std::endl;
        }

        csv.open(filePath + "MOTIVATIONAL11_Sharded_PBFT.csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        MOTIVATIONAL11_Sharded_PBFT(csv,log);
        csv.close();

        csv.open(filePath + "MOTIVATIONAL12_Sharded_PBFT.csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        MOTIVATIONAL12_Sharded_PBFT(csv,log);
        csv.close();

        csv.open(filePath + "PARAMETER1_Sharded_PBFT.csv");
        if ( csv.fail() ){
            std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
        }
        PARAMETER1_Sharded_PBFT(csv, log);
        csv.close();

        for(int delay = 1; delay < 11; delay = delay + 2){
            csv.open(filePath + "PARAMETER2_Sharded_PBFT_" + std::to_string(delay) + ".csv");
            if ( csv.fail() ){
                std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
            }
            PARAMETER2_Sharded_PBFT(csv, log, delay);
            csv.close();
        }

        for(int delay = 1; delay < 11; delay = delay + 2){
            csv.open(filePath + "ADAPTIVE11_Sharded_PBFT_" + std::to_string(delay) + ".csv");
            if ( csv.fail() ){
                std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
            }
            ADAPTIVE11_Sharded_PBFT(csv, log, delay);
            csv.close();
            csv.open(filePath + "ADAPTIVE21_Sharded_PBFT_" + std::to_string(delay) + ".csv");
            if ( csv.fail() ){
                std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
            }
            ADAPTIVE21_Sharded_PBFT(csv, log, delay);
            csv.close();
        }

        for(double byz = 0; byz < 1; byz = byz + 0.2){
            csv.open(filePath + "ADAPTIVE12_Sharded_PBFT_" + std::to_string(byz) + ".csv");
            if ( csv.fail() ){
                std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
            }
            ADAPTIVE12_Sharded_PBFT(csv, log, byz);
            csv.close();
            csv.open(filePath + "ADAPTIVE22_Sharded_PBFT_" + std::to_string(byz) + ".csv");
            if ( csv.fail() ){
                std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
            }
            ADAPTIVE22_Sharded_PBFT(csv, log, byz);
            csv.close();
        }

        for(double byz = 0; byz < 1; byz = byz + 0.2){
            csv.open(filePath + "ADAPTIVE3_Sharded_PBFT_" + std::to_string(byz) + ".csv");
            if ( csv.fail() ){
                std::cerr << "Error: could not open file: "<< filePath + ".csv" << std::endl;
            }
            ADAPTIVE3_Sharded_PBFT(csv,log,byz);
            csv.close();
        }
        log.close();
    }else if (algorithm == "bitcoin") {
        std::ofstream out;
        bitcoin(out, 1);
    }else if (algorithm == "DS_bitcoin") {
		//	Program arguments: DS_bitcoin asdf 1 1 1 64 100 0.7 1
		std::ofstream out;
		int runs = std::stoi(argv[3]);
		for(int i = 0; i< runs; i++){
			DS_bitcoin(argv);
		}
    }

    return 0;
}

void Example(std::ofstream &logFile){
    ByzantineNetwork<ExampleMessage,ExamplePeer> system;
    system.setLog(logFile); // set the system to write log to file logFile
    system.setToRandom(); // set system to use a uniform random distribution of weights on edges (channel delays)
    system.setMaxDelay(3); // set the max weight an edge can have to 3 (system will now pick randomly between [1, 3])
    system.initNetwork(5); // Initialize the system (create it) with 5 peers given the above settings

    for(int i =0; i < 3; i++){
       logFile<< "-- STARTING ROUND "<< i<< " --"<<  std::endl; // write in the log when the round started

        system.receive(); // do the receive phase of the round
        //system.log(); // log the system state
        system.preformComputation();  // do the preform computation phase of the round
        //system.log();
        system.transmit(); // do the transmit phase of the round
        //system.log();

        logFile<< "-- ENDING ROUND "<< i<< " --"<<  std::endl; // log the end of a round
    }

    system = ByzantineNetwork<ExampleMessage,ExamplePeer>(); // clear old setup by creating a fresh object
    system.setLog(std::cout); // set the system to write log to terminal
    system.setToRandom();
    system.setMaxDelay(10);
    system.initNetwork(3);

    for(int i =0; i < 3; i++){
        std::cout<< "-- STARTING ROUND "<< i<< " --"<<  std::endl; // print outwhen the round started

        system.receive();
        system.log(); // log now goes to the terminal
        system.preformComputation();
        system.log();
        system.transmit();
        system.log();

        std::cout<< "-- ENDING ROUND "<< i<< " --"<<  std::endl; // print out the end of a round
    }

    // The base peer class tracks number of messages sent by a(1) peer. To calculate the total number of messages that where in the system we need
    //      to add up each peers indivudal message count (example of looping thouth the Network class)

    int numberOfMessages = 0;
    for(int i = 0; i < system.size(); i++){
        numberOfMessages += system[i]->getMessageCount(); // notice that the index operator ([]) return a pointer to a peer. NEVER DELETE THIS PEER INSTANCE.
                                                          //    The netwrok class deconstructor will get ride off ALL peer instances once it is deconstructed.
                                                          //    Use the -> to call method on the peer instance. The Network class will also cast the instance to
                                                          //    your derived class so all methods that you add will be avalable via the -> operator
    }
    std::cout<< "Number of Messages: "<< numberOfMessages<< std::endl;
}

void PBFT(std::ofstream &out,int delay){
    ByzantineNetwork<PBFT_Message, PBFT_Peer> system;
    system.setLog(out);
    system.setToRandom();
    system.setMaxDelay(delay);
    system.initNetwork(128);
    for(int i = 0; i < system.size(); i++){
        system[i]->setFaultTolerance(0.3);
        system[i]->init();
    }

    int numberOfRequests = 0;
    for(int i =-1; i < 1000; i++){
        for(int i = 0; i < system.size(); i++){
            if(system[i]->isPrimary()){
                system.makeRequest(i);
                numberOfRequests++;
                break;
            }
        }
        system.receive();
        std::cout<< 'r'<< std::flush;
        system.preformComputation();
        std::cout<< 'p'<< std::flush;
        system.transmit();
        std::cout<< 't'<< std::flush;
        system.log();
    }

    int min = (int)system[0]->getLedger().size();
    int max = (int)system[0]->getLedger().size();
    int totalMessages = system[0]->getMessageCount();
    for(int i = 0; i < system.size(); i++){
        //out<< "Peer ID:"<< system[i].id() << " Ledger Size:"<< system[i].getLedger().size()<< std::endl;
        if(system[i]->getLedger().size() < min){
            min = (int)system[i]->getLedger().size();
        }
        if(system[i]->getLedger().size() > max){
            max = (int)system[i]->getLedger().size();
        }
        totalMessages += system[i]->getMessageCount();
    }
    out<< "Min Ledger:,"<< min<< std::endl;
    out<< "Max Ledger:,"<< max<< std::endl;
    out<< "Total Messages:,"<< totalMessages<< std::endl;
    out<< "Total Request:,"<< numberOfRequests<<std::endl;
    //std::cout<< std::endl;
}

void bitcoin(std::ofstream &out, int avgDelay){
    ByzantineNetwork<bCoinMessage, bCoin_Peer> n;
    n.setToPoisson();
    n.setAvgDelay(avgDelay);
    n.setLog(std::cout);
    n.initNetwork(10);

    //mining delays at the beginning
    for(int i = 0; i<n.size(); i++){
        n[i]->setMineNextAt( bCoin_Peer::distribution(bCoin_Peer::generator) );
    }

    for(int i = 1; i<100; i++){
        std::cerr<<"Iteration "<<i<<std::endl;
        n.receive();
        n.preformComputation();
        n.transmit();
    }
    int maxChain = 0;
    for(int i =0;i<n.size();i++){
        if(n[i]->getBlockchain()->getChainSize()>maxChain)
            maxChain = i;
    }
    std::cerr<<"Number of confirmations = "<<n[maxChain]->getBlockchain()->getChainSize()<<std::endl;
}

void DS_bitcoin(const char ** argv){
	std::string filePath	= 	argv[2];
	int avgDelay 			= 	std::stoi(argv[4]);
	int byzantineOrNot 		= 	std::stoi(argv[5]);
	int peersCount 			= 	std::stoi(argv[6]);
	int iterationCount 		= 	std::stoi(argv[7]);
	double tolerance 		= 	std::stod(argv[8]);
	int txRate 				= 	std::stoi(argv[9]);

	Logger::setLogFileName(filePath + "_"+std::to_string(std::chrono::system_clock::now().time_since_epoch().count())+"_"+argv[1]+"_delay"+std::to_string(avgDelay)+"_peerCount"+std::to_string(peersCount)
						   +"_iterationCount"+std::to_string(iterationCount)+"_tolerance"+std::to_string(tolerance)+"_txRate"+std::to_string(txRate)+".txt");

	someByzantineCount.clear();
	moreThanHalfByzantineCount.clear();
	Network<DS_bCoinMessage, DS_bCoin_Peer> n;
	n.setToRandom();
	n.setMaxDelay(avgDelay);
	n.setMinDelay(1);
	n.initNetwork(peersCount);
	n.setLog(std::cout);
	n.buildInitialDAG();

	std::vector<bCoin_Committee *> currentCommittees;
	std::deque<std::string> txQueue;

	//	for printing security levels
	std::vector<int> securityLevels;
	securityLevels.push_back(peersCount/16);
	securityLevels.push_back(peersCount/8);
	securityLevels.push_back(peersCount/4);
	securityLevels.push_back(peersCount/2);
	securityLevels.push_back(peersCount/1);

	vector<string> byzantinePeers;
	if(byzantineOrNot == 1){
		while(byzantinePeers.size()< tolerance*((double)peersCount)) {
			int index = rand()%peersCount;
			if (std::find(byzantinePeers.begin(),byzantinePeers.end(),n[index]->id())!= byzantinePeers.end()){
			}else{
				n[index]->setByzantineFlag(true);
				byzantinePeers.push_back(n[index]->id());
			}
		}
	}

	Logger::instance()->log("BYZANTINE PEER COUNT " + std::to_string(byzantinePeers.size()) + "\n");


	for(int i = 0; i<n.size(); i++){
		n[i]->setMineNextAt( DS_bCoin_Peer::distribution(DS_bCoin_Peer::generator) );
	}

	//	phase mining or collecting
	std::string Mining 		= "MINING";
	std::string Collecting 	= "COLLECTING";

	std::string status = Collecting;
	int collectInterval = 0;
	std::vector<vector<DS_bCoin_Peer*>> consensusGroups;
	std::vector<int> consensusGroupCount;
	std::map<int, int> securityLevelCount;
	std::map<int,int> defeatedCommittee;

	std::map<int, double> confirmationRate;
	//	initial block size is peersCount + 1
	int prevConfirmationSize = peersCount + 1;

	for(int i = 0; i<iterationCount; i++){

		if(i % 100 == 0){
			//	saturation point calculation, keep track of confirmed count, look at the dag of any peer to find the chain size. i.e. number of confirmed blocks
			//	number of transactions introduced will be 100/txRate
			confirmationRate[i] = ((double)(n[0]->dag.getSize() - prevConfirmationSize))/(100/(double)txRate);
			prevConfirmationSize = n[0]->dag.getSize();

		}

		Logger::instance()->log("----------------------------------------------------------Iteration " + std::to_string(i) + "\n");

		if( i%txRate == 0  ){
			//	create transactions every 5 iterations
			//	insert a transaction to queue
			txQueue.push_back("Tx_"+std::to_string(i));
		}

		if(status == Mining){
			for(auto & currentCommittee : currentCommittees){
				currentCommittee->receive();
				currentCommittee->performComputation();
				currentCommittee->transmit();
			}

			//	don't erase but check to see if consensus reached in all
			bool consensus = true;
			auto it = currentCommittees.begin();
			while(it != currentCommittees.end()) {
				if(!(*it)->getConsensusFlag()) {
					consensus = false;
					break;
				}
				++it;
			}
			if (consensus){
				//	send blocks
				for(auto & currentCommittee : currentCommittees){
					//	propogate the block to whole network except the committee itself.
					//	set neighbours
					std::vector<std::string> peerIds = currentCommittee->getCommitteePeerIds();
					std::map<std::string, Peer<DS_bCoinMessage>* > neighbours;

					DS_bCoin_Peer * minerPeer = currentCommittee->getCommitteePeers()[currentCommittee->getFirstMinerIndex()];

					for(int j = 0; j < n.size(); j++) {
						if(minerPeer->id()!=n[j]->id()) {
							if (std::find(peerIds.begin(), peerIds.end(), n[j]->id()) != peerIds.end()) {
							} else {
								neighbours[n[j]->id()] = n[j];
							}
						}
					}

					if(currentCommittee->size()!=n.size())
						assert(!neighbours.empty());
					minerPeer->setCommitteeNeighbours(neighbours);
					minerPeer->sendBlock();
					minerPeer->transmit();
				}
				for(auto committee: currentCommittees ){
					delete committee;
				}
				currentCommittees.clear();

				status =Collecting;

				collectInterval = 2 * avgDelay;
			}

		}else if(status == Collecting){
			//	make sure no peer is busy
			for(int a = 0; a< n.size();a++){
				assert(!n[a]->isBusy());
				assert(n[a]->getConsensusTx().empty());
			}

			assert (currentCommittees.empty());

			if(collectInterval > 0){
				n.receive();

				if(--collectInterval == 0){
					Logger::instance()->log("CALLING UPDATE DAG\n");
					for(int index = 0; index< n.size();index++){
						n[index]->updateDAG();
					}
				}else{
					//	skip to next duration
					continue;
				}

			}

			//	shuffling byzantines
			if(byzantineOrNot==1){
				Logger::instance()->log("Shuffling " + std::to_string(peersCount/10) + " Peers.\n");
				n.shuffleByzantines (peersCount/10);
				//	n.shuffleByzantines (1);
			}

			if(!txQueue.empty()){
				//	select a random peer
				int randIndex;
				DS_bCoin_Peer *p;

				std::vector<DS_bCoin_Peer*> consensusGroup;
				int concurrentGroupCount = 0;
				do{
					int securityLevel = n.pickSecurityLevel(peersCount);
					randIndex = rand()%n.size();

					p = n[randIndex];
					consensusGroup = n.setPeersForConsensusDAG(p, securityLevel);
					if(!consensusGroup.empty())
					//create a committee if only a consensus group is formed.
					if(!consensusGroup.empty()){
						Logger::instance()->log("COMMITTEE FORMED\n");
						bCoin_Committee *co = new bCoin_Committee(consensusGroup, p, txQueue.front(), securityLevel);
						concurrentGroupCount++;
						txQueue.pop_front();
						currentCommittees.push_back(co);
						consensusGroups.push_back(consensusGroup);
						securityLevelCount[securityLevel]++;
						if(co->getByzantineRatio()>=0.5){
							defeatedCommittee[securityLevel]++;
						}
					}
				}while(!consensusGroup.empty() && !txQueue.empty()); //build committees until a busy peer is jumped on.
				consensusGroupCount.push_back(concurrentGroupCount);
			}


			Logger::instance()->log("DONE WITH CONSENSUS GROUP FORMING.\n");

			status = Mining;
			Logger::instance()->log("COLLECTION COMPLETE IN ITERATION " + std::to_string(i) + ":\t START MINING \n");

			if(!currentCommittees.empty()){
				for(auto &committee: currentCommittees){
					committee->initiate();
				}
				status = Mining;
			}

		}

	}
	Logger::instance()->log("FINALLY\n");
	for(int i =0; i<n.size();i++){
		Logger::instance()->log("PEER " + std::to_string(i) + " DAG SIZE IS " + std::to_string(n[i]->dag.getSize()) + "\n");
	}

	Logger::instance()->log("SOME BYZANTINE COUNT \n");
	std::map<int,int> someByzantineDups;

	for_each( someByzantineCount.begin(), someByzantineCount.end(), [&someByzantineDups]( int val ){ someByzantineDups[val]++; } );
	for(auto l : securityLevels){
		if(someByzantineDups.count(l)){
			Logger::instance()->log(std::to_string(l) + " " + std::to_string(someByzantineDups[l]) + "\n");
		} else
			Logger::instance()->log(std::to_string(l) + " 0\n");
	}

	Logger::instance()->log("MORE THAN HALF BYZANTINE COUNT \n");
	std::map<int,int> moreThanHalfByzantineDups;

	for_each( moreThanHalfByzantineCount.begin(), moreThanHalfByzantineCount.end(), [&moreThanHalfByzantineDups]( int val ){ moreThanHalfByzantineDups[val]++; } );

	for(auto l : securityLevels){
		if(moreThanHalfByzantineDups.count(l)){
			Logger::instance()->log(std::to_string(l) + " " + std::to_string(moreThanHalfByzantineDups[l]) + "\n");
		} else
			Logger::instance()->log(std::to_string(l) + " 0\n");
	}

	Logger::instance()->log("CONSENSUS GROUP COUNT \n");
	for(auto i :consensusGroupCount){
		Logger::instance()->log(std::to_string(i) + " ");
	}

	Logger::instance()->log("SECURITY LEVELS CHOSEN\n");

	for(auto l : securityLevels){
		if(securityLevelCount.count(l)){
			Logger::instance()->log(std::to_string(l) + " " + std::to_string(securityLevelCount[l]) + "\n");
		} else
			Logger::instance()->log(std::to_string(l) + " 0\n");
	}

	Logger::instance()->log("DEFEATED COMMITTEES COUNT\n");

	for(auto l : securityLevels){
		if(defeatedCommittee.count(l)){
			Logger::instance()->log(std::to_string(l) + " " + std::to_string(defeatedCommittee[l]) + "\n");
		} else
			Logger::instance()->log(std::to_string(l) + " 0\n");
	}

	Logger::instance()->log("RATIO OF DEFEATED COMMITTEES\n");

	for(auto l : securityLevels){
		if(defeatedCommittee.count(l)){
			Logger::instance()->log(std::to_string(l) +  "" + std::to_string((double)defeatedCommittee[l]/securityLevelCount[l]) + "\n");
		} else
			Logger::instance()->log(std::to_string(l) + " 0\n");
	}

	Logger::instance()->log("FRACTION OF BAD BLOCKS\n");
	for(auto l : securityLevels){
		if(moreThanHalfByzantineDups.count(l)){
			Logger::instance()->log(std::to_string(l) + " " + std::to_string(float(moreThanHalfByzantineDups[l])/securityLevelCount[l]*100) + "\n");
		} else
			Logger::instance()->log(std::to_string(l) + " 0");
	}

	Logger::instance()->log("CONFIRMATION RATE\n");
	for(auto cr : confirmationRate){
		Logger::instance()->log(std::to_string(cr.first) + ": " + std::to_string(cr.second) + "\n");
	}

	for(auto committee : currentCommittees){
		delete committee;
	}
}

void syncBFT(const char ** argv){

	std::string filePath	= 	argv[2];
	int avgDelay 			= 	std::stoi(argv[4]);
	int byzantineOrNot 		= 	std::stoi(argv[5]);
	int peersCount 			= 	std::stoi(argv[6]);
	int iterationCount 		= 	std::stoi(argv[7]);
	double tolerance 		= 	std::stod(argv[8]);
	int txRate 				= 	std::stoi(argv[9]);

	Logger::setLogFileName(filePath + "_"+std::to_string(std::chrono::system_clock::now().time_since_epoch().count())+"_"+argv[1]+"_delay"+std::to_string(avgDelay)+"_peerCount"+std::to_string(peersCount)
						   +"_iterationCount"+std::to_string(iterationCount)+"_tolerance"+std::to_string(tolerance)+"_txRate"+std::to_string(txRate)+".txt");

	Network<syncBFTmessage, syncBFT_Peer> n;
	n.setToRandom();
	n.setMaxDelay(avgDelay);
	n.setMinDelay(1);
	n.initNetwork(peersCount);
	n.buildInitialDAG();

	std::vector<syncBFT_Committee *> currentCommittees;
	std::deque<std::string> txQueue;

//	for printing security levels
	std::vector<int> securityLevels;
	securityLevels.push_back(peersCount/16);
	securityLevels.push_back(peersCount/8);
	securityLevels.push_back(peersCount/4);
	securityLevels.push_back(peersCount/2);
	securityLevels.push_back(peersCount/1);

	vector<string> byzantinePeers;
	if(byzantineOrNot == 1){
		while(byzantinePeers.size()< tolerance*((double)peersCount)) {
			int index = rand()%peersCount;
			if (std::find(byzantinePeers.begin(),byzantinePeers.end(),n[index]->id())!= byzantinePeers.end()){
			}else{
				n[index]->setByzantineFlag(true);
				byzantinePeers.push_back(n[index]->id());
			}
		}
	}

	//phase mining or collecting
	const std::string MINING 			= "MINING";
	const std::string COLLECTING 		= "COLLECTING";
	const std::string WAITING_FOR_TX 	= "WAITING_FOR_TX";

	std::string status = COLLECTING;
	int collectInterval = 0;
	std::vector<vector<syncBFT_Peer*>> consensusGroups;
	std::vector<int> consensusGroupCount;
	std::map<int, int> securityLevelCount;
	std::map<int, int> defeatedCommittee;
	std::map<int, double> confirmationRate;

	//	initial block size is peersCount + 1
	int prevConfirmationSize = peersCount + 1;
	int waitTime = 2 * n.maxDelay();

	for(int i = 0; i<iterationCount; i++){
		if(i % 100 == 0){
			//	saturation point calculation, keep track of confirmed count, look at the dag of any peer to find the chain size. i.e. number of confirmed blocks
			//	number of transactions introduced will be 100/txRate
			confirmationRate[i] = ((double)(n[0]->getDAG().getSize() - prevConfirmationSize))/(100);
			prevConfirmationSize = n[0]->getDAG().getSize();
		}

		if( i%txRate == 0  ){
			txQueue.push_back("Tx_"+std::to_string(i));
		}
		Logger::instance()->log("----------------------------------------------------------Iteration "+std::to_string(i)+"\n");
		if(status == WAITING_FOR_TX){
//			wait for max delay until all committees receive their transactions
			if(waitTime >=0 ){
				for(auto & currentCommittee : currentCommittees) {
					currentCommittee->receive();
				}
				waitTime--;
			}

			if(waitTime == 0){
				for(auto & currentCommittee : currentCommittees) {
					currentCommittee->receiveTx();
				}
				status = MINING;
				waitTime = 2*n.maxDelay();
			}

		}else if(status == MINING){
			waitTime--;

			for(auto & currentCommittee : currentCommittees){
				if(currentCommittee->getConsensusFlag()){
					continue;
				}
				currentCommittee->receive();
				currentCommittee->performComputation();
				if(waitTime == 0){
					currentCommittee->nextState(i, n.maxDelay());
					waitTime = 2*n.maxDelay();
				}
				currentCommittee->transmit();
			}

			//	don't erase but check to see if consensus reached in all
			bool consensus = true;
			auto it = currentCommittees.begin();
			while(it != currentCommittees.end()) {
				if(!(*it)->getConsensusFlag()) {
					consensus = false;
					break;
				}
				++it;
			}
			if (consensus){
				for(auto committee: currentCommittees ){
					committee->refreshPeers();
				}
				//	send blocks
				Logger::instance()->log("CONSENSUS REACHEDDDD \n");
				for(auto & currentCommittee : currentCommittees){
					//	propogate the block to whole network except the committee itself.
					std::vector<std::string> peerIds = currentCommittee->getCommitteePeerIds();
					std::map<std::string, Peer<syncBFTmessage>* > neighbours;


					//	selecting the first peer in the committee, as all peers agree on the same block.
					syncBFT_Peer * minerPeer = currentCommittee->getCommitteePeers()[0];

					for(int j = 0; j < n.size(); j++) {
						if(minerPeer->id()!=n[j]->id()) {
							if (std::find(peerIds.begin(), peerIds.end(), n[j]->id()) != peerIds.end()) {
							} else {
								neighbours[n[j]->id()] = n[j];
							}
						}
					}
					if(currentCommittee->size()!=n.size())
						assert(!neighbours.empty());
					minerPeer->setCommitteeNeighbours(neighbours);
					minerPeer->sendBlock();
					Logger::instance()->log("TRANSMITTING MINED BLOCK FROM PEER "+ minerPeer->id() + "\n");
					minerPeer->transmit();
				}
				for(auto committee: currentCommittees ){
					delete committee;
				}
				currentCommittees.clear();

				Logger::instance()->log("CONSENSUS REACHED IN ALL COMMITTEES, STARTING COLLECTION IN ITERATION "+std::to_string(i)+"\n");
				status =COLLECTING;
				collectInterval = 2 * n.maxDelay();
			}
		}else if(status == COLLECTING){
			//	make sure no peer is busy
			for(int a = 0; a< n.size();a++){
				assert(n[a]->isTerminated());
				assert(n[a]->getConsensusTx().empty());
			}
			assert (currentCommittees.empty());

			if(collectInterval > 0){
				n.receive();

				if(--collectInterval == 0){
					Logger::instance()->log("UPDATING THE DAG\n");
					for(int index = 0; index< n.size();index++){
						n[index]->updateDAG();
					}
				}else{
					//	collection is not complete yet
					continue;
				}
			}

			//	shuffling byzantines
			if(byzantineOrNot==1){
				Logger::instance()->log("Shuffling "+std::to_string(peersCount/10)+" Peers.\n");
				n.shuffleByzantines (peersCount/10);
			}

			if(!txQueue.empty()){
				//	select a random peer
				int randIndex;
				syncBFT_Peer *p;

				std::vector<syncBFT_Peer*> consensusGroup;
				int concurrentGroupCount = 0;
				do{
					int securityLevel = n.pickSecurityLevel(peersCount);
					randIndex = rand()%n.size();
					p = n[randIndex];
					consensusGroup = n.setPeersForConsensusDAG(p, securityLevel);

					//create a committee if only a consensus group is formed.
					if(!consensusGroup.empty()){
						syncBFT_Committee *co = new syncBFT_Committee(consensusGroup, p, txQueue.front(), securityLevel);
						if(co->getByzantineRatio()>= 0.5){
							co->refreshPeers();
							defeatedCommittee[securityLevel]++;
							txQueue.pop_front();
							delete co;

						}else{
							concurrentGroupCount++;
							txQueue.pop_front();
							currentCommittees.push_back(co);
							consensusGroups.push_back(consensusGroup);
							securityLevelCount[securityLevel]++;
						}

					}
				}while(!consensusGroup.empty() && !txQueue.empty()); //build committees until a busy peer is jumped on.

				consensusGroupCount.push_back(concurrentGroupCount);
			}

//			change status after waiting for all peers to get their transaction
			status = WAITING_FOR_TX;

			if(!currentCommittees.empty()){
				Logger::instance()->log("INITIATING " + std::to_string(currentCommittees.size()) + " COMMITTEES\n");
				for(auto &committee: currentCommittees){
					committee->initiate();
				}
//				change status after waiting for all peers to get their transaction
				status = WAITING_FOR_TX;
			}

		}

	}

	for(int i =0; i<n.size();i++){
		Logger::instance()->log("PEER " + std::to_string(i) + " DAG SIZE IS " + std::to_string(n[i]->getDAG().getSize())+"\n");
	}

	Logger::instance()->log("DEFEATED COMMITTEES COUNT\n");
	for(auto l : securityLevels){
		if(defeatedCommittee.count(l)){
			Logger::instance()->log(std::to_string(l) + " " + std::to_string(defeatedCommittee[l]) + "\n");
		} else
			Logger::instance()->log(std::to_string(l) + " 0 \n");
	}

	Logger::instance()->log("CONFIRMATION RATE\n");
	for(auto cr : confirmationRate){
		Logger::instance()->log(std::to_string(cr.first) + ": " + std::to_string(cr.second) + "\n");
	}

	for(auto committee : currentCommittees){
		delete committee;
	}

}

void Sharded_PBFT(std::ofstream &csv, std::ofstream &log,int delay, double fault){
    std::cout<< std::endl<< "########################### PBFT_Sharded ###########################"<< std::endl;
    csv<< ""<< std::endl;
    PBFTReferenceCommittee system = PBFTReferenceCommittee();
    system.setGroupSize(8);
    system.setToRandom();
    system.setMaxDelay(delay);
    system.setLog(log);
    system.setSquenceNumber(999);
    system.initNetwork(256);
    system.setFaultTolerance(0.3);
    system.makeByzantines(256*fault);

    int numberOfRequests = 0;
    int test = -1;
    for(int i =0; i < 1000; i++){
        system.makeRequest();numberOfRequests++;
        system.makeRequest();numberOfRequests++;
        system.makeRequest();numberOfRequests++;

         if(i%5 == 0){
             system.shuffleByzantines(256*fault);
         }

        system.receive();
        std::cout<< 'r'<< std::flush;
        system.preformComputation();
        std::cout<< 'p'<< std::flush;
        system.transmit();
        std::cout<< 't'<< std::flush;
        system.log();

        if(i%100 == 0){
            calculateResults(system,csv);
            if(test == system.getGlobalLedger().size()){
                system.printNetworkOn();
                system.log();
                system.printNetworkOff();
            }
            test = system.getGlobalLedger().size();
        }
    }
    calculateResults(system,csv);
}

//////////////////////////////////////////////
// util functions
//

void buildInitialChain(std::vector<std::string> peerIds) {
    std::cerr << "Building initial chain" << std::endl;
    srand((time(nullptr)));
    Blockchain *preBuiltChain = new Blockchain(true);
    int index = 1;                  //blockchain index starts from 1;
    int blockCount = 1;
    std::string prevHash = "genesisHash";

    while (blockCount < blockChainLength) {
        std::set<std::string> publishers;

        std::string peerId = peerIds[rand()%peerIds.size()];
        std::string blockHash = std::to_string(index) + "_" + peerId;
        publishers.insert(peerId);
        preBuiltChain->createBlock(index, prevHash, blockHash, publishers);
        prevHash = blockHash;
        index++;
        blockCount++;
    }

    std::cerr << "Initial chain build complete." << std::endl;
    std::cerr << "Prebuilt chain: " << std::endl;
    std::cerr << *preBuiltChain << std::endl;
    std::cerr << preBuiltChain->getChainSize() << std::endl;
    blockchain = preBuiltChain;


}

std::set<std::string> getPeersForConsensus(int securityLevel) {
    int numOfPeers = peerCount / securityLevel;
    std::set<std::string> peersForConsensus;
    int chainSize = blockchain->getChainSize ();
    int i = blockchain->getChainSize() - 1;             //i keeps track of index in the chain
    while (peersForConsensus.size() < numOfPeers) {
        std::string peerId = *(blockchain->getBlockAt(i).getPublishers()).begin();
        peersForConsensus.insert(peerId);
        //random value
        int randVal = rand()%peerCount;

        int skip = randVal;
        if ((i - skip) <= 0) {
            i = chainSize - i - skip;

        } else
            i = i - skip;
    }

    return peersForConsensus;
}

int sumMessagesSentBGS(const PBFTReferenceCommittee &system){
    int sum = 0;
    for(int i = 0; i < system.size(); i++){
        sum += system[i]->getMessageCount();
    }
    return sum;
}

double ratioOfSecLvl(std::vector<std::pair<PBFT_Message,int> > globalLedger, double secLvl){
    double total = globalLedger.size();
    double defeated = 0;
    for(auto entry = globalLedger.begin(); entry != globalLedger.end(); entry++){
        if(entry->second == secLvl){
            if(entry->first.defeated){
                defeated++;
            }
        }
    }

    return defeated/total;
}

double waitTimeOfSecLvl(std::vector<std::pair<PBFT_Message,int> > globalLedger, double secLvl){
    double sumOfWaitingTime = 0;
    double totalNumberOfTrnasactions = 0;
    for(auto entry = globalLedger.begin(); entry != globalLedger.end(); entry++){
        if(entry->second == secLvl){
            sumOfWaitingTime += entry->first.commit_round - entry->first.submission_round;
            totalNumberOfTrnasactions++;
        }
    }
    if(totalNumberOfTrnasactions == 0){
        return 0;
    }else{
        return sumOfWaitingTime/totalNumberOfTrnasactions;
    }
}

double waitTime(std::vector<std::pair<PBFT_Message,int> > globalLedger){
    double sumOfWaitingTime = 0;
    double totalNumberOfTrnasactions = 0;
    for(auto entry = globalLedger.begin(); entry != globalLedger.end(); entry++){
        sumOfWaitingTime += entry->first.commit_round - entry->first.submission_round;
        totalNumberOfTrnasactions++;
    }
    if(totalNumberOfTrnasactions == 0){
        return 0;
    }else{
        return sumOfWaitingTime/totalNumberOfTrnasactions;
    }
}

int totalNumberOfDefeatedCommittees(std::vector<std::pair<PBFT_Message,int> > globalLedger, double secLvl){
    int total = 0;
    for(auto entry = globalLedger.begin(); entry != globalLedger.end(); entry++){
        if(entry->second == secLvl){
            if(entry->first.defeated){
                total++;
            }
        }
    }
    return total;
}

int defeatedTrnasactions(std::vector<std::pair<PBFT_Message,int> > globalLedger){
    int defeated = 0;

    for(auto entry = globalLedger.begin(); entry != globalLedger.end(); entry++){
        if(entry->first.defeated){
            defeated++;
        }
    }

    return defeated;
}

int totalNumberOfCorrectCommittees(std::vector<std::pair<PBFT_Message,int> > globalLedger, double secLvl){
    int total = 0;
    for(auto entry = globalLedger.begin(); entry != globalLedger.end(); entry++){
        if(entry->second == secLvl){
            if(!entry->first.defeated){
                total++;
            }
        }
    }
    return total;
}

//"Total Request:,Max Ledger:,Ratio Defeated To Honest 1,Ratio Defeated To Honest 2,Ratio Defeated To Honest 3,Ratio Defeated To Honest 4,Ratio Defeated To Honest 5,Average Waiting Time 1,Average Waiting Time 2,Average Waiting Time 3 ,Average Waiting Time 4,Average Waiting Time 5, total honest 1, total honest 2, total honest 3, total honest 4, total honest 5, total defeated 1, total defeated 2, total defeated 3, total defeated 4, total defeated 5\n"
void calculateResults(const PBFTReferenceCommittee system, std::ofstream &csv){
    std::vector<std::pair<PBFT_Message,int> > globalLedger = system.getGlobalLedger();
    csv<<std::to_string(system.totalSubmissions());
    csv<< ","<< std::to_string(globalLedger.size());
    csv<< ","<< std::to_string(ratioOfSecLvl(globalLedger,system.securityLevel1()*system.getGroupSize()));
    csv<< ","<< std::to_string(ratioOfSecLvl(globalLedger,system.securityLevel2()*system.getGroupSize()));
    csv<< ","<< std::to_string(ratioOfSecLvl(globalLedger,system.securityLevel3()*system.getGroupSize()));
    csv<< ","<< std::to_string(ratioOfSecLvl(globalLedger,system.securityLevel4()*system.getGroupSize()));
    csv<< ","<< std::to_string(ratioOfSecLvl(globalLedger,system.securityLevel5()*system.getGroupSize()));

    csv<< ","<< std::to_string(waitTimeOfSecLvl(globalLedger,system.securityLevel1()*system.getGroupSize()));
    csv<< ","<< std::to_string(waitTimeOfSecLvl(globalLedger,system.securityLevel2()*system.getGroupSize()));
    csv<< ","<< std::to_string(waitTimeOfSecLvl(globalLedger,system.securityLevel3()*system.getGroupSize()));
    csv<< ","<< std::to_string(waitTimeOfSecLvl(globalLedger,system.securityLevel4()*system.getGroupSize()));
    csv<< ","<< std::to_string(waitTimeOfSecLvl(globalLedger,system.securityLevel5()*system.getGroupSize()));

    csv<< ","<< std::to_string(totalNumberOfCorrectCommittees(globalLedger,system.securityLevel1()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfCorrectCommittees(globalLedger,system.securityLevel2()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfCorrectCommittees(globalLedger,system.securityLevel3()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfCorrectCommittees(globalLedger,system.securityLevel4()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfCorrectCommittees(globalLedger,system.securityLevel5()*system.getGroupSize()));

    csv<< ","<< std::to_string(totalNumberOfDefeatedCommittees(globalLedger,system.securityLevel1()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfDefeatedCommittees(globalLedger,system.securityLevel2()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfDefeatedCommittees(globalLedger,system.securityLevel3()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfDefeatedCommittees(globalLedger,system.securityLevel4()*system.getGroupSize()));
    csv<< ","<< std::to_string(totalNumberOfDefeatedCommittees(globalLedger,system.securityLevel5()*system.getGroupSize()));
    csv<< std::endl;

}

//static const int SPBFT_PEER_COUNT = 256
//static const int SPBFT_GROUP_SIZE = 8
//static const double FAULT = 0.3

//void MOTIVATIONAL11_Sharded_PBFT(std::ofstream &csv, std::ofstream &log);
//void MOTIVATIONAL12_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, int committee_size);
//void PARAMETER_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, int delay);
//void ADAPTIVE11_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, int delay);
//void ADAPTIVE12_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, double byzantine);
//void ADAPTIVE21_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, int delay);
//void ADAPTIVE22_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, double byzantine);
//void ADAPTIVE3_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, double byzantine);

void MOTIVATIONAL11_Sharded_PBFT(std::ofstream &csv, std::ofstream &log){
    std::string header = "Committee Size, Defeated Transactions, Confirmed/Submitted";

    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(SPBFT_GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.setSquenceNumber(999);
        system.initNetwork(SPBFT_PEER_COUNT);
        system.setFaultTolerance(FAULT);
        system.makeByzantines(SPBFT_PEER_COUNT*0.2);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            system.shuffleByzantines(SPBFT_PEER_COUNT*0.2);
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
        }
        csv<< header<< std::endl;
        csv<< system.securityLevel1()*SPBFT_GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel1())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
        csv<< system.securityLevel2()*SPBFT_GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel2())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
        csv<< system.securityLevel3()*SPBFT_GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel3())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
        csv<< system.securityLevel4()*SPBFT_GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel4())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
        csv<< system.securityLevel5()*SPBFT_GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel5())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;

    }
}

void MOTIVATIONAL12_Sharded_PBFT(std::ofstream &csv, std::ofstream &log){
    std::string header = "Committee Size, Defeated Transactions, Confirmed/Submitted";
    
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(SPBFT_GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.setSquenceNumber(999);
        system.initNetwork(SPBFT_PEER_COUNT);
        system.setFaultTolerance(FAULT);
        system.makeByzantines(SPBFT_PEER_COUNT*0.2);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.shuffleByzantines(SPBFT_PEER_COUNT*0.2);
            system.makeRequest(system.securityLevel1());
            system.makeRequest(system.securityLevel1());
            system.makeRequest(system.securityLevel1());
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
        }
        csv<< header<< std::endl;
        csv<< system.securityLevel1()*SPBFT_GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel1())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
    }
    
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(SPBFT_GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.setSquenceNumber(999);
        system.initNetwork(SPBFT_PEER_COUNT);
        system.setFaultTolerance(FAULT);
        system.makeByzantines(SPBFT_PEER_COUNT*0.2);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.shuffleByzantines(SPBFT_PEER_COUNT*0.2);
            system.makeRequest(system.securityLevel2());
            system.makeRequest(system.securityLevel2());
            system.makeRequest(system.securityLevel2());
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
        }
        csv<< header<< std::endl;
        csv<< system.securityLevel2()*SPBFT_GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel2())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
    }
    
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(SPBFT_GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.setSquenceNumber(999);
        system.initNetwork(SPBFT_PEER_COUNT);
        system.setFaultTolerance(FAULT);
        system.makeByzantines(SPBFT_PEER_COUNT*0.2);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.shuffleByzantines(SPBFT_PEER_COUNT*0.2);
            system.makeRequest(system.securityLevel3());
            system.makeRequest(system.securityLevel3());
            system.makeRequest(system.securityLevel3());
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
        }
        csv<< header<< std::endl;
        csv<< system.securityLevel3()*SPBFT_GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel3())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
        
    }
    
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(SPBFT_GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.setSquenceNumber(999);
        system.initNetwork(SPBFT_PEER_COUNT);
        system.setFaultTolerance(FAULT);
        system.makeByzantines(SPBFT_PEER_COUNT*0.2);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest(system.securityLevel4());
            system.makeRequest(system.securityLevel4());
            system.makeRequest(system.securityLevel4());
            system.shuffleByzantines(SPBFT_PEER_COUNT*0.2);
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
        }
        csv<< header<< std::endl;
        csv<< system.securityLevel4()*SPBFT_GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel4())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
        
    }
    
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(SPBFT_GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.setSquenceNumber(999);
        system.initNetwork(SPBFT_PEER_COUNT);
        system.setFaultTolerance(FAULT);
        system.makeByzantines(SPBFT_PEER_COUNT*0.2);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.shuffleByzantines(SPBFT_PEER_COUNT*0.2);
            system.makeRequest(system.securityLevel5());
            system.makeRequest(system.securityLevel5());
            system.makeRequest(system.securityLevel5());
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
        }
        csv<< header<< std::endl;
        csv<< system.securityLevel5()*SPBFT_GROUP_SIZE<< ","<< totalNumberOfDefeatedCommittees(system.getGlobalLedger(),system.securityLevel5())<< ","<< double(system.getGlobalLedger().size())/totalSub <<std::endl;
        
    }
    
}
void PARAMETER1_Sharded_PBFT(std::ofstream &csv, std::ofstream &log){
    std::string header = "Round, Confirmed/Submitted";
    csv<< header<< std::endl;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(SPBFT_GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.setSquenceNumber(999);
        system.initNetwork(SPBFT_PEER_COUNT);
        system.setFaultTolerance(FAULT);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                csv<< i<< ","<< double(system.getGlobalLedger().size())/totalSub<< std::endl;
            }
        }
        csv<< NUMBER_OF_ROUNDS<< ","<< double(system.getGlobalLedger().size())/totalSub<< std::endl;
    }
}

void PARAMETER2_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, int delay){
    std::string header = "Round, Confirmed/Submitted";
    csv<< header<< std::endl;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(SPBFT_GROUP_SIZE);
        system.setToRandom();
        system.setMaxDelay(delay);
        system.setLog(log);
        system.setSquenceNumber(999);
        system.initNetwork(SPBFT_PEER_COUNT);
        system.setFaultTolerance(FAULT);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.shuffleByzantines(SPBFT_PEER_COUNT*0.1);
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                csv<< i<< ","<< double(system.getGlobalLedger().size())/totalSub<< std::endl;
            }
        }
        csv<< NUMBER_OF_ROUNDS<< ","<< double(system.getGlobalLedger().size())/totalSub<< std::endl;
    }
}

void ADAPTIVE11_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, int delay){
    std::string header = "Round, Confirmed/Submitted";
    csv<< header<< std::endl;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(SPBFT_GROUP_SIZE);
        system.setToRandom();
        system.setMaxDelay(delay);
        system.setLog(log);
        system.setSquenceNumber(999);
        system.initNetwork(SPBFT_PEER_COUNT);
        system.setFaultTolerance(FAULT);
        system.makeByzantines(SPBFT_PEER_COUNT*0.2);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            system.shuffleByzantines(SPBFT_PEER_COUNT*0.2);
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                csv<< i<< ","<< double(system.getGlobalLedger().size())/totalSub<< std::endl;
            }
        }
        csv<< NUMBER_OF_ROUNDS<< ","<< double(system.getGlobalLedger().size())/totalSub<< std::endl;
    }
}
void ADAPTIVE12_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, double byzantine){
    std::string header = "Round, Confirmed/Submitted";
    csv<< header<< std::endl;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(SPBFT_GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.setSquenceNumber(999);
        system.initNetwork(SPBFT_PEER_COUNT);
        system.setFaultTolerance(FAULT);
        system.makeByzantines(SPBFT_PEER_COUNT*byzantine);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            system.shuffleByzantines(SPBFT_PEER_COUNT*byzantine);
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                csv<< i<< ","<< double(system.getGlobalLedger().size())/totalSub<< std::endl;
            }
        }
        csv<< NUMBER_OF_ROUNDS<< ","<< double(system.getGlobalLedger().size())/totalSub<< std::endl;
    }
}
void ADAPTIVE21_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, int delay){
    std::string header = "Round, Confirmed/Submitted";
    csv<< header<< std::endl;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(SPBFT_GROUP_SIZE);
        system.setToRandom();
        system.setMaxDelay(delay);
        system.setLog(log);
        system.setSquenceNumber(999);
        system.initNetwork(SPBFT_PEER_COUNT);
        system.setFaultTolerance(FAULT);
        system.makeByzantines(SPBFT_PEER_COUNT*0.2);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            system.shuffleByzantines(SPBFT_PEER_COUNT*0.2);
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                csv<< i<< ","<< waitTime(system.getGlobalLedger())<< std::endl;
            }
        }
        csv<< NUMBER_OF_ROUNDS<< ","<< waitTime(system.getGlobalLedger())<< std::endl;
    }
}
void ADAPTIVE22_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, double byzantine){
    std::string header = "Round, Confirmed/Submitted";
    csv<< header<< std::endl;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(SPBFT_GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.setSquenceNumber(999);
        system.initNetwork(SPBFT_PEER_COUNT);
        system.setFaultTolerance(FAULT);
        system.makeByzantines(SPBFT_PEER_COUNT*byzantine);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            system.shuffleByzantines(SPBFT_PEER_COUNT*byzantine);
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
            if(i%100 == 0){
                csv<< i<< ","<< waitTime(system.getGlobalLedger())<< std::endl;
            }
        }
        csv<< NUMBER_OF_ROUNDS<< ","<< waitTime(system.getGlobalLedger())<< std::endl;
    }
}
void ADAPTIVE3_Sharded_PBFT(std::ofstream &csv, std::ofstream &log, double byzantine){
    std::string header = "fraction of byzantine, defeated/submitted";
    csv<< header<< std::endl;
    for(int r = 0; r < NUMBER_OF_RUNS; r++){
        PBFTReferenceCommittee system = PBFTReferenceCommittee();
        system.setGroupSize(SPBFT_GROUP_SIZE);
        system.setToRandom();
        system.setToOne();
        system.setLog(log);
        system.setSquenceNumber(999);
        system.initNetwork(SPBFT_PEER_COUNT);
        system.setFaultTolerance(FAULT);
        system.makeByzantines(SPBFT_PEER_COUNT*byzantine);
        int totalSub = 0;
        for(int i = 0; i < NUMBER_OF_ROUNDS; i++){
            system.makeRequest();
            system.makeRequest();
            system.makeRequest();
            system.shuffleByzantines(SPBFT_PEER_COUNT*byzantine);
            totalSub = totalSub + 3;
            system.receive();
            std::cout<< 'r'<< std::flush;
            system.preformComputation();
            std::cout<< 'p'<< std::flush;
            system.transmit();
            std::cout<< 't'<< std::flush;
            system.log();
        }
        csv<< byzantine<< ","<< defeatedTrnasactions(system.getGlobalLedger())<< std::endl;
    }
}
