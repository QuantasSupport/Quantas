/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include "LogWritter.hpp"

namespace quantas {

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