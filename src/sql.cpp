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

#include <assert.h>
#include <string>
#include <vector>
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <quince/detail/abstract_query.h>
#include <quince/detail/binomen.h>
#include <quince/detail/compiler_specific.h>
#include <quince/detail/column_type.h>
#include <quince/detail/junction.h>
#include <quince/detail/query_base.h>
#include <quince/mappers/detail/persistent_column_mapper.h>
#include <quince/exprn_mappers/detail/lexicographic_comparison.h>
#include <quince/detail/sql.h>
#include <quince/query.h>
#include <quince/table.h>

using boost::format;
using boost::optional;
using std::pair;
using std::string;
using std::make_unique;
using std::to_string;
using std::unique_ptr;
using std::vector;


namespace quince {

sql::sql(const database &db) :
    _database(&db),
    _input(&db),
    _wanted_aliases(universalizable_column_id_set::universal()),
    _implicit_table(boost::none),
    _next_subquery_alias(0)
{}

void
sql::write(const string &s) {
    _text.append(s);
}

void
sql::write_quoted(const string &str) {
    write('"' + str + '"');
}

void
sql::write_quoted(const binomen &binomen) {
    if (binomen._enclosure) {
        write_quoted(*binomen._enclosure);
        write(".");
    }
    write_quoted(binomen._local);
}

void
sql::write_next_subquery_alias() {
    write("q$" + to_string(++_next_subquery_alias));
}

void
sql::write_alter_table(const binomen &table) {
    write("ALTER TABLE ");
    write_quoted(table);
    write(" ");
}

void
sql::write_create_table(
    const binomen &table,
    const abstract_mapper_base &value_mapper,
    const abstract_mapper_base &key_mapper,
    optional<column_id> generated_key,
    const vector<foreign_spec> &foreign_specs
) {
    write("CREATE TABLE ");
    write_quoted(table);
    write(" (");
    write_titles(value_mapper, generated_key);
    write(", PRIMARY KEY ");
    write_parenthesized_persistent_column_list(key_mapper);
    for (const foreign_spec &fs: foreign_specs) {
        write(", FOREIGN KEY ");
        write_parenthesized_persistent_column_list(*fs._foreign);
        write(" REFERENCES ");
        write_quoted(fs._target_table);
        write_parenthesized_persistent_column_list(*fs._target_mapper);
    }
    write(")");
}

void
sql::write_unconstrain_as_not_null(const binomen &table, const abstract_mapper_base &mapper) {
    write_alter_table(table);
    comma_separated_list_scope list_scope(*this);
    mapper.for_each_persistent_column([&](const persistent_column_mapper &p) {
        list_scope.start_item();
        write("ALTER COLUMN ");
        write_quoted(p.name());
        write(" DROP NOT NULL");
    });
}

void
sql::write_constrain_as_not_null_where_appropriate(const binomen &table, const abstract_mapper_base &mapper) {
    write_alter_table(table);
    comma_separated_list_scope list_scope(*this);
    mapper.for_each_persistent_column([&](const persistent_column_mapper &p) {
        list_scope.start_item();
        write("ALTER COLUMN ");
        write_quoted(p.name());
        write(p.can_be_all_null() ? " DROP" : " SET");
        write(" NOT NULL");
    });
}

void
sql::write_begin_transaction() {
    write("BEGIN");
}

void
sql::write_commit_transaction() {
    write("COMMIT");
}

void
sql::write_rollback_transaction() {
    write("ROLLBACK");
}

void
sql::write_mark_save_point(const string &save_point) {
    write("SAVEPOINT " + save_point);
}

void
sql::write_forget_save_point(const string &save_point) {
    write("RELEASE SAVEPOINT " + save_point);
}

void
sql::write_revert_to_save_point(const string &save_point) {
    write("ROLLBACK TO " + save_point);
}

void
sql::write_evaluation(const column_mapper &column) {
    if (alias_is_defined(column.id()))
        write(column.alias());
    else
        column.write_expression(*this);
}

void
sql::write_value(const cell &value) {
    write(next_placeholder());
    attach_cell(value);
}

void
sql::write_persistent_column(const string &table_name, const string &column_name) {
    if (_implicit_table)
        assert(table_name == *_implicit_table);
    else {
        write_quoted(table_name);
        write(".");
    }
    write_quoted(column_name);
}

void
sql::write_function_call(const string &function_name, const vector<const column_mapper *> &args) {
    // If function_name contains caps then it must be quoted.  Otherwise it doesn't
    // need to be quoted and, in case of certain built-ins may need not to be.
    //
    bool got_cap = false;
    for (char c: function_name) if (isupper(c)) got_cap = true;
    if (got_cap)
        write(function_name);
    else
        write_quoted(function_name);
    
    write("(");
    comma_separated_list_scope list_scope(*this);
    for (const auto arg: args) {
        list_scope.start_item();
        write_evaluation(*arg);
    }
    write(")");
}

void
sql::write_prefix_exprn(const string &op, const column_mapper &operand) {
    write(op + "(");
    write_evaluation(operand);
    write(")");
}

void
sql::write_postfix_exprn(const column_mapper &operand, const string &op) {
    write("(");
    write_evaluation(operand);
    write(")" + op);
}

void
sql::write_infix_exprn(const column_mapper &lhs, const string &op, const column_mapper &rhs) {
    write("(");
    write_evaluation(lhs);
    write(")" + op + "(");
    write_evaluation(rhs);
    write(")");
}

void
sql::write_cast(const column_mapper &arg, const column_type cast_type) {
    write("CAST (");
    write_evaluation(arg);
    write(" AS " + column_type_name(cast_type) + ")");
}

void
sql::write_case(
    optional<const column_mapper &> switch_,
    const vector<pair<const column_mapper *, const column_mapper *>> &clauses,
    optional<const column_mapper &> default_
)
{
    write("CASE ");
    if (switch_)  write_evaluation(*switch_);
    for (const auto &c: clauses) {
        write(" WHEN ");
        write_evaluation(*c.first);
        write(" THEN ");
        write_evaluation(*c.second);
    }
    if (default_) {
        write(" ELSE ");
        write_evaluation(*default_);
    }
    write(" END");
}

void
sql::write_scalar_subquery(const query_base &query) {
    write_subquery_exprn(query);
}

void
sql::write_subquery_exists(const query_base &query) {
    write("EXISTS");
    write_subquery_exprn(query);
}

void
sql::write_in(const column_mapper &lhs, const vector<const column_mapper *> &rhs) {
    write_evaluation(lhs);
    write(" IN (");
    comma_separated_list_scope list_scope(*this);
    for (const auto r: rhs) {
        list_scope.start_item();
        write_evaluation(*r);
    }
    write(")");
}

void
sql::write_in(const column_mapper &lhs, const query_base &rhs) {
    write_evaluation(lhs);
    write(" IN ");
    write_subquery_exprn(rhs);
}

void
sql::write_are_all_null(const abstract_column_sequence &cols) {
    bool begun = false;
    cols.for_each_column([&](const column_mapper &c) {
        if (begun) {
            write (" AND ");
            begun = true;
        }
        write_evaluation(c);
        write (" IS NULL");
    });
}

void
sql::write_intrinsic_comparison(relation r, const abstract_column_sequence &lhs, const abstract_column_sequence &rhs) {
    vector<thunk> lhs_thunks;
    lhs.for_each_column([&](const column_mapper &c) {
        lhs_thunks.push_back([&](sql &cmd) {
            cmd.write_evaluation(c);
        });
    });

    vector<thunk> rhs_thunks;
    rhs.for_each_column([&](const column_mapper &c) {
        rhs_thunks.push_back([&](sql &cmd) {
            cmd.write_evaluation(c);
        });
    });

    write_lexicographic_comparison(r, lhs_thunks, rhs_thunks);
}

void
sql::write_extrinsic_comparison(relation r, const abstract_column_sequence &lhs, const row &rhs) {
    vector<thunk> lhs_thunks;
    vector<thunk> rhs_thunks;
    lhs.for_each_column([&](const column_mapper &c) {
        lhs_thunks.push_back([&](sql &cmd) {
            cmd.write_evaluation(c);
        });

        const string placeholder = next_placeholder();
        rhs_thunks.push_back([=](sql &cmd) {
            write(placeholder);
        });
        attach_cell(*rhs.find_cell(c.name()));
    });

    write_lexicographic_comparison(r, lhs_thunks, rhs_thunks);
}

void
sql::write_collective_comparison(relation r, const abstract_column_sequence &lhs, const collective_base &rhs) {
    throw unsupported_exception();
}

void
sql::write_where(const abstract_predicate &pred) {
    assert (! a_priori_true(pred));
    write(" WHERE ");
    write_evaluation(predicate(pred));
}

void
sql::write_distinct() {
    write("DISTINCT ");
}

void
sql::write_returning(const abstract_mapper_base &mapper) {
    throw unsupported_exception();
}

void
sql::write_limit(uint32_t n) {
    write(" LIMIT " + to_string(n));
}

void
sql::write_offset(uint32_t n) {
    write(" OFFSET " + to_string(n));
}

void
sql::write_group_by(const vector<const abstract_mapper_base *> &group_by) {
    write(" GROUP BY ");
    comma_separated_list_scope list_scope(*this);
    for (const abstract_mapper_base *g: group_by)
        g->for_each_column([&](const column_mapper &c) {
            list_scope.start_item();
            write_evaluation(c);
        });
}

void
sql::write_ordered_by(const vector<const abstract_mapper_base *> &orders) {
    write(" ORDER BY ");
    comma_separated_list_scope list_scope(*this);
    for (const abstract_mapper_base *o: orders) {
        const auto pair = o->dissect_as_order_specification();
        const abstract_mapper_base * const mapper = pair.first;
        bool invert = pair.second;

        mapper->for_each_column([&](const column_mapper &c) {
            list_scope.start_item();
            write_evaluation(c);
            if (invert)  write(" DESC");
            if (c.can_be_all_null())  write_nulls_low(invert);
        });
    }
}

void
sql::write_insert(
    const binomen &table,
    const abstract_mapper_base &value_mapper,
    optional<column_id> excluded
) {
    write("INSERT INTO ");
    write_quoted(table);
    write_parenthesized_persistent_column_list(value_mapper, excluded);
}

void
sql::write_select_none(const binomen &table) {
    write("SELECT * FROM ");
    write_quoted(table);
    write(" WHERE FALSE");
}

void
sql::write_values(
    const abstract_mapper_base &value_mapper,
    const row &data,
    optional<column_id> excluded
) {
    write(" VALUES (");
    comma_separated_list_scope list_scope(*this);
    value_mapper.for_each_persistent_column(
        [&](const persistent_column_mapper &p) {
            if (p.id() != excluded) {
                list_scope.start_item();
                write_value(*data.find_cell(p.name()));
            }
        }
    );
    write(")");
}

void
sql::write_cross_join(const vector<const abstract_query_base *> &joinees) {
    assert(! joinees.empty());
    auto iter = joinees.begin();
    (*iter++)->write_table_reference(*this);
    while (iter != joinees.end()) {
        write(" CROSS JOIN ");
        (*iter++)->write_table_reference(*this);
    }
}

void
sql::write_qualified_join(
    const abstract_query_base &lhs,
    const abstract_query_base &rhs,
    conditional_junction_type type
) {
    lhs.write_table_reference(*this);

    switch(type) {
        case conditional_junction_type::inner:  write(" INNER"); break;
        case conditional_junction_type::left:   write(" LEFT"); break;
        case conditional_junction_type::right:  write(" RIGHT"); break;
        case conditional_junction_type::full:   write(" FULL"); break;
        default:                                abort();
    }
    write(" JOIN ");

    rhs.write_table_reference(*this);
}

void
sql::write_combination(combination_type type, bool all, const query_base &rhs) {
    switch (type) {
        case combination_type::union_:      write(" UNION "); break;
        case combination_type::intersect:   write(" INTERSECT "); break;
        case combination_type::except:      write(" EXCEPT "); break;
        default:                            abort();
    }
    if (all)  write(" ALL ");
    if (rhs.is_combined()) write("(");
    rhs.write_maximal_select(*this);
    if (rhs.is_combined()) write(")");
}

void
sql::write_on(const abstract_predicate &pred) {
    write(" ON ");
    write_evaluation(predicate(pred));
}

void
sql::write_update(
    const binomen &table,
    const abstract_mapper_base &dest,
    const abstract_mapper_base &src,
    optional<column_id> excluded
) {
    vector<const column_mapper *> src_columns;
    src.for_each_column([&](const column_mapper &c) {
        src_columns.push_back(&c);
    });
    auto src_column_iter = src_columns.begin();

    write("UPDATE ");
    write_quoted(table);
    write(" SET ");
    comma_separated_list_scope list_scope(*this);
    dest.for_each_persistent_column([&](const persistent_column_mapper &p) {
        if (p.id() != excluded) {
            list_scope.start_item();
            write_quoted(p.name());
            write(" = ");
            write_evaluation(**src_column_iter++);
        }
    });
}

void
sql::write_update(
    const binomen &table,
    const abstract_mapper_base &dest,
    const row &src,
    optional<column_id> excluded
) {
    // TODO: factor out the code in common with the other overload
    //
    write("UPDATE ");
    write_quoted(table);
    write(" SET ");
    comma_separated_list_scope list_scope(*this);
    dest.for_each_persistent_column([&](const persistent_column_mapper &p) {
        if (p.id() != excluded) {
            list_scope.start_item();
            write_quoted(p.name());
            write(" = ");
            write_value(*src.find_cell(p.name()));
        }
    });
}

void
sql::write_delete_from(const binomen &table) {
    write("DELETE FROM ");
    write_quoted(table);
}

void
sql::write_set_search_path(const string &schema_name) {
    write("SET search_path TO ");
    write_quoted(schema_name);
}

void
sql::write_rename_table(const binomen &old, const string &new_local) {
    write_alter_table(old);
    write("RENAME TO ");
    write_quoted(new_local);
}

void
sql::write_add_columns(
    const binomen &table,
    const abstract_mapper_base &mapper,
    optional<column_id> generated_key
) {
    throw unsupported_exception();
}

void
sql::write_drop_columns(const binomen &table, const abstract_mapper_base &mapper) {
    throw unsupported_exception();
}

void
sql::write_rename_column(const binomen &table, const string &before, const string &after) {
    throw unsupported_exception();
}

void
sql::write_set_columns_types(
    const binomen &table,
    const abstract_mapper_base &mapper,
    optional<column_id> generated_key
) {
    throw unsupported_exception();
}

void
sql::write_drop_table(const binomen &table) {
    write("DROP TABLE ");
    write_quoted(table);
}

void
sql::write_drop_table_if_exists(const binomen &table) {
    write("DROP TABLE IF EXISTS ");
    write_quoted(table);
}

void
sql::attach_cell(const cell &c) {
    _input.add_cell(c);
}

bool
sql::alias_is_wanted(column_id id) const {
    return _wanted_aliases.count(id) != 0;
}

bool
sql::alias_is_defined(column_id r) const {
    return _aliased_columns.count(r) != 0;
}

sql::marker_t
sql::here() const {
    return _text.length();
}

sql::text_insertion_scope::text_insertion_scope(sql &cmd, marker_t insertion_point) :
    _command(cmd),
    _pending_tail(_command._text.substr(insertion_point))
{
    _command._text.erase(insertion_point);
}

sql::text_insertion_scope::~text_insertion_scope()
{
    _command._text.append(_pending_tail);
}


sql::comma_separated_list_scope::comma_separated_list_scope(sql &cmd) :
    _command(cmd),
    _begun(false)
{}

void
sql::comma_separated_list_scope::start_item() {
    if (_begun)
        _command.write(", ");
    else
        _begun = true;
}

bool
sql::comma_separated_list_scope::has_begun() const {
    return _begun;
}


sql::wanted_aliases_scope::wanted_aliases_scope(sql &cmd, const universalizable_column_id_set &wanted) :
    wanted_aliases_scope(cmd)
{
    _command._wanted_aliases = wanted;
}

sql::wanted_aliases_scope::~wanted_aliases_scope() {
    _command._wanted_aliases = _pending;
}

sql::wanted_aliases_scope::wanted_aliases_scope(sql &cmd) :
    _command(cmd),
    _pending(cmd._wanted_aliases)
{}


sql::additional_wanted_aliases_scope::additional_wanted_aliases_scope(
    sql &cmd,
    const universalizable_column_id_set &additional_wanted
) :
    wanted_aliases_scope(cmd)
{
    cmd._wanted_aliases.insert(additional_wanted);
}



sql::aliased_columns_scope::aliased_columns_scope(sql &cmd, const column_id_set &aliaseds) :
    aliased_columns_scope(cmd)
{
    _command._aliased_columns = aliaseds;
}
    
sql::aliased_columns_scope::~aliased_columns_scope() {
    _command._aliased_columns = _pending;
}

sql::aliased_columns_scope::aliased_columns_scope(sql &cmd) :
    _command(cmd),
    _pending(cmd._aliased_columns)
{}


sql::additional_aliased_columns_scope::additional_aliased_columns_scope(
    sql &cmd,
    const column_id_set &additional_aliaseds
) :
    aliased_columns_scope(cmd)
{
    add_to_set(cmd._aliased_columns, additional_aliaseds);
}

sql::expression_restriction_scope::expression_restriction_scope(sql &cmd, const string &implicit_table) :
    _command(cmd)
{
    assert(! _command._implicit_table);
    _command._implicit_table = implicit_table;
}

sql::expression_restriction_scope::~expression_restriction_scope() {
    _command._implicit_table = boost::none;
}


const string sql::unary_plus_operator = "+";
const string sql::unary_minus_operator = "-";


void
sql::write_select_list(const abstract_column_sequence &column_sequence) {
    comma_separated_list_scope list_scope(*this);
    column_sequence.for_each_column([&](const column_mapper &c) {
        if (alias_is_wanted(c.id())) {
            list_scope.start_item();
            if (alias_is_defined(c.id()))
                write(c.alias());
            else {
                write_evaluation(c);
                write(" AS " + c.alias());
            }
        }
    });
    if (! list_scope.has_begun())
        // Rare edge case: the whole select statement is for nothing.
        // Make a minimal select list, just for SQL syntax compliance.
        write("1");
}

void
sql::write_parenthesized_persistent_column_list(const abstract_mapper_base &mapper, optional<column_id> excluded) {
    write("(");
    comma_separated_list_scope list_scope(*this);
    mapper.for_each_persistent_column(
        [&](const persistent_column_mapper &p) {
            if (p.id() != excluded) {
                list_scope.start_item();
                write_quoted(p.name());
            }
        }
    );
    write(")");
}

void
sql::write_title(const persistent_column_mapper &pcm, optional<column_id> generated_key) {
    const bool is_generated = generated_key == pcm.id();

    write_quoted(pcm.name());
    write(" ");
    write(column_type_name(pcm.get_column_type(is_generated)));
}

void
sql::write_titles(const abstract_mapper_base &mapper, optional<column_id> generated_key) {
    comma_separated_list_scope list_scope(*this);
    mapper.for_each_persistent_column([&](const persistent_column_mapper &p) {
        list_scope.start_item();
        write_title(p, generated_key);
        if (! p.can_be_all_null())  write(" NOT NULL");
    });
}


void
sql::write_simple_comparison(const string &op, thunk lhs, thunk rhs) {
    lhs(*this);
    write(op);
    rhs(*this);
}

void
sql::write_flat_lexicographic_comparison(
    const string &compare_op,
    const string &logic_op,
    const vector<thunk> &lhs,
    const vector<thunk> &rhs
) {
    const size_t n = lhs.size();
    assert(rhs.size() == n);
    for (size_t i = 0; i < n; i++) {
        write_simple_comparison(compare_op, lhs[i], rhs[i]);
        if (i < n-1)  write(logic_op);
    }
}

void
sql::write_nesting_lexicographic_comparison(
    const string &compare_op,
    const string &strict_compare_op,
    const vector<thunk> &lhs,
    const vector<thunk> &rhs
) {
    const size_t n = lhs.size();
    assert(rhs.size() == n);
    size_t i;
    for (i = 0; i < n-1; i++) {
        write_simple_comparison(strict_compare_op, lhs[i], rhs[i]);
        write(" OR ");
        write_simple_comparison(compare_op, lhs[i], rhs[i]);
        write(" AND (");
    }
    write_simple_comparison(compare_op, lhs[i], rhs[i]);
    while (i --> 0)
        write(")");
}

void
sql::write_lexicographic_comparison(relation r, const vector<thunk> &lhs, const vector<thunk> &rhs) {
    const string op = relop(r);
    switch (r) {
        case relation::equal:       return write_flat_lexicographic_comparison(op, " AND ", lhs, rhs);
        case relation::not_equal:   return write_flat_lexicographic_comparison(op, " OR ", lhs, rhs);
        default:                    return write_nesting_lexicographic_comparison(op, strict_relop(r), lhs, rhs);
    }
}

void
sql::write_subquery_exprn(const query_base &query) {
    assert(!_implicit_table);
    write("(");
    query.write_maximal_select(*this);
    write(")");
}

string
sql::column_type_name(column_type t) const {
    return _database->column_type_name(t);
}

string
sql::relop(relation r) {
    switch (r) {
        case relation::equal:               return " = ";
        case relation::not_equal:           return " != ";
        case relation::greater:             return " > ";
        case relation::less:                return " < ";
        case relation::greater_or_equal:    return " >= ";
        case relation::less_or_equal:       return " <= ";
        default:                            abort();
    }
}

string
sql::strict_relop(relation r) {
    switch (r) {
        case relation::greater:             return " > ";
        case relation::less:                return " < ";
        case relation::greater_or_equal:    return " > ";
        case relation::less_or_equal:       return " < ";
        default:                            abort();
    }
}

}
