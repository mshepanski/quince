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