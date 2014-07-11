#ifndef QUINCE__mappers__detail__object_id_h
#define QUINCE__mappers__detail__object_id_h

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

#include <stdint.h>


/*
    Everything in this file is for quince internal use only.
*/

namespace quince {

// object_id wraps a 64-bit counter, with every newly constructed object_id getting a unique value,
// and copies (whether by copy construction or assignment) getting unique values also.
//
// If your class has an object_id member, then you can distinguish different objects by comparing
// the values of their object_ids.  It's a lot like comparing addresses, but it's stricter in two ways:
//
//    1. When objects get destroyed, their addresses can be reused, but their object_ids won't.
//    2. When objects get overwritten by assignment, their addresses stay the same, but the
//       object_id does not.
//
class object_id {
public:
    typedef int64_t value_type;

    object_id();
    object_id(const object_id &);
    object_id &operator=(const object_id &);

    uint64_t get() const;

private:
    value_type _id;
};

}

#endif
