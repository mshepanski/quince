//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <assert.h>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <quince/database.h>
#include <quince/detail/query_base.h>
#include <quince/detail/sql.h>
#include <quince/detail/util.h>
#include <quince/query.h>
#include <quince/table.h>
#include <quince/transaction.h>
#include <quince/exprn_mappers/operators.h>
#include <quince/transaction.h>

using boost::format;
using boost::optional;
using std::string;
using std::shared_ptr;
using std::unique_ptr;
using std::vector;


namespace quince {

struct query_base::combination {
    const combination_type _type;
    const bool _all;
    const unique_ptr<const query_base> _rhs;

    combination(combination_type type, bool all, unique_ptr<const query_base> &&rhs) :
        _type(type),
        _all(all),
        _rhs(std::move(rhs))
    {}
};


query_base::~query_base()
{}

unique_ptr<row>
query_base::fetch_row(const session &s) const {
    assert (! a_priori_empty());  // Failure means we haven't optimized well.

    const unique_ptr<sql> cmd = get_database().make_sql();
    write_maximal_select(*cmd);
    return s->exec_with_one_output(*cmd);
}

void
query_base::write_table_reference(sql &cmd) const {
    cmd.write("(");
    {
        sql::nested_select_scope nested(cmd);
        write_select(cmd);
    }
    cmd.write(") AS ");
    cmd.write_next_subquery_alias();
}

column_id_set
query_base::aliased_columns() const {
    return exports().column_ids();
}

bool
query_base::a_priori_empty() const {
    return a_priori_false(_predicate) || _from.a_priori_empty();
}

void
query_base::remove() {
    const table_base &t = table();
    t.remove_where(predicate_applicable_to(t));
}

void
query_base::remove_existing() {
    const table_base &t = table();
    t.remove_existing_where(predicate_applicable_to(t));
}

bool
query_base::remove_if_exists() {
    const table_base &t = table();
    return t.remove_if_exists_where(predicate_applicable_to(t));
}

const database &
query_base::get_database() const {
    return _database;
}

bool
query_base::might_have_duplicate_rows() const {
    if (_from.might_have_duplicate_rows())
        return true;

    const column_id_set input = _from.exports().column_ids();
    const column_id_set output = exports().column_ids();

    if (! is_subset(input, output))  return true;  // throwing away columns may turn non-duplicate rows into duplicates

    for (const auto c: _combinations)  if (c->_all)  return true;

    return false;
}

void
query_base::write_select(sql &cmd) const {
    size_t paren_depth = 0;
    if (_database.imposes_combination_precedence()) {
        // If the following assertion fails, we've added a new database class that imposes
        // combination precedence but doesn't allow us to overrule it with combination nesting.
        // Then the remedy will be to change query::real_combine() so that it wraps the lhs
        // when necessary (as it does now for the rhs).
        //
        assert(_database.supports_nested_combinations());
        for (; paren_depth+1 < _combinations.size(); paren_depth++)
            cmd.write("(");
    }
    {
        sql::additional_aliased_columns_scope aliases1(cmd, _from.aliased_columns());
        cmd.write("SELECT ");
        if (_distinct_list) {
            sql::additional_aliased_columns_scope aliases2(cmd, get_value_mapper_base().column_ids());
            cmd.write_distinct(*_distinct_list);
        }
        cmd.write_select_list(get_value_mapper_base());
    }
    {
        sql::additional_wanted_aliases_scope during_from(cmd, additional_imports());
        cmd.write(" FROM ");
        _from.write_table_reference(cmd);
    }

    sql::marker_t site_for_combinations;
    {
        sql::additional_aliased_columns_scope aliases1(cmd, _from.aliased_columns());

        if (! a_priori_true(_predicate)) {
            sql::wanted_aliases_scope during_where(cmd, universalizable_column_id_set::universal());
            cmd.write_where(_predicate);
        }

       sql::additional_aliased_columns_scope aliases2(cmd, exports().column_ids());

        if (!_group_by.empty())  cmd.write_group_by(_group_by);

        site_for_combinations = cmd.here();

        if (! a_priori_empty()) {
            const vector<const abstract_mapper_base *> orders = all_orders_hi_to_lo();
            if (!orders.empty()) {
                sql::wanted_aliases_scope during_order(cmd, universalizable_column_id_set::universal());
                cmd.write_ordered_by(orders);
            }
            if (_limit)  cmd.write_limit(*_limit);
            if (_offset != 0) {
                if (!_limit)  cmd.write_no_limit();
                cmd.write_offset(_offset);
            }
        }
    }

    if (! _combinations.empty()) {
        sql::text_insertion_scope insertion(cmd, site_for_combinations);
        for (const auto c: _combinations) {
            if (paren_depth > 0) {
                cmd.write(")");
                paren_depth--;
            }
            cmd.write_combination(c->_type, c->_all, *c->_rhs);
        }
    }
}

void
query_base::write_maximal_select(sql &cmd) const {
    sql::wanted_aliases_scope(cmd, imports());
    write_select(cmd);
}

std::string
query_base::to_string() const {
    const unique_ptr<sql> cmd = _database.make_sql();
    write_maximal_select(*cmd);
    return cmd->get_text();
}

query_base::query_base(const abstract_query_base &from) :
    _from_id(from.query_id().get()),
    _from(own(clone(from))),
    _database(_from.get_database()),
    _value_mapper_is_inherited(true),
    _predicate(true),
    _offset(0),
    _fetch_size(100)
{}

void
query_base::add_constraint(const abstract_predicate &pred) {
    assert(is_predicational());
    _predicate = _predicate && pred;
}

void
query_base::add_distinct() {
    assert(! _distinct_list);
    _distinct_list = vector<const abstract_mapper_base*>();
}

void
query_base::add_distinct_on(vector<unique_ptr<const abstract_mapper_base>> &&distincts) {
    assert(_distinct_list);
    for (auto &distinct: distincts)
        _distinct_list->push_back(&own(distinct));
}

const optional<vector<const abstract_mapper_base *>> &
query_base::distinct_list() const {
    return _distinct_list;
}

void
query_base::add_orders(vector<unique_ptr<const abstract_mapper_base>> &&orders_hi_to_lo) {
    BOOST_REVERSE_FOREACH(auto &order, orders_hi_to_lo)
        _orders_lo_to_hi.push_back(&own(order));
}

void
query_base::clear_orders() {
    // May leave some unreferenced objects in the object_owner, which could become
    // numerous if you made repeated calls to add_orders/clear_orders; but these are
    // protected functions and I trust the caller.

    _orders_lo_to_hi.clear();
}

void
query_base::set_limit(uint32_t n_rows) {
    if (!_limit || n_rows <= _limit)
        _limit = n_rows;
}

void
query_base::set_skip(uint32_t n_rows) {
    _offset += n_rows;

    if (! _limit)
        ;
    else if (*_limit > n_rows)
        *_limit -= n_rows;
    else
        *_limit = 0;
}

void
query_base::set_fetch_size(uint32_t n_rows) {
    _fetch_size = n_rows;
}

void
query_base::set_group_by(vector<unique_ptr<const abstract_mapper_base>> &&group_by) {
    // TODO: refactor so I can use the code currently in abstract_expressionist::own_all().

    for (auto &g: group_by)
        _group_by.push_back(&own(g));
}

bool
query_base::is_fake_combinable() const {
    return is_predicational() && !might_have_duplicate_rows();
}

bool
query_base::is_trivial() const {
    return is_predicational() && a_priori_true(_predicate);
}

bool
query_base::allow_fake_combine_with(const query_base &rhs) const {
    return is_fake_combinable() && rhs.is_fake_combinable() && _from_id == rhs._from_id;
}

void
query_base::add_fake_combine(combination_type type, const query_base &rhs) {
    switch (type) {
        case combination_type::union_:      _predicate = _predicate || rhs._predicate;  break;
        case combination_type::intersect:   _predicate = _predicate && rhs._predicate;  break;
        case combination_type::except:      _predicate = _predicate && !rhs._predicate; break;
        default:                            abort();
    }
}

void
query_base::add_real_combine(combination_type type, bool all, const query_base &rhs) {
    _combinations.push_back(
        &own(quince::make_unique<combination>(type, all, clone(rhs)))
    );
}

void
query_base::init_iterator(query_iterator_base &iterator) const {
    if (a_priori_empty())  return;

    const unique_ptr<sql> cmd = get_database().make_sql();
    write_maximal_select(*cmd);
    iterator.init(_database.get_session()->exec_with_stream_output(*cmd, _fetch_size));
}

void
query_base::set_value_mapper_is_inherited(bool value_mapper_is_inherited) {
    _value_mapper_is_inherited = value_mapper_is_inherited;
}

predicate
query_base::predicate_applicable_to(const table_base &t) const {
    const table_base * const from_table = dynamic_cast<const table_base *>(&_from);
    if (    from_table != nullptr
        &&  from_table->table_id() == t.table_id()
        &&  is_predicational()
       )
        return _predicate;
    else {
        predicate result(true);
        t.get_key_mapper_base().build_match_tester(*this, result);
        return result;
    }
}

bool
query_base::is_predicational() const {
    return _value_mapper_is_inherited
        && !_limit
        && _offset == 0
        && ! _distinct_list
        && _group_by.empty()
        && _combinations.empty();
}

bool
query_base::is_combined() const {
    return !_combinations.empty();
}

bool
query_base::value_mapper_is_inherited() const {
    return _value_mapper_is_inherited;
}

bool
query_base::is_valid_combination_rhs() const {
    if (_database.supports_nested_combinations())  return true;

    return !is_combined()
        && !_limit
        && _offset == 0
        && (! _distinct_list || _distinct_list->empty())
        && _orders_lo_to_hi.empty();
}

column_id_set
query_base::imports() const {
    column_id_set result = abstract_query_base::imports();
    if (result.count(wildcard_column_id))
        add_to_set(result, _from.imports());
    return result;
}

const table_base &
query_base::table() const {
    if (const table_base * const table = get_value_mapper_base()._table_whose_value_mapper_i_am)
        return *table;
    else
        throw malformed_modification_exception();
}

column_id_set
query_base::additional_imports() const {
    column_id_set result = _predicate.imports();
    for (const auto &o: all_orders_hi_to_lo())
        add_to_set(result, o->imports());
    return result;
}

vector<const abstract_mapper_base *>
query_base::all_orders_hi_to_lo() const {
    vector<const abstract_mapper_base *> result;
    if (_distinct_list)
        result.insert(result.end(), _distinct_list->begin(), _distinct_list->end());
    result.insert(result.end(), _orders_lo_to_hi.rbegin(), _orders_lo_to_hi.rend());
    return result;
}


query_iterator_base::query_iterator_base(const query_iterator_base &that) :
    _database(that._database),
    _session(that._session),
    _result_stream(that._result_stream),
    _row(that.invalidate())
{}

query_iterator_base::~query_iterator_base()
{}

query_iterator_base::query_iterator_base(const database &database) :
    _database(database),
    _session(database.get_session())
{}

void
query_iterator_base::init(const result_stream &rs) {
    _result_stream = rs;
    assert (!_row);
}

const session &
query_iterator_base::get_session() const {
    assert(_database.is_using_session(_session));
    return _session;
}

const row *
query_iterator_base::advance() {
    _row = get_session()->next_output(_result_stream);
    return _row.get();
}

std::unique_ptr<const row>
query_iterator_base::invalidate() const {
    auto &moveable = const_cast<std::unique_ptr<const row>&>(_row);
    return std::move(moveable);
}

}
