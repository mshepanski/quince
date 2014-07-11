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

#include <quince/exceptions.h>
#include <quince/detail/abstract_column_sequence.h>
#include <quince/mappers/detail/column_mapper.h>

using std::vector;


namespace quince {

column_set
abstract_column_sequence::columns() const {
    column_set result;
    for_each_column([&](const column_mapper &c) {
        result.insert(&c);
    });
    return result;
}

column_id_set
abstract_column_sequence::column_ids() const {
    column_id_set result;
    for_each_column([&](const column_mapper &c) {
        result.insert(c.id());
    });
    return result;
}

const column_mapper &
abstract_column_sequence::only_column() const {
    const column_mapper *found = nullptr;
    for_each_column([&](const column_mapper &c) {
        if (found != nullptr)  throw multi_column_exception();
        found = &c;
    });
    if (found == nullptr)  throw missing_column_exception();
    return *found;
}

size_t
abstract_column_sequence::size() const {
    size_t result = 0;
    for_each_column([&](const column_mapper &c) {
        result++;
    });
    return result;
}

}