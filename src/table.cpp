//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <assert.h>
#include <quince/detail/sql.h>
#include <quince/table.h>
#include <quince/transaction.h>
#include <quince/mappers/detail/persistent_column_mapper.h>
#include <quince/mappers/serial_mapper.h>

using boost::optional;
using std::set;
using std::string;
using std::unique_ptr;
using std::vector;


namespace quince {

table_base::~table_base()
{}

void
table_base::open() {
    if (_is_open)  throw table_open_exception();
    if (_key_mapper == nullptr)  throw no_primary_key_exception();

    {
        _database.make_enclosure_available(_binomen._enclosure);
        transaction txn(_database);
        const unique_ptr<sql> cmd = _database.make_sql();
        cmd->write_create_table(
            _binomen,
            get_value_mapper_base(),
            *_key_mapper,
            readback_id(),
            _foreign_specs
        );
        if (_database.get_session()->unchecked_exec(*cmd)) {  // failure is benign, but in that case we needn't commit.
            for (size_t i = 0; i < _index_specs.size(); i++) {
                const index_spec &is = _index_specs[i];
                const unique_ptr<sql> cmd = _database.make_sql();
                cmd->write_create_index(_binomen, i, is._mappers, is._is_unique);
                _database.get_session()->exec(*cmd);
            }
            txn.commit();
        }
    }
    check_metadata();
    _is_open = true;
}

void
table_base::close() {
    if (!_is_open)  throw table_closed_exception();
    _is_open = false;
}

void
table_base::check_metadata(bool db_may_have_extra_columns) const {
    const set<string> actual = to_set(_database.retrieve_column_titles(_binomen));
    set<string> expected;
    get_value_mapper_base().for_each_persistent_column([&](const persistent_column_mapper &p) {
        const column_type type = _database.retrievable_column_type(
            p.get_column_type(p.id() == readback_id())
        );
        expected.insert(
            "\"" + p.name() + "\" " + _database.column_type_name(type)
        );
    });
    if (   ! is_subset(expected, actual)  // deficit
        || (! db_may_have_extra_columns && ! is_subset(actual, expected))  // surplus
       )
        throw table_mismatch_exception(_binomen, expected, actual);
}

void
table_base::specify_key_base(const abstract_mapper_base *key_mapper) {
    if (_is_open)  throw table_open_exception();

    _key_mapper = key_mapper;
}

unique_ptr<sql>
table_base::sql_insert(const row &input) {
    unique_ptr<sql> result = _database.make_sql();
    result->write_insert(_binomen, _value_mapper, readback_id());
    result->write_values(_value_mapper, input, readback_id());
    return result;
}


template<typename Src>
void
table_base::update_with_no_output(
    const abstract_mapper_base &dest,
    const Src &src,
    const abstract_predicate &pred
) const {
    _database.get_session()->exec(
        *sql_update(dest, src, pred)
    );
}
template
void
table_base::update_with_no_output<abstract_mapper_base>(
    const abstract_mapper_base &,
    const abstract_mapper_base &,
    const abstract_predicate &
) const;
template
void
table_base::update_with_no_output<row>(
    const abstract_mapper_base &,
    const row &,
    const abstract_predicate &
) const;


template<typename Src>
result_stream
table_base::update_with_stream_output(
    const abstract_mapper_base &dest,
    const Src &src,
    const abstract_predicate &pred,
    const abstract_mapper_base &returning,
    uint32_t fetch_size
) const {
    return _database.get_session()->exec_with_stream_output(
        *sql_update(dest, src, pred, &returning),
        fetch_size
    );
}
template
result_stream
table_base::update_with_stream_output<abstract_mapper_base>(
    const abstract_mapper_base &,
    const abstract_mapper_base &,
    const abstract_predicate &,
    const abstract_mapper_base &,
    uint32_t
) const;
template
result_stream
table_base::update_with_stream_output<row>(
    const abstract_mapper_base &,
    const row &,
    const abstract_predicate &,
    const abstract_mapper_base &,
    uint32_t
) const;


template<typename Src>
unique_ptr<row>
table_base::update_with_one_output(
    const abstract_mapper_base &dest,
    const Src &src,
    const abstract_predicate &pred,
    const abstract_mapper_base &returning
) const {
    return _database.get_session()->exec_with_one_output(
        *sql_update(dest, src, pred, &returning)
    );
}
template
unique_ptr<row>
table_base::update_with_one_output<abstract_mapper_base>(
    const abstract_mapper_base &,
    const abstract_mapper_base &,
    const abstract_predicate &,
    const abstract_mapper_base &
) const;
template
unique_ptr<row>
table_base::update_with_one_output<row>(
    const abstract_mapper_base &,
    const row &,
    const abstract_predicate &,
    const abstract_mapper_base &
) const;


void
table_base::write_table_reference(sql &cmd) const {
    if (!_is_open)  throw table_closed_exception();

    cmd.write_quoted(_binomen);
}

column_id_set 
table_base::aliased_columns() const {
    return {};
}

void
table_base::for_each_column(std::function<void(const column_mapper &)> op) const {
    _value_mapper.for_each_column(op);
}

void
table_base::remove_where(const abstract_predicate &pred) const {
    if (a_priori_false(pred))  return;

    const unique_ptr<sql> cmd = clone(_sql_delete);
    if (! a_priori_true(pred))
        cmd->write_where(pred);
    _database.get_session()->exec(*cmd);
}

void
table_base::remove_existing_where(const abstract_predicate &pred) const {
    if (! remove_if_exists_where(pred))
        throw no_row_exception();
}

bool
table_base::remove_if_exists_where(const abstract_predicate &pred) const {
    if (empty_impl(pred))  return false;

    remove_where(pred);
    return true;
}

optional<column_id>
table_base::readback_id() const {
    if (const serial_mapper * const readback = readback_mapper())
        return readback->id();
    else
        return boost::none;
}

template<typename Src>
unique_ptr<sql>
table_base::sql_update(
    const abstract_mapper_base &dest,
    const Src &src,
    const abstract_predicate &pred,
    const abstract_mapper_base *returning
) const {
    if (! is_subset(dest.columns(), _value_mapper.columns()))
        throw outside_table_exception(_binomen);

    unique_ptr<sql> cmd = _database.make_sql();
    cmd->write_update(_binomen, dest, src, readback_id());
    if (! a_priori_true(pred))  cmd->write_where(pred);
    if (returning != nullptr)  cmd->write_returning(*returning);
    return cmd;
}

template
unique_ptr<sql>
table_base::sql_update<abstract_mapper_base>(
    const abstract_mapper_base &,
    const abstract_mapper_base &,
    const abstract_predicate &,
    const abstract_mapper_base *
) const;
template
unique_ptr<sql>
table_base::sql_update<row>(
    const abstract_mapper_base &,
    const row &,
    const abstract_predicate &,
    const abstract_mapper_base *
) const ;

void
table_base::specify_index_impl(bool is_unique, vector<unique_ptr<const abstract_mapper_base>> &&mappers) {
    if (_is_open)  throw table_open_exception();

    const index_spec spec(
        transform(
            std::move(mappers),
            [this](unique_ptr<const abstract_mapper_base> &m)  { return &own(m); }
        ),
        is_unique
    );
    if (! get_database().supports_index(spec))  throw unsupported_exception();
    _index_specs.push_back(spec);
}

void
table_base::specify_foreign_impl(
    unique_ptr<const abstract_mapper_base> foreign,
    const binomen &target_table,
    unique_ptr<const abstract_mapper_base> target_mapper
) {
    if (_is_open)  throw table_open_exception();

    _foreign_specs.emplace_back(
        &own(foreign),
        target_table,
        &own(target_mapper)
    );
}

void
table_base::rename_from(const string &old_local_name) const {
    if (_is_open)  throw table_open_exception();

    binomen old_binomen = _binomen;
    old_binomen._local = old_local_name;

    const unique_ptr<sql> cmd = _database.make_sql();
    cmd->write_rename_table(old_binomen, _binomen._local);
    _database.get_session()->exec(*cmd);
}

template<typename Src>
void
table_base::add_field_impl(const abstract_mapper_base &dest, const Src &src) const {
    if (_is_open)
        throw table_open_exception();
    if (! is_subset(dest.column_ids(), _value_mapper.column_ids()))
        throw outside_table_exception(_binomen);

    const optional<column_id> readback = readback_id();

    transaction txn(_database);
    unique_ptr<sql> cmd;

    cmd = _database.make_sql();
    cmd->write_add_columns(_binomen, dest, readback);
    _database.get_session()->exec(*cmd);

    if (! readback || dest.column_ids() != column_id_set{*readback})
        update_with_no_output(dest, src, predicate(true));

    cmd = _database.make_sql();
    cmd->write_constrain_as_not_null_where_appropriate(_binomen, dest);
    _database.get_session()->exec(*cmd);

    txn.commit();
}
template
void
table_base::add_field_impl<abstract_mapper_base>(
    const abstract_mapper_base &,
    const abstract_mapper_base &
) const;
template
void
table_base::add_field_impl<row>(
    const abstract_mapper_base &,
    const row &
) const;


void
table_base::drop_field(const abstract_mapper_base &deletion) const {
    if (is_open())
        throw table_open_exception();

    if (! is_subset(deletion.column_ids(), _value_mapper.column_ids()))
        throw outside_table_exception(_binomen);

    const unique_ptr<sql> cmd = _database.make_sql();
    cmd->write_drop_columns(_binomen, deletion);
    _database.get_session()->exec(*cmd);
}

void
table_base::rename_field_impl(const abstract_mapper_base &before, const abstract_mapper_base &after) const {
    if (_is_open)
        throw table_open_exception();

    const column_id_set my_columns = _value_mapper.column_ids();
    if (! (
            is_subset(before.column_ids(), my_columns)
         || is_subset(after.column_ids(), my_columns)
    ))
        throw outside_table_exception(_binomen);

    vector<string> names_before;
    vector<string> names_after;
    before.for_each_persistent_column([&](const persistent_column_mapper &p)    { names_before.push_back(p.name()); } );
    after.for_each_persistent_column([&](const persistent_column_mapper &p)     { names_after.push_back(p.name()); } );

    const size_t n = names_before.size();
    assert(names_after.size() == n);

    transaction txn(_database);
    unique_ptr<sql> cmd;
    for (size_t i = 0; i < n; i++) {
        cmd = _database.make_sql();
        cmd->write_rename_column(_binomen, names_before[i], names_after[i]);
        _database.get_session()->exec(*cmd);
    }
    txn.commit();
}

void
table_base::set_field_type(const abstract_mapper_base &mapper) const {
    if (is_open())
        throw table_open_exception();

    if (! is_subset(mapper.column_ids(), _value_mapper.column_ids()))
        throw outside_table_exception(_binomen);

    transaction txn(_database);

    unique_ptr<sql> cmd;

    // In case some fields went from non-optional to optional, and also to avoid duplication
    // of abiding non-null constraints, which will be re-added by constrain_as_not_null() below:
    //
    cmd = _database.make_sql();
    cmd->write_unconstrain_as_not_null(_binomen, mapper);
    _database.get_session()->exec(*cmd);

    cmd = _database.make_sql();
    cmd->write_set_columns_types(_binomen, mapper, readback_id());
    _database.get_session()->exec(*cmd);

    // In case some fields went from optional to non-optional:
    //
    cmd = _database.make_sql();
    cmd->write_constrain_as_not_null_where_appropriate(_binomen, mapper);
    _database.get_session()->exec(*cmd);

    txn.commit();
}

void
table_base::drop() const {
    if (is_open())  throw table_open_exception();

    const unique_ptr<sql> cmd = _database.make_sql();
    cmd->write_drop_table(_binomen);
    _database.get_session()->exec(*cmd);
}

void
table_base::drop_if_exists() const {
    if (is_open())  throw table_open_exception();

    const unique_ptr<sql> cmd = _database.make_sql();
    cmd->write_drop_table_if_exists(_binomen);
    _database.get_session()->exec(*cmd);
}

}
