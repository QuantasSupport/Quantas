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
#include <iostream>
#include <chrono>
#include <random>

#include "./Common/Network.hpp"
#include "./Common/Peer.hpp"
#include "ExamplePeer.hpp"
// RefCom
#include "./Experiments/refComExperiments.hpp"
// SBFT
#include "./SBFT/syncBFT_Peer.hpp"
#include "./SBFT/syncBFT_Committee.hpp"
// PBFT
#include "./PBFT/PBFT_Peer.hpp"
#include "./PBFT/PBFTPeer_Sharded.hpp"
#include "./PBFT/DS_PBFT_Peer.hpp"
#include "./PBFT/DS_PBFT.hpp"
#include "./PBFT/PBFT_Committee.hpp"
// POW
#include "./bCoin/bCoin_Peer.hpp"
#include "./bCoin/bCoin_Committee.hpp"
#include "./bCoin/DS_bCoin_Peer.hpp"
// UTIL
#include "./Common/Logger.hpp"
#include "./Common/Blockchain.hpp"
#include "MarkPBFT_peer.hpp"
#include "SmartShard.hpp"

const int peerCount = 10;
const int blockChainLength = 100;
Blockchain *blockchain;
int shuffleByzantineInterval = 0;

// util functions
void buildInitialChain(std::vector<std::string>);
std::set<std::string> getPeersForConsensus(int);

void Example(std::ofstream &logFile);
void syncBFT(const char ** argv);
void bitcoin(std::ofstream &,int );
void DS_bitcoin(const char ** argv);
void run_DS_PBFT(const char ** argv);
void markPBFT(const std::string&);
void smartShard(const std::string&);

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
    else if (algorithm == "syncBFT") {
		//	Program arguments: syncBFT fileName 2 1 1 128 100 0.3 1
		std::ofstream out;
		int runs = std::stoi(argv[3]);
		for(int i = 0; i< runs; i++){
			syncBFT(argv);
		}
	}else if (algorithm == "pbft_s") {
        PBFT_refCom(filePath);
    }else if(algorithm == "pow_s"){
        POW_refCom(filePath);
    }else if(algorithm == "sbft_s"){
        SBFT_refCom(filePath);
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
    }else if (algorithm == "DS_PBFT") {
		//	Program arguments: DS_PBFT asdf 1 1 1 64 100 0.7 1
		std::ofstream out;
		int runs = std::stoi(argv[3]);
		for(int i = 0; i< runs; i++){
			run_DS_PBFT(argv);
		}
	}else if (algorithm == "markpbft") {
		markPBFT(filePath);
	}
	else if (algorithm == "smartshard") {
		smartShard(filePath);
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
    std::deque<std::pair<int,std::string> > txQueue;

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
			txQueue.push_back(std::pair<int,std::string>(i,"Tx_"+std::to_string(i)));
		}

		if(status == Mining){
			for(auto & currentCommittee : currentCommittees){
				currentCommittee->receive();
				currentCommittee->preformComputation();
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
						bCoin_Committee *co = new bCoin_Committee(consensusGroup, p, txQueue.front().second, securityLevel);
						concurrentGroupCount++;
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
					committee->initiate(txQueue.front().first);
				}
                txQueue.pop_front();
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

    std::string filePath     =     argv[2];
    int avgDelay             =     std::stoi(argv[4]);
    int byzantineOrNot       =     std::stoi(argv[5]);
    int peersCount           =     std::stoi(argv[6]);
    int iterationCount       =     std::stoi(argv[7]);
    double tolerance         =     std::stod(argv[8]);
    int txRate               =     std::stoi(argv[9]);

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

//    for printing security levels
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
    const std::string MINING             = "MINING";
    const std::string COLLECTING         = "COLLECTING";
    const std::string WAITING_FOR_TX     = "WAITING_FOR_TX";

    std::string status = COLLECTING;
    int collectInterval = 0;
    std::vector<vector<syncBFT_Peer*>> consensusGroups;
    std::vector<int> consensusGroupCount;
    std::map<int, int> securityLevelCount;
    std::map<int, int> defeatedCommittee;
    std::map<int, double> confirmationRate;

    //    initial block size is peersCount + 1
    int prevConfirmationSize = peersCount + 1;
    int waitTime = 2 * n.maxDelay();

    for(int i = 0; i<iterationCount; i++){
        /////////////////////////////////////////////////
        // collect metrics
        //
        if(i % 100 == 0){
            //    saturation point calculation, keep track of confirmed count, look at the dag of any peer to find the chain size. i.e. number of confirmed blocks
            //    number of transactions introduced will be 100/txRate
            confirmationRate[i] = ((double)(n[0]->getDAG().getSize() - prevConfirmationSize))/(100);
            prevConfirmationSize = n[0]->getDAG().getSize();
        }
        /////////////////////////////////////////////////
        // queue transactions
        //
        if( i%txRate == 0  ){
            txQueue.push_back("Tx_"+std::to_string(i));
        }
        Logger::instance()->log("----------------------------------------------------------Iteration "+std::to_string(i)+"\n");
        /////////////////////////////////////////////////
        // phase 1 waiting for tx
        //
        if(status == WAITING_FOR_TX){
//            wait for max delay until all committees receive their transactions
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

        /////////////////////////////////////////////////
        // phase 2 working on consensus
        //
        }else if(status == MINING){
            waitTime--;

            for(auto & currentCommittee : currentCommittees){
                if(currentCommittee->getConsensusFlag()){
                    continue;
                }
                currentCommittee->receive();
                currentCommittee->preformComputation();
                if(waitTime == 0){
                    currentCommittee->nextState(i, n.maxDelay());
                    waitTime = 2*n.maxDelay();
                }
                currentCommittee->transmit();
            }

            //    don't erase but check to see if consensus reached in all
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
                //    send blocks
                Logger::instance()->log("CONSENSUS REACHEDDDD \n");
                for(auto & currentCommittee : currentCommittees){
					//  defeated committees count
					if(currentCommittee->getDefeated()){
						defeatedCommittee[currentCommittee->getSecurityLevel()]++;
					}
                    //    propogate the block to whole network except the committee itself.
                    std::vector<std::string> peerIds = currentCommittee->getCommitteePeerIds();
                    std::map<std::string, Peer<syncBFTmessage>* > neighbours;


                    //    selecting the first peer in the committee, as all peers agree on the same block.
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
        /////////////////////////////////////////////////
        // phase 3 working on collcetcing
        //
        }else if(status == COLLECTING){
			waitTime = 2*n.maxDelay();
            //    make sure no peer is busy
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
                    //    collection is not complete yet
                    continue;
                }
            }

            //    shuffling byzantines
            if(byzantineOrNot==1){
				int shuffleCount = peersCount/10;
				if (byzantinePeers.size()<peersCount/10) {
					shuffleCount = byzantinePeers.size();
				}
				Logger::instance()->log("Shuffling "+std::to_string(shuffleCount)+" Peers.\n");
				n.shuffleByzantines (shuffleCount);
            }
            /////////////////////////////////////////////////
            // adding new committees
            //
            if(!txQueue.empty()){
                //    select a random peer
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

                        concurrentGroupCount++;
                        txQueue.pop_front();
                        currentCommittees.push_back(co);
                        consensusGroups.push_back(consensusGroup);
                        securityLevelCount[securityLevel]++;

                    }
                }while(!consensusGroup.empty() && !txQueue.empty()); //build committees until a busy peer is jumped on.

                consensusGroupCount.push_back(concurrentGroupCount);
            }

