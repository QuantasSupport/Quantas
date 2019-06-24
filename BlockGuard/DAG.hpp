#ifndef Dag_hpp
#define Dag_hpp


#include <iostream>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <vector>
#include <queue>
#include "DAGBlock.hpp"

class DAG {
	struct Vertex {
		std::string 							id;
		DAGBlock 								dagBlock;
		std::string 							publisherId;
		int 									vIndex = 0;
		bool 									operator<							(const Vertex& rhs) const{ return  this->id<rhs.id; }
		Vertex& 								operator=							(const Vertex& rhs) = default;
		Vertex																		(const Vertex& rhs) = default;
		Vertex																		() = default;

	};

	struct Edge {
		std::string 							edgeInfo;
	};

	typedef 									boost::adjacency_list<boost::listS, boost::listS, boost::directedS, Vertex, Edge> 		Graph;
	typedef 									boost::graph_traits<Graph>::vertex_descriptor 											vertexT;
	typedef 									boost::graph_traits<Graph>::edge_descriptor 											edgeT;
	typedef 									boost::graph_traits<Graph>::vertex_iterator 											vertexIt;
	typedef 									boost::graph_traits<Graph>::edge_iterator 												edgeIt;

	Graph 										g;
	std::vector<std::string> 					tips;

public:

	DAG(){
		g = {};
		tips ={};
	}

	explicit DAG(bool init){
		if (init) {
			std::cerr<<"From init"<<std::endl;
			DAGBlock newBlock = this->createBlock(0,{"-1_-1"},"genesisHash", std::set<string>{"genesisP"}, "genesisD", false);
			this->addBlockToDAG(newBlock, {"-1_-1"}, false);
		}
	}

	DAG& operator=(const DAG& rhs){
		if(this == &rhs)
			return *this;
		tips = rhs.tips;
		g = rhs.g;
		return *this;
	}

	void addVertex(const DAGBlock& dagBlock, const std::vector<std::string>& ids = {"EMPTY"}, bool byzantineFlag=false) {
		int vertex_count = num_vertices(g);
		Vertex v = { dagBlock.getHash(), dagBlock, *(dagBlock.getPublishers().begin()), vertex_count };
		//check if vertex already present
		{
			const auto iter = findVertex(g, v.id);
			const auto theEnd = boost::vertices(g).second;
			if (iter != theEnd) {
//                std::cerr<<"VERTEX FOUND ALREADY"<<std::endl;
				//don't add
				return;
			} else {
//                std::cerr << "VERTEX NOT FOUND, ADDING IT\n";
			}
		}
        v.dagBlock.setConfirmedRound(dagBlock.getConfirmedRound());
        v.dagBlock.setSubmissionRound(dagBlock.getSubmissionRound());
		vertexT a = boost::add_vertex(v, g);
		for (auto &id: ids) {
			const auto iter = findVertex(g, id);
			const auto theEnd = boost::vertices(g).second;
			if (iter != theEnd) {
				boost::add_edge(a, *iter, Edge{id}, g);

			} else {
//                std::cerr << "FAILED TO FIND THE PREVIOUS HASH\n";
			}
		}
	}

	bool transactionInDag(const std::string& tx) {
		//iterate over the blocks in each vertex and check if the transaction is present or not
		for (auto vd : boost::make_iterator_range(vertices(g))) {
			if(g[vd].dagBlock.getData() == tx){
				std::cerr<<"BLOCK FOUND BEFORE TRANSACTION"<<g[vd].dagBlock.getData()<<std::endl;
				return true;
			}
		}
		std::cerr<<"BLOCK NOT FOUND BEFORE TRANSACTION"<<std::endl;
		return false;

	}

	void printGraph() {
		for (std::pair<edgeIt, edgeIt> it = boost::edges(g); it.first != it.second; ++it.first) {
			std::cerr << g[source(*it.first, g)].id << "-> " << g[target(*it.first, g)].id << std::endl;
		}

		Graph::vertex_iterator v, vend, next;

		boost::tie(v, vend) = boost::vertices(g);
		for (next = v; v != vend; v = next) {
			++next;
			std::cerr<< g[*v].id << " "<<out_degree(*v, g) << std::endl;
		}
		for (auto vd : boost::make_iterator_range(vertices(g))) {
			std::cerr << "Index: "<<g[vd].vIndex
			          <<" Vertex id:" << g[vd].id
					  << "\n";
		}
	}

