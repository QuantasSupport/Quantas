//
// Created by srai on 3/28/19.
//

#ifndef Block_hpp
#define Block_hpp

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <set>
using std::vector;
using std::string;
using std::set;

class Block {
	int 							index = 0;
	string 							previousHash;
	string 							hash;
	set<string> 					publishers;

public:
    Block                                                               ()= default;
	Block																(int, string, string, set<string> );
	string 							getPreviousHash						() const;
	string 							getHash								() const;
	int 							getIndex							() const;
	set<string> 					getPublishers						() const;
	friend std::ostream &operator<<(std::ostream &os, const Block &blockToPrint);

};


#endif