//            change status after waiting for all peers to get their transaction
            status = WAITING_FOR_TX;
            /////////////////////////////////////////////////
            // restart/start consensus for each committee
            //
            if(!currentCommittees.empty()){
                Logger::instance()->log("INITIATING " + std::to_string(currentCommittees.size()) + " COMMITTEES\n");
                for(auto &committee: currentCommittees){
                    committee->initiate();
                }
//                change status after waiting for all peers to get their transaction
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

void run_DS_PBFT(const char ** argv){
	std::string filePath	= 	argv[2];
	int delay 			= 	std::stoi(argv[4]);
	int byzantineOrNot 		= 	std::stoi(argv[5]);
	int peersCount 			= 	std::stoi(argv[6]);
	int iterationCount 		= 	std::stoi(argv[7]);
	double tolerance 		= 	std::stod(argv[8]);
	int txRate 				= 	std::stoi(argv[9]);

	Logger::setLogFileName(filePath + "_"+std::to_string(std::chrono::system_clock::now().time_since_epoch().count())+"_"+argv[1]+"_delay"+std::to_string(delay)+"_peerCount"+std::to_string(peersCount)
						   +"_iterationCount"+std::to_string(iterationCount)+"_tolerance"+std::to_string(tolerance)+"_txRate"+std::to_string(txRate)+".txt");

	int numberOfPeers = peersCount;
	double fault = 0.3;


	//	for printing security levels
	std::vector<int> securityLevels;
	securityLevels.push_back(peersCount/16);
	securityLevels.push_back(peersCount/8);
	securityLevels.push_back(peersCount/4);
	securityLevels.push_back(peersCount/2);
	securityLevels.push_back(peersCount/1);


	std::cout<< std::endl<< "########################### DS_PBFT ###########################"<< std::endl;
	DS_PBFT instance = DS_PBFT();
	instance.setToRandom();
	instance.setMaxDelay(delay);
	instance.setSquenceNumber(999);
	instance.initNetwork(numberOfPeers);
	instance.setFaultTolerance(FAULT);
	instance.setDelay(delay);
	instance.makeByzantines(numberOfPeers*tolerance);

	std::map<int, double> confirmationPerIteration;
	int prevConfirmationSize = peersCount + 1;

	instance.status = COLLECTING;
	int numberOfRequests = 0;
	for(int i =0; i < iterationCount; i++){
		//	saturation point calculation, keep track of confirmed count, look at the dag of any peer to find the chain size. i.e. number of confirmed blocks
		//	number of transactions introduced will be 1/txRate
//		confirmation rate, rolling average
		confirmationPerIteration[i] = (double)(instance[0]->getDAG().getSize() - prevConfirmationSize);
		prevConfirmationSize = instance[0]->getDAG().getSize();

		if(i%txRate == 0){
			Logger::instance()->log("Making a request\n");
			instance.makeRequest();numberOfRequests++;
		}

		Logger::instance()->log("------------------ITERATION-----------------\n");
		instance.run(i);
//		instance.receive();
//		std::cout<< 'r'<< std::flush;
//		instance.preformComputation();
//		std::cout<< 'p'<< std::flush;
//		instance.transmit();
//		std::cout<< 't'<< std::flush;
	}


	Logger::instance()->log("FINALLY\n");


	for(int i =0; i<instance.size();i++){
		Logger::instance()->log("PEER " + std::to_string(i) + " DAG SIZE IS " + std::to_string(instance[i]->getDAG().getSize())+"\n");
	}

	Logger::instance()->log("CONFIRMATION COUNT = " + std::to_string(instance[0]->getDAG().getSize() - numberOfPeers - 1)+"\n");

	Logger::instance()->log("DEFEATED COMMITTEES COUNT \n");

	std::map<int, int> defeatedCommitteesBySecurityLevel;
	for_each( instance.defeatedCommittees.begin(), instance.defeatedCommittees.end(), [&defeatedCommitteesBySecurityLevel]( int val ){ defeatedCommitteesBySecurityLevel[val]++; } );


	for(auto l : securityLevels){
		if(defeatedCommitteesBySecurityLevel.count(l)){
			Logger::instance()->log(std::to_string(l) + " " + std::to_string(defeatedCommitteesBySecurityLevel[l]) + "\n");
		} else
			Logger::instance()->log(std::to_string(l) + " 0\n");
	}

	Logger::instance()->log("TOTAL COMMITTEES COUNT \n");

	std::map<int, int> totalCommitteesBySecurityLevel;
	for_each( instance.totalCommittees.begin(), instance.totalCommittees.end(), [&totalCommitteesBySecurityLevel]( int val ){ totalCommitteesBySecurityLevel[val]++; } );


	for(auto l : securityLevels){
		if(totalCommitteesBySecurityLevel.count(l)){
			Logger::instance()->log(std::to_string(l) + " " + std::to_string(totalCommitteesBySecurityLevel[l]) + "\n");
		} else
			Logger::instance()->log(std::to_string(l) + " 0\n");
	}

	Logger::instance()->log("RATIO OF DEFEATED COMMITTEES\n");
	for(auto l : securityLevels){
		if(defeatedCommitteesBySecurityLevel.count(l)){
			Logger::instance()->log(std::to_string(l) +  " " + std::to_string((double)defeatedCommitteesBySecurityLevel[l]/totalCommitteesBySecurityLevel[l]) + "\n");
		} else
			Logger::instance()->log(std::to_string(l) + " 0\n");
	}

	Logger::instance()->log("CONFIRMATION RATE, ROLLING TIMELINE\n");
	for(auto cr : confirmationPerIteration){
		Logger::instance()->log(std::to_string(cr.first) + ": " + std::to_string(cr.second) + "\n");
	}
	int rangeStart = 0;
	std::vector<double> rollingAvgThroughputTimeline;
	for(rangeStart=0;(rangeStart+100)<=iterationCount;rangeStart++){
		int rangeEnd = rangeStart + 100;
		double confirmations = 0;
		for(int i = rangeStart; i<rangeEnd; i++){
			confirmations+= confirmationPerIteration[i];
		}
		rollingAvgThroughputTimeline.push_back(confirmations/(100.0/txRate));
	}

	for(auto timeline: rollingAvgThroughputTimeline){
		Logger::instance()->log("ROLLING TIMELINE THROUGHPUT  " + std::to_string(timeline)+"\n");
	}

	std::map<int, std::vector<PBFT_Message>>::iterator iit;

	for(iit = instance.confirmedMessagesPerIteration.begin();iit!= instance.confirmedMessagesPerIteration.end();iit++){
		Logger::instance()->log("Iteration "+std::to_string(iit->first)+"\n");
		for(const auto& msg:iit->second){
			Logger::instance()->log(std::to_string(iit->first)+" - "+ std::to_string(msg.submission_round) + " = " + std::to_string(iit->first - msg.submission_round) + "\n");
		}
	}

//	rolling average waiting time
	std::vector<double> rollingAvgWaitTime;
	for(rangeStart=0;(rangeStart+100)<=iterationCount;rangeStart++){
		int rangeEnd = rangeStart + 100;
		int confirmed = 0;
		double waitTime = 0;
		for(iit = instance.confirmedMessagesPerIteration.begin();iit!= instance.confirmedMessagesPerIteration.end();iit++){
			if( rangeStart < iit->first && iit->first <= rangeEnd){
				for(const auto& msg:iit->second){
					confirmed++;
					waitTime+=  iit->first - msg.submission_round;
				}
			}
		}
		rollingAvgWaitTime.push_back(waitTime/confirmed);
	}

	Logger::instance()->log("ROLLING AVERAGE Throughput\n");
	for(auto waitTime: rollingAvgWaitTime){
		Logger::instance()->log("ROLLING THROUGHPUT " + std::to_string(waitTime)+"\n");
	}
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

void smartShard(const std::string& filePath) {
	
	int minDelay = 1;
	int maxDelay = 1;
	int minShards = 5;
	int maxShards = 5;
	int minChurn = 1;
	int maxChurn = 10;

	double faultTolerance = .333334;
	int numRounds = 1000;
	int requestPerRound = 1;
	int roundstoRequest = 1;
	int tests = 1;

	std::ofstream summary;
	summary.open(filePath + "/summary.log");
	summary << "delay, shards, confirmations, churn" << std::endl;

	for (int delay = minDelay; delay <= maxDelay; delay += 5) {
		for (int numShards = minShards; numShards <= maxShards; ++numShards) {
			for (int churn = minChurn; churn <= maxChurn; ++churn) {

				int totalConfirmations = 0;

				for (int i = 0; i < tests; ++i) {
					std::ofstream out;
					out.open(filePath + "/smart shard_" + "delay_" + std::to_string(delay) + "_shards_" + std::to_string(numShards) + "_test_" + std::to_string(i) + ".log");


					SmartShard system(numShards, out, delay, -1, 0, 2);

					system.setFaultTolerance(faultTolerance);
					system.setRequestsPerRound(requestPerRound); // number of requests to make at one time
					system.setRoundsToRequest(roundstoRequest); // number of times to make a request
					system.setMaxWait();

					system.run(numRounds, churn, 50);

					system.printPeers();
					//system.run(numRounds, churn, 100);

					totalConfirmations += system.getConfirmationCount();
					out.close();

				}
				summary << delay << ", " << numShards << ", " << totalConfirmations / tests << ", " << churn << std::endl;
			}
		}
		if (delay == 1)
			--delay;
	}
	summary.close();
}

void markPBFT(const std::string& filePath) {
	std::cout << "markPBFT" << std::endl;

	std::ofstream summary;
	summary.open(filePath + "/summary.log");

	summary << "test,rounds,requests per round,rounds till requests,delay setting,network size,confirmations,message count,request sent,vote changes,force changes,num byzantine\n";

	int initNetworkSize = 1;
	int maxNetworkSize = 10;
	int initDelay = 1;
	int maxDelay = 1;
	int numTests = 1;
	int initRequestPerRound = 1;
	int maxRequestPerRoound = 1;
	int initRoundstoRequest = 5;
	int maxRoundstoRequest = 5;
	double fault_tolerance = .333333;
	int initByzantine = 0;
	int maxByzantine = 0;

	int viewChangeMult = 5;
	bool enableViewChange = false;

	int rounds = 1000;

	for (int networkSize = initNetworkSize; networkSize <= maxNetworkSize; networkSize = networkSize +1) {
		for (int delay = initDelay; delay <= maxDelay; delay += 1) {

			for (int requestPerRound = initRequestPerRound; requestPerRound <= maxRequestPerRoound; ++requestPerRound) {
				for (int roundstoRequest = initRoundstoRequest; roundstoRequest <= maxRoundstoRequest; ++roundstoRequest) {
					for (int numByzantine = initByzantine; numByzantine <= maxByzantine; ++numByzantine) {

						int totalLatency = 0;
						long totalMessages = 0;
						int numConfirmations = 0;
						int totalRequests = 0;
						int totalVoteChanges = 0;
						int totalForceChanges = 0;

						for (int test = 1; test <= numTests; ++test) {


							std::ofstream out;
							out.open(filePath + "/markPBFT_Delay" + std::to_string(delay) + "networksize_" +
								std::to_string(networkSize) + "requests_" + std::to_string(requestPerRound) + "delayrequest_" + std::to_string(roundstoRequest)
								+ "byzantine_" + std::to_string(numByzantine) + "Test" + std::to_string(test) + ".log");

							if (out.fail())
								std::cerr << "failed to open log file";


							ByzantineNetwork<markPBFT_message, markPBFT_peer> system;
							system.setLog(out);
							system.setToRandom();
							system.setMaxDelay(delay);
							system.initNetwork(networkSize);

							system.makeByzantines(numByzantine);

							// PBFT selects random primary
							int primaryIndex = rand() % networkSize;
							system[primaryIndex]->setPrimary(true);

							// Set Parameters in network
							for (int i = 0; i < networkSize; ++i) {
								system[i]->setFaultTolerance(fault_tolerance);
								system[i]->setRequestPerRound(requestPerRound);
								system[i]->setRoundsToRequest(roundstoRequest);
								system[i]->setMaxWait();

							}

							// Used to monitor notes wanting view change
							int forceCount = 0;
							int voteChanges = 0;
							int forceChanges = 0;

							// Process rounds for every in in network
							for (int i = 0; i < rounds; ++i) {
								for (int j = 0; j < networkSize; ++j)
									system.makeRequest(j);

								if (enableViewChange) {
									// Check votes towards view change
									int voteCount = 0;
									for (int k = 0; k < networkSize; ++k)
										if (system[k]->getVote()) {
											++voteCount;
										}

									// Check conditions for view change
									if (voteCount > (int)(2 * (fault_tolerance * networkSize)) || (forceCount++ > delay * viewChangeMult)) {
										if (voteCount > (int)(2 * (fault_tolerance * networkSize)))
											++voteChanges;
										if (forceCount > delay * viewChangeMult)
											++forceChanges;
										while (system[primaryIndex]->isPrimary())
											primaryIndex = rand() % networkSize;
										system[primaryIndex]->setPrimary(true);
										voteCount = 0;
										forceCount = 0;
									}

								}
								system.log();
							}

							// Following is used for logging

							// Get Ledgers
							std::map<std::string, int> ledger;

							for (int i = 0; i < networkSize; ++i)
								ledger.insert(system[i]->getLedger().begin(), system[i]->getLedger().end());

							out << "--Ledger--\nrequest\t\tround found\n";
							for (auto itr = ledger.begin(); itr != ledger.end(); ++itr) {
								out << itr->first << "\t\t" << itr->second << std::endl;

								totalLatency += itr->second;
							}

							if (ledger.empty()) {
								out << "--Ledger--\n\trequest\t\tround found\n\tMessage not agreed, increase round count" << std::endl;
								//	summary << "not found,";
							}

							int messageSum = 0;
							int requestSum = 0;
							int byzantineSum = 0;
							for (int i = 0; i < networkSize; ++i) {
								messageSum += system[i]->getMessageCount();
								requestSum += system[i]->getRequests().size();
								if (system[i]->isByzantine())
									++byzantineSum;
							}


							out << "--Total Message Count--\t--Total Requests--\t--Byzantine Sum--\n\t" << messageSum << "\t\t\t"
								<< requestSum << "\t\t\t" << byzantineSum << std::endl;
							//summary << messageSum << std::endl;

							totalRequests += requestSum;
							totalMessages += messageSum;
							numConfirmations += ledger.size();
							totalVoteChanges += voteChanges;
							totalForceChanges += forceChanges;


							if (out)
								out.close();
						}
						summary << std::fixed << "averages," << rounds << ',' << requestPerRound << ',' << roundstoRequest << ',' << delay << ','
							<< networkSize << ',' << (float)numConfirmations / numTests << ',' << (float)totalMessages / numTests <<
							',' << (float)totalRequests / numTests << ',' << (float)totalVoteChanges / numTests << ','
							<< (float)totalForceChanges / numTests << ',' << numByzantine << std::endl;
					}
				}
			}
		}

	}
	summary.close();
}