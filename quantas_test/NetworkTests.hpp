/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef NetworkTests_hpp
#define NetworkTests_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include "./../quantas/Common/Network.hpp"
#include "./../quantas/ExamplePeer.hpp"

void runNetworkTests    (std::string filepath);

void testSize           (std::ostream &log); // test default const
void testIndex          (std::ostream &log); // test access to peers via []


void testOneDelay       (std::ostream &log); // test setDelayOne
void testRandomDelay    (std::ostream &log); // test random distribution
void testPoissonDelay   (std::ostream &log); // test poisson distribution


#endif /* NetworkTests_hpp */
