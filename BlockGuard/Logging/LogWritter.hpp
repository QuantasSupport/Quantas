/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LogWritter_hpp
#define LogWritter_hpp

#include <string>
#include <iostream>
#include "../Common/Json.hpp"

namespace quantas {

    using nlohmann::json;
    using std::ostream;
    using std::cout;
    using std::endl;

    class LogWritter{
    protected:
        ostream* _log = &cout;
        int         _round = 0;
        int         _test = 0;

    public:
        json        data;
        
        static LogWritter*  instance ();

        void                print           ();

        void                setLog          (ostream& out)  { _log = &out; };
        ostream*            getLog          ()const         { return _log; };
        void                setTest         (int test)      { _test = test; };
        int                 getTest         ()const         { return _test; };
        void                setRound        (int round)     { _round = round; };
        int                 getRound        ()const         { return _test; };

    private:
        // initialization is prohibited 
        LogWritter() = default; // private so that it cannot be invoked

        // copying prohibited
        LogWritter(const LogWritter&) = delete;
        LogWritter& operator=(const LogWritter&) = delete;

        // pointer to its instance
        static LogWritter* onlyInstanceInt_;
    };

}

#endif // LogWritter_hpp
