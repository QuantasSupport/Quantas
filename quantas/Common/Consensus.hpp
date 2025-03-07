/*
Copyright 2023

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version. QUANTAS is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef Consensus_hpp
#define Consensus_hpp

#include <algorithm>
#include <atomic>
#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>

namespace quantas {
// -- basic data storage types --

class Member {
    public:
        interfaceId getId() {return _id};
        bool o
    private:
        interfaceId _id;
}

class Committee {
    public:

        int getId() {return _committeeId};
        std::unordered_set<*Member> getMembers() {return _members};
        int size() { return _members.size(); }
        virtual bool has(interfaceId peerID) = 0;

    private:
        int _committeeId;
        std::unordered_set<*Member> _members;
};

}