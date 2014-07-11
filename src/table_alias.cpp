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
#include <string>

#include <quince/query.h>
#include <quince/table_alias.h>
#include <quince/detail/sql.h>

using std::string;


namespace quince {

table_alias_base::~table_alias_base()
{}

void
table_alias_base::write_table_reference(sql &cmd) const {
    _table.write_table_reference(cmd);
    cmd.write(" AS " + _name);
}

column_id_set
table_alias_base::aliased_columns() const {
    return {};
}

void
table_alias_base::for_each_column(std::function<void(const column_mapper &)> op) const {
    _value_mapper.for_each_column(op);
}

namespace {
    std::atomic<int_fast64_t> counter(0);
}

string
table_alias_base::generate_name() {
    return "a$" + std::to_string(++counter);
}

}