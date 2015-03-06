#ifndef QUINCE__detail__abstract_column_sequence_h
#define QUINCE__detail__abstract_column_sequence_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <functional>
#include <quince/detail/column_id.h>


/*
    Everything in this file is for quince internal use only.
*/

namespace quince {

class column_mapper;
typedef std::set<const column_mapper *> column_set;

class abstract_column_sequence {
public:
    virtual ~abstract_column_sequence()  {}

    virtual void for_each_column(std::function<void(const column_mapper &)>) const = 0;

    column_set columns() const;
    column_id_set column_ids() const;
    const column_mapper &only_column() const;  // throws multi_column_exception() or missing_column_exception(), or returns
    size_t size() const;
};

}

#endif
