/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LogWriter_hpp
#define LogWriter_hpp

#include <string>
#include <iostream>
#include "../Common/Json.hpp"

namespace quantas {

    using nlohmann::json;
    using std::ostream;
    using std::cout;
    using std::endl;

    class LogWriter {
    protected:
        ostream* _log = &cout;
        int         _round = 0;
        int         _test = 0;

    public:
        static LogWriter*  instance () {
            static LogWriter s;
            return &s;
        }

        void print () {
            *_log << data.dump(4);
            *_log << endl;
            data.clear();
        }

        json        data;
        void                setLog          (ostream& out)  { _log = &out; }
        ostream*            getLog          ()const         { return _log; }
        void                setTest         (int test)      { _test = test; }
        int                 getTest         ()const         { return _test; }
        void                setRound        (int round)     { _round = round; }
        int                 getRound        ()const         { return _round; }
        static json&        getTestLog      ()              { return instance()->data["tests"][instance()->getTest()]; }

    private:
        // copying and creation prohibited by clients
        LogWriter(){}
        LogWriter(const LogWriter&){}
    };

}

#endif // LogWriter_hpp
