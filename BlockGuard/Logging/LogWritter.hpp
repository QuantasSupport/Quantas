//
// LogWritter
// BlockGuard
//
// Logs for Blockguard are kept as a JSON file to make it easer to parse and record metrics from a log
// This class handles logging to specific location in the log (path). It holds a reference to a log file (see LogFile.hpp) as a static varable. 
// each instance of LogWritter will thus write to the same file. However diffrent Writers will be configured to write at diffrent paths. This 
// prevents repeation of a JSON path and helps to maintain consistency (helps prevent a refactor from breaking the logging). 
//
// It is responsible for 
// * creating/destoying the LogFile obj
// * tracking the current path that JSON objects should be written
// * passing objects to the LogFile object to be written



#ifndef LogWritter_hpp
#define LogWritter_hpp

#include <string>
#include "Logging/LogFile.hpp"
#include "Common/Json.hpp"

namespace blockguard{

    using std::string;
    using nlohmann::json;

    class LogWritter{
    protected:

        string pathToWriteTo;


    public:
        LogWritter              ();
        LogWritter              (string jsonPath);
        LogWritter              (const LogWritter &);
        ~LogWritter             ();

        void    log             (json objectToWrite);
        void    setPath         (string jsonPath);
        string  getPath         () const;

    };

}

#endif // LogWritter_hpp