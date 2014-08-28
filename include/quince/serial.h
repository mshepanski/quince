#ifndef QUINCE__serial_h
#define QUINCE__serial_h

/*
    Copyright 2014 Michael Shepanski

    This file is part of the quince library.

    Quince is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Quince is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with quince.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <stdint.h>
#include <boost/optional.hpp>


namespace quince {

// serial is a C++ representation of the type that the DBMS uses for automatically-assigned primary keys.
//
class serial {
public:
    bool
    operator==(const serial &that) const {
        return _value == that._value;
    }

    void clear()            { _value = boost::none; }
    void assign(int64_t v)  { _value = v; }
    bool has_value() const  { return bool(_value); }
    int64_t value() const   { return *_value; }

    boost::optional<serial> to_optional() const {
        if (has_value())
            return *this;
        else
            return boost::none;
    }

    friend std::ostream &
    operator<<(std::ostream &os, const serial &s) {
        return s.has_value()
            ? os << s.value()
            : os << "<null>";
    }

private:
    boost::optional<int64_t> _value;
};

}

#endif
