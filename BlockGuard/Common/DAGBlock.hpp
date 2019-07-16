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
	int 								index           = 0;
	std::vector<string> 				previousHashes  = {};
	string 								hash            = "";
	set<string> 						publishers      = {};
	string                          	data            = "";
	bool 								byzantine       = false;
    
    // metrics
    int                                 secruityLevel   = 0;
    int                                 submissionRound = -1;
    int                                 confirmedRound  = -1;

public:
	DAGBlock                                                               	() = default;
	DAGBlock																(const DAGBlock &);
	DAGBlock&							operator=							(const DAGBlock &);
    
	DAGBlock																(int, vector<string>, string, set<string> , string, bool);
	~DAGBlock()= default;
    
    // setters
    void                                setSecruityLevel                    (int s)                                                     {secruityLevel = s;};
    void                                setSubmissionRound                  (int s)                                                     {submissionRound = s;};
    void                                setConfirmedRound                   (int c)                                                     {confirmedRound = c;};
    void                                setIndex                            (int i)                                                     {index = i;};
    void                                setPreviousHashes                   (std::vector<string> ph)                                    {previousHashes = ph;};
    void                                setHash                             (string h)                                                  {hash = h;};
    void                                setPublishers                       (set<string> p)                                             {publishers = p;};
    void                                setData                             (string d)                                                  {data = d;};
    void                                setByzantine                        (bool b)                                                    {byzantine = b;};
    
    
    
    // getters
	std::vector<string> 				getPreviousHashes					() const;
	string 								getHash								() const;
	int 								getIndex							() const;
	set<string> 						getPublishers						() const;
	string                          	getData                             () const;
    bool                          		isByzantine                         () const                                                    {return byzantine;};
    int                                 getSecruityLevel                    () const                                                    {return secruityLevel;};
    int                                 getSubmissionRound                  () const                                                    {return submissionRound;};
    int                                 getConfirmedRound                   () const                                                    {return confirmedRound;};
    
    // operators
    bool                                operator==                          (const DAGBlock &rhs)const;
    bool                                operator!=                          (const DAGBlock &rhs)const                                  {return !(*this == rhs);};
	friend std::ostream&				operator<<							(std::ostream &os, const DAGBlock &blockToPrint);

};


#endif
