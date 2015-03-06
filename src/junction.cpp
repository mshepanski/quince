//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <assert.h>
#include <quince/detail/junction.h>
#include <quince/detail/sql.h>
#include <quince/query.h>

using std::unique_ptr;


namespace quince {

void
junction_base::write_table_reference(sql &cmd) const {
    cmd.write_cross_join(_joinees);
}

column_id_set
junction_base::aliased_columns() const {
    column_id_set result;
    for (const auto j: _joinees)  add_to_set(result, j->aliased_columns());
    return result;
}

void
junction_base::for_each_column(std::function<void(const column_mapper &)> op) const {
    for (const auto j: _joinees)  j->exports().for_each_column(op);
}

bool
junction_base::a_priori_empty() const {
    for (const auto j: _joinees)  if (j->a_priori_empty())  return true;
    return false;
}

bool
junction_base::might_have_duplicate_rows() const {
    for (const auto j: _joinees)  if (j->might_have_duplicate_rows())  return true;
    return false;
}

const abstract_query_base &
junction_base::lhs() const {
    assert(_joinees.size() == 2);
    return *_joinees[0];
}

const abstract_query_base &
junction_base::rhs() const {
    assert(_joinees.size() == 2);
    return *_joinees[1];
}

const abstract_query_base &
junction_base::adopt(unique_ptr<abstract_query_base> query) {
    if (query->get_database() != _database)  throw cross_database_query_exception();

    return own(std::move(query));
}

void
conditional_junction_base::write_table_reference(sql &cmd) const {
    cmd.write_qualified_join(lhs(), rhs(), _type);

    sql::aliased_columns_scope aliases(cmd, aliased_columns());
    cmd.write_on(_predicate);
}

bool
conditional_junction_base::a_priori_empty() const {
    switch (_type) {
        case conditional_junction_type::inner:  return a_priori_false(_predicate) || lhs().a_priori_empty();
        case conditional_junction_type::left:   return lhs().a_priori_empty();
        case conditional_junction_type::right:  return rhs().a_priori_empty();
        case conditional_junction_type::full:   return lhs().a_priori_empty() && rhs().a_priori_empty();
        default:                                abort();
    }
}

}