//
//  ByzantineNetwork_Test.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 5/23/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef ByzantineNetwork_Test_hpp
#define ByzantineNetwork_Test_hpp

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "../BlockGuard/ExamplePeer.hpp"
#include "../BlockGuard/Common/ByzantineNetwork.hpp"

void RunByzantineNetworkTest    (std::string filepath);

// Byzantine tests
void testMakeByzantine          (std::ostream &log); // test making peers correct and Byzantine
void testByzantineShuffle       (std::ostream &log); // test that shuffling byzantine 


#endif /* ByzantineNetwork_Test_hpp */
