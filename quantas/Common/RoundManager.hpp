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

#include <chrono>

namespace quantas {

class RoundManager {
private:
    size_t _currentRound{0};
    size_t _lastRound{0};
    bool _synchronous{true};
    std::chrono::steady_clock::time_point _start_time;

    // Private constructor and copy operations to enforce singleton usage:
    RoundManager() {
        _start_time = std::chrono::steady_clock::now();
    };
    RoundManager(const RoundManager&) = delete;
    RoundManager& operator=(const RoundManager&) = delete;

public:
    static RoundManager* instance() {
        static RoundManager s;
        return &s;
    }

    static size_t currentRound() { 
        RoundManager* inst = instance();
        if (inst->_synchronous) {
            return inst->_currentRound;
        } else {
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - inst->_start_time);
            return static_cast<size_t>(duration.count());
        }
    }

    static size_t lastRound() { 
        RoundManager* inst = instance();
        return inst->_lastRound;
    }

    static void setLastRound(size_t lastRound) { 
        RoundManager* inst = instance();
        inst->_lastRound = lastRound;
    }
    static void setCurrentRound(size_t currentRound) {
        RoundManager* inst = instance();
        inst->_currentRound = currentRound;
    }
    static void incrementRound() {
        RoundManager* inst = instance();
        ++(inst->_currentRound);
    }
    static void increaseRound(size_t val) {
        RoundManager* inst = instance();
        inst->_currentRound += val;
    }
    static void asynchronous() {
        RoundManager* inst = instance();
        inst->_synchronous = false;
    }
};

} // end namespace quantas

#endif // RoundManager_hpp