	boost::graph_traits<Graph>::vertex_iterator findVertex(const Graph &g, const std::string &id) {
		boost::graph_traits<Graph>::vertex_iterator vi, vi_end;

		for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
			if (g[*vi].id == id) return vi;
		}
		return vi_end;
	}

	std::vector<std::string> topologicalSort() {
		// Create a vector to store indegrees of all vertices. Initialize all indegrees as 0.
		int verticesCount = num_vertices(g);
		std::map<Vertex*, int> vDegree;

		// Traverse adjacency lists to fill indegrees of vertices.  This step takes O(V+E) time

		auto vertex_idMap = get(boost::vertex_bundle, g);
		boost::graph_traits<Graph>::vertex_iterator iter, end;
		boost::graph_traits<Graph>::adjacency_iterator ai, ai_end;

		for (boost::tie(iter, end) = vertices(g); iter != end; ++iter) {
			vDegree.insert({&vertex_idMap[*iter], 0});
			int adjCount = 0;
			for (boost::tie(ai, ai_end) = adjacent_vertices(*iter, g); ai != ai_end; ++ai) {
				adjCount++;
				vDegree[&vertex_idMap[*ai]] = vDegree[&vertex_idMap[*ai]] + 1;
			}
			vDegree.insert({&vertex_idMap[*iter], adjCount});
		}

		//Adjacency count
		std::vector<Vertex*> tempVec;
		for (const auto &pair: vDegree) {
			if (vDegree[pair.first] == 0) {
				tempVec.push_back(pair.first);
			}
		}

		//sort the vector lexicographically
		Vertex temp;
		for (int i = 0; i < tempVec.size() - 1; ++i) {
			for (int j = i + 1; j < tempVec.size(); ++j) {
				if (tempVec[i]->id < tempVec[j]->id) {
					temp = *tempVec[i];
					*(tempVec[i]) = *(tempVec[j]);
					*(tempVec[j]) = temp;
				}
			}
		}

		std::queue<Vertex*> q;
		for (auto i : tempVec) {
			q.push(i);
		}

		// Initialize count of visited vertices
		int cnt = 0;

		// Create a vector to store result (A topological ordering of the vertices)
		vector<std::string> top_order;

		// One by one dequeue vertices from queue and enqueue adjacents if indegree of adjacent becomes 0
		while (!q.empty()) {
			//sort lexicographically
			std::vector<Vertex*> tVec;
			while (!q.empty()) {
				tVec.push_back(q.front());
				q.pop();
			}

			Vertex tmp;
			for (int i = 0; i < tVec.size() - 1; ++i) {
				for (int j = i + 1; j < tVec.size(); ++j) {
					if (tVec[i]->id < tVec[j]->id) {
						tmp = *(tVec[i]);
						*(tVec[i]) = *(tVec[j]);
						*(tVec[j]) = tmp;
					}
				}
			}

			for (auto i : tVec) {
				q.push(i);
			}

			// Extract front of queue (or perform dequeue) and add it to topological order
			std::string u = q.front()->publisherId;
			std::string idToSearch = q.front()->id;
			q.pop();
			top_order.push_back(u);

			/* Iterate through all its neighbouring nodes
			 of dequeued node u and decrease their in-degree
			 by 1*/

			const auto ite = findVertex(g, idToSearch);
			boost::graph_traits<Graph>::adjacency_iterator adi, a_end;

			for (boost::tie(adi, a_end) = adjacent_vertices(*ite, g); adi != a_end; ++adi) {
				vDegree[&vertex_idMap[*adi]] = vDegree[&vertex_idMap[*adi]] - 1;

				if (vDegree[&vertex_idMap[*adi]] == 0) {
					q.push(&vertex_idMap[*adi]);
				}
			}
			cnt++;
		}

		// Check if there was a cycle
		if (cnt != verticesCount) {
			std::cerr << "There exists a cycle in the graph\n";
		}
		assert(cnt == verticesCount);

		// Print topological order
		for (const auto & i : top_order){ std::cerr<< i << " "; }

		return top_order;

	}

	DAGBlock createBlock(int blockIndex, vector<string> prevHashes, string blockHash, set<string> publishers, string data, bool byzantine) {
		//inserting to the chain without any work
		DAGBlock b(blockIndex,prevHashes,blockHash, publishers, data, byzantine);
		return b;
	}

	void addBlockToDAG(const DAGBlock& b, const vector<string>& prevHashes, bool byzantine){
		addVertex(b,prevHashes,byzantine);
	}

	int getSize(){
		return num_vertices(g);
	}

	void setTips(){
		tips.clear();

		std::map<Vertex*, int> vDegree;
		auto vertex_idMap = get(boost::vertex_bundle, g);

		boost::graph_traits<Graph>::vertex_iterator i, end;
		boost::graph_traits<Graph>::adjacency_iterator ai, a_end;
		for (boost::tie(i, end) = vertices(g); i != end; ++i) {
			vDegree.insert({&vertex_idMap[*i], 0});
			int adjCount = 0;
			for (boost::tie(ai, a_end) = adjacent_vertices(*i, g); ai != a_end; ++ai) {
				adjCount++;
				vDegree[&vertex_idMap[*ai]] = vDegree[&vertex_idMap[*ai]] + 1;

			}
			vDegree.insert({&vertex_idMap[*i], adjCount});
		}

		for (auto it : vDegree) {
			if(it.second==0){
				tips.push_back(it.first->id);
			}
		}
	}

    std::vector<string> getTips(){
        return tips;
    }

    std::vector<DAGBlock> getTransactions()const{
        std::vector<DAGBlock> transactions = std::vector<DAGBlock>();
        for (auto vd : boost::make_iterator_range(vertices(g))) {
            transactions.push_back(g[vd].dagBlock);
        }
        return transactions;
    }
};


#endif //Dag_hpp

