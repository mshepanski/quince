//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/sql.h>
#include <quince/detail/table_interface.h>
#include <quince/database.h>
#include <quince/exceptions.h>
#include <quince/mappers/detail/persistent_column_mapper.h>

using boost::optional;
using std::string;


namespace quince {

persistent_column_mapper::persistent_column_mapper(const optional<string> &name) :
    abstract_mapper_base(name),
    column_mapper(name),
    _table(nullptr)
{}

const table_interface &
persistent_column_mapper::table() const {
    assert(_table != nullptr);
    return *_table;
}

const string &
persistent_column_mapper::table_basename() const {
    return table().basename();
}

column_id_set
persistent_column_mapper::imports() const {
    return column_ids();
}

void
persistent_column_mapper::set_table(const table_interface *table) {
    if (optional<size_t> max = table->get_database().max_column_name_length())
        if (has_name() && name().size() >= *max)
            throw column_name_length_exception(name(), *max);
    _table = table;
}

void
persistent_column_mapper::for_each_persistent_column(std::function<void(const persistent_column_mapper &)> op) const {
    op(*this);
}

void
persistent_column_mapper::write_expression(sql &cmd) const {
    cmd.write_persistent_column(table_basename(), name());
}

void
persistent_column_mapper::check_compatibility(const database &database) const {
    if (_table == nullptr)  return;  // exemption for mappers produced by database::temporary_mapper()

    if (database != _table->get_database())  throw cross_database_query_exception();
}

}