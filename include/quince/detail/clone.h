#ifndef QUINCE__mappers__detail__clone_h
#define QUINCE__mappers__detail__clone_h

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

#include <assert.h>
#include <boost/optional.hpp>
#include <memory>


/*
    Everything in this file is for quince internal use only.
*/

namespace quince {

class cloneable {
public:
    virtual std::unique_ptr<cloneable> clone_impl() const = 0;
};


template<typename T>
std::unique_ptr<T>
clone(const T &object) {
    const cloneable &base = object;
    std::unique_ptr<T> result(dynamic_cast<T*>(base.clone_impl().release()));
    assert(result);
    return result;
}

template<typename T>
std::unique_ptr<T>
clone_or_null(boost::optional<const T &> object) {
    if (object) return clone(*object);
    else        return nullptr;
}

}

#endif
