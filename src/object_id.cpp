//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <atomic>
#include <quince/detail/object_id.h>


namespace quince {

namespace {
    std::atomic<int_fast64_t> counter(0);
}

object_id::object_id() :
    _id(++counter)
{}

object_id::object_id(const object_id &) :
    object_id()
{}

object_id &
object_id::operator=(const object_id &) {
    _id = ++counter;
    return *this;
}

uint64_t
object_id::get() const {
    return _id;
}

}