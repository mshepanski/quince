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

#include <quince/mappers/detail/column_mapper.h>

using boost::optional;
using std::string;


namespace quince {

column_mapper::column_mapper(const optional<string> &name) :
    abstract_mapper_base(name),
    _id(next_column_id()),
    _alias("r$" + std::to_string(_id))
{}

void
column_mapper::for_each_column(std::function<void(const column_mapper &)> op) const {
    op(*this);
}

}