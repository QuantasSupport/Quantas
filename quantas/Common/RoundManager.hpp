/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QUANTAS is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef RoundManager_hpp
#define RoundManager_hpp

namespace quantas {

class RoundManager {
private:
    int _currentRound{0};
    int _lastRound{0};

    // Private constructor and copy operations to enforce singleton usage:
    RoundManager() = default;
    RoundManager(const RoundManager&) = delete;
    RoundManager& operator=(const RoundManager&) = delete;

public:
    static RoundManager* instance() {
        static RoundManager s;
        return &s;
    }

    static int currentRound() { 
        RoundManager* inst = instance();
        return inst->_currentRound; 
    }

    static int lastRound() { 
        RoundManager* inst = instance();
        return inst->_lastRound;
    }

    void setLastRound(int lastRound) { _lastRound = lastRound;}
    void setCurrentRound(int currentRound) {_currentRound = currentRound;}
    void incrementRound() {++_currentRound;}
    void increaseRound(int val) {_currentRound += val;}
};

} // end namespace quantas

#endif // RoundManager_hpp
