//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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