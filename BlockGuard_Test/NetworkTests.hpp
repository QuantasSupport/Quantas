//
//  NetworkTests.hpp
//  BlockGuard
//
//  Created by Kendric Hood on 6/3/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#ifndef NetworkTests_hpp
#define NetworkTests_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include "./../BlockGuard/Common/Network.hpp"
#include "./../BlockGuard/ExamplePeer.hpp"

void runNetworkTests    (std::string filepath);

void testSize           (std::ostream &log); // test default const
void testIndex          (std::ostream &log); // test access to peers via []


void testOneDelay       (std::ostream &log); // test setDelayOne
void testRandomDelay    (std::ostream &log); // test random distribution
void testPoissonDelay   (std::ostream &log); // test poisson distribution


#endif /* NetworkTests_hpp */
