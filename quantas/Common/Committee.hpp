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

#ifndef COMMITTEE_HPP
#define COMMITTEE_HPP

#include <set>

#include "Packet.hpp"

namespace quantas {

//---------------------------------
// Committee: A group of members
//---------------------------------
class Committee {
public:
    Committee() : _committeeId(-1) {};
    Committee(int committeeId) : _committeeId(committeeId) {};
    Committee(const Committee& other)
        : _committeeId(other._committeeId),
          _members(other._members)  // std::set has its own copy constructor
    {}
    
    virtual ~Committee() {};

    int getId() const { return _committeeId; }
    void setId(int committeeId) {_committeeId = committeeId;}

    // Return a const reference to avoid copying the entire set
    const std::set<interfaceId>& getMembers() const { return _members; }

    void addMember(const interfaceId& toAdd) {
        _members.insert(toAdd);
    }

    void removeMember(const interfaceId& toRemove) {
        _members.erase(toRemove);
    }

    bool contains(interfaceId peerID) const {
        return _members.find(peerID) != _members.end();
    }

    int size() const {
        return static_cast<int>(_members.size());
    }

    // Comparisons for sorting committees, if needed
    bool operator==(const Committee& rhs) const {
        return _committeeId == rhs._committeeId;
    }

    bool operator<(const Committee& rhs) const {
        return _committeeId < rhs._committeeId;
    }

private:
    int _committeeId = -1;
    std::set<interfaceId> _members;
};

}

#endif /* COMMITTEE_HPP */