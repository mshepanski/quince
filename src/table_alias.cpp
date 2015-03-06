//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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