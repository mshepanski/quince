#ifndef QUINCE__mappers__detail__clone_h
#define QUINCE__mappers__detail__clone_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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
clone_or_null(const boost::optional<T> &object) {
    if (object) return clone(*object);
    else        return nullptr;
}

}

#endif
