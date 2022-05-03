//
// LogWritter
// BlockGuard
//
// Logs for Blockguard are kept as a JSON file to make it easer to parse and record metrics from a log
// This class holds the records to be logged

#ifndef LogWritter_hpp
#define LogWritter_hpp

#include <string>
#include <iostream>
#include "../Common/Json.hpp"

namespace blockguard{

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
        LogWritter() {} // private so that it cannot be invoked

        // copying prohibited
        LogWritter(const LogWritter&) {}
        LogWritter& operator=(const LogWritter&) {}

        // pointer to its instance
        static LogWritter* onlyInstanceInt_;
    };

}

#endif // LogWritter_hpp