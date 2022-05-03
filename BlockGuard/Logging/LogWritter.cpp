
#include "LogWritter.hpp"

namespace blockguard {

    LogWritter* LogWritter::onlyInstanceInt_ = nullptr;

    // returns pointer to instance of singleton
    LogWritter* LogWritter::instance() {
        if (onlyInstanceInt_ == nullptr)
            onlyInstanceInt_ = new LogWritter;
        return onlyInstanceInt_;
    }

    void LogWritter::print() {
        *_log << data.dump(4);
        *_log << endl;
        data.clear();
    }


}