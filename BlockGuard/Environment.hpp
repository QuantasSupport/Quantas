//
//  Environment.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 5/30/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef Environment_hpp
#define Environment_hpp

#include <stdio.h>
#include <map>
#include "ByzantineNetwork.hpp"


template<class type_msg, class peer_type>
class Environment{
protected:
    // Environment settings
    std::map<type_msg,int>                  _confirmedTransactionsBySecurity;
    double                                  _transactionRate;   // double becouse transaction rate can be less then one per round
    int                                     _shuffleTime;       // number of rounds between shuffling byzantine peers
    int                                     _byzantineMax;      // number of byzantine peers in the system (there will be one experiment for [max,min])
    int                                     _byzantineMin;
    int                                     _delayMax;          // max and min delay to use for simulations (there will be one experiment for [max,min])
    int                                     _dleayMin;
    int                                     _size;              // number of peers in experiments
    
    // system
    ByzantineNetwork<type_msg, peer_type>   _system;
    
public:
    Environment                                          ();
    Environment                                          (const Environment&);
    ~Environment                                         ()                                         {};
    
    // setters
    
    
    // getters (metrics)
    
    // mutators
    
    // logging and debugging
    void                              log                ()const;
    std::ostream&                     printTo            (std::ostream&)const;
    
    // operators
    Environment&                      operator=          (const Environment&);
    friend std::ostream&              operator<<          (std::ostream&, const Environment& e)     {return e.printTo(out);};;
};

#endif /* Environment_hpp */
