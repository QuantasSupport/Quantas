//
// Created by srai on 4/12/19.
//

#ifndef DAGBlock_hpp
#define DAGBlock_hpp


#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <set>

using std::vector;
using std::string;
using std::set;

class DAGBlock {
	int 								index = 0;
	std::vector<string> 				previousHashes = {};
	string 								hash="";
	set<string> 						publishers={};
	string                          	data="";
	bool 								isByzantine = false;

public:
	DAGBlock                                                               	() = default;
	DAGBlock																(const DAGBlock &);
	DAGBlock&							operator=							(const DAGBlock &);
	DAGBlock																(int, vector<string>, string, set<string> , string, bool);
	~DAGBlock()= default;
	std::vector<string> 				getPreviousHashes					() const;
	string 								getHash								() const;
	int 								getIndex							() const;
	set<string> 						getPublishers						() const;
	string                          	getData                             () const;
	bool                          		isByantine                          () const;
	friend std::ostream&				operator<<							(std::ostream &os, const DAGBlock &blockToPrint);

};


#endif