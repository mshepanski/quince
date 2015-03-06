#ifndef QUINCE__serial_h
#define QUINCE__serial_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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
