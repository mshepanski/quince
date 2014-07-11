#ifndef QUINCE__detail__abstract_column_sequence_h
#define QUINCE__detail__abstract_column_sequence_h

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
