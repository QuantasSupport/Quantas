#ifndef LogWriter_hpp
#define LogWriter_hpp

#include <string>
#include <fstream>
#include <iostream>
#include <mutex>
#include "Json.hpp"

namespace quantas {

    using nlohmann::json;

    class LogWriter {
    public:
        static LogWriter* instance() {
            static LogWriter s;
            return &s;
        }

        // Set log file path and open stream
        static void setLogFile(const std::string& path) {
            LogWriter* inst = instance();
            std::lock_guard<std::mutex> lock(inst->_mutex);

            if (inst->_file_stream.is_open()) {
                inst->_file_stream.close();
            }

            if (path == "cout") {
                inst->_log_stream = &std::cout;
                return;
            }

            inst->_file_stream.open(path);

            if (inst->_file_stream.is_open()) {
                inst->_log_stream = &inst->_file_stream;
            } else {
                std::cerr << "[LogWriter] Failed to open log file: " << path << ". Falling back to std::cout.\n";
                inst->_log_stream = &std::cout;
            }
        }

        static void print() {
            LogWriter* inst = instance();
            std::lock_guard<std::mutex> lock(inst->_mutex);
            if (inst->_log_stream != nullptr) {
                (*inst->_log_stream) << inst->data.dump(4) << std::endl;
                inst->_log_stream->flush();
            }
            inst->data.clear();
            if (inst->_file_stream.is_open()) {
                inst->_file_stream.close();
            }
            inst->_log_stream = nullptr;
        }

        static void setTest(int test) {
            LogWriter* inst = instance();
            std::lock_guard<std::mutex> lock(inst->_mutex);
            inst->_test = test;
        }

        static int getTest() {
            LogWriter* inst = instance();
            std::lock_guard<std::mutex> lock(inst->_mutex);
            return inst->_test;
        }

        template <typename T>
        static void pushValue(const std::string& key, const T& val) {
            LogWriter* inst = instance();
            std::lock_guard<std::mutex> lock(inst->_mutex);
            inst->data["tests"][inst->_test][key].push_back(val);
        }

        template <typename T>
        static void setValue(const std::string& key, const T& val) {
            LogWriter* inst = instance();
            std::lock_guard<std::mutex> lock(inst->_mutex);
            inst->data[key] = val;
        }

    private:
        std::ofstream _file_stream;
        std::ostream* _log_stream = nullptr;
        int _test = 0;
        json data;
        mutable std::mutex _mutex;

        // disallow copies
        LogWriter() = default;
        LogWriter(const LogWriter&) = delete;
        LogWriter& operator=(const LogWriter&) = delete;
    };

} // namespace quantas

#endif // LogWriter_hpp
