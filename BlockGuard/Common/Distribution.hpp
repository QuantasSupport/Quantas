/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

// This class handles the destitution of channel delays in the network. The distribution can be uniform, Poisson or one. 


#ifndef Distribution_hpp
#define Distribution_hpp

#include <string>
#include <random>
#include <iostream>
#include "./../Common/Peer.hpp"
#include "Json.hpp"

namespace blockguard{

    using std::string;
    using std::uniform_int_distribution;
    using std::poisson_distribution;
    using nlohmann::json;
    using std::cerr;

    static const string                POISSON = "POISSON";
    static const string                UNIFORM = "UNIFORM";
    static const string                ONE     = "ONE";
    class Distribution {
    private:
        int                                 _avgDelay = 1;
        int                                 _maxDelay = 1;
        int                                 _minDelay = 1;
        string                              _type = ONE;

    public:
        Distribution                                                 ();
        Distribution                                                 (const Distribution&);
        ~Distribution                                                ();

        // setters
        
        void                                setDistribution     (json distribution);

        // getters
        int                                 maxDelay            ()const                                         {return _maxDelay;};
        int                                 avgDelay            ()const                                         {return _avgDelay;};
        int                                 minDelay            ()const                                         {return _minDelay;};
        string                              type                ()const                                         {return _type;};
        int                                 getDelay();

    };

    
    Distribution::Distribution(){
        _avgDelay = 1;
        _maxDelay = 1;
        _minDelay = 1;
        _type = UNIFORM;
    }

    Distribution::Distribution(const Distribution &rhs){
        if(this == &rhs){
            return;
        }

        _avgDelay = rhs._avgDelay;
        _maxDelay = rhs._maxDelay;
        _minDelay = rhs._minDelay;
        _type = rhs._type;
    }

    Distribution::~Distribution(){
        
    }

    void Distribution::setDistribution(json distribution) {
        if (distribution.contains("avgDelay")) {
            _avgDelay = distribution["avgDelay"];
        }
        if (distribution.contains("maxDelay")) {
            _maxDelay = distribution["maxDelay"];
        }
        if (distribution.contains("minDelay")) {
            _minDelay = distribution["minDelay"];
        }

        if (distribution.contains("type")) {
            string type = distribution["type"];
            if (type == UNIFORM) {
                _type = UNIFORM;
            }
            else if (type == POISSON) {
                _type = POISSON;
            }
            else if (type == ONE) {
                _type = ONE;
            }
        }
    }
    
    int Distribution::getDelay(){
        int delay = -1;
        do {
            if (_type == UNIFORM) {
                uniform_int_distribution<int> randomDistribution(_minDelay, _maxDelay);
                delay = randomDistribution(RANDOM_GENERATOR);
            }
            if (_type == POISSON) {
                poisson_distribution<int> poissonDistribution(_avgDelay);
                delay = poissonDistribution(RANDOM_GENERATOR);
            }
            if (_type == ONE) {
                delay = 1;
            }
            // guard agenst 0 and negative numbers
        } while (delay < 1 || delay > _maxDelay || delay < _minDelay);

        return delay;
    }
}
#endif /* Distribution_hpp */
