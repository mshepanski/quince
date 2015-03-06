#ifndef QUINCE__mappers__detail__table_h
#define QUINCE__mappers__detail__table_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <vector>
#include <quince/exprn_mappers/detail/exprn_mapper.h>
#include <quince/detail/binomen.h>
#include <quince/detail/object_id.h>
#include <quince/detail/object_owner.h>
#include <quince/detail/index_spec.h>
#include <quince/detail/mapper_factory.h>
#include <quince/detail/table_interface.h>
#include <quince/detail/session.h>
#include <quince/database.h>
#include <quince/mapping_customization.h>


namespace quince {

template<typename> class abstract_mapper;
typedef abstract_mapper<bool> abstract_predicate;
class serial_mapper;


// For quince internal use only.
//
struct foreign_spec {
    const abstract_mapper_base *_foreign;
    const binomen _target_table;
    const abstract_mapper_base *_target_mapper;

    foreign_spec(const abstract_mapper_base *f, const binomen &tt, const abstract_mapper_base *tm) :
        _foreign(f),
        _target_table(tt),
        _target_mapper(tm)
    {}
};


// Base class of all tables and serial_tables (but not table_aliases).
//
class table_base : protected object_owner, public table_interface {
public:
    virtual ~table_base();

    // All of these public members are described at http://quince-lib.com/tables.html

    void open();
    void close();

    template<typename... T>
    void
    specify_index(const abstract_mapper<T> &... mappers) {
        specify_index_impl(
            false,
            make_unique_ptr_vector<const abstract_mapper_base>(clone(mappers)...)
        );
    }

    template<typename... T>
    void
    specify_unique(const abstract_mapper<T> &... mappers) {
        specify_index_impl(
            true,
            make_unique_ptr_vector<const abstract_mapper_base>(clone(mappers)...))
        ;
    }

    template<typename T>
    void
    specify_foreign(const abstract_mapper<T> &foreign, const table_base &target_table, const abstract_mapper<T> &target_mapper) {
        specify_foreign_impl(
            clone(foreign),
            target_table._binomen,
            clone(target_mapper)
        );
    }

    template<typename T>
    void
    specify_foreign(const abstract_mapper<T> &foreign, const table_base &target_table) {
        specify_foreign(
            foreign,
            target_table,
            dynamic_cast<const abstract_mapper<T>&>(target_table.get_key_mapper_base())
        );
    }

    template<typename T>
    void
    add_field(const abstract_mapper<T> &dest, const abstract_mapper<T> &src) const {
        const abstract_mapper_base &src_as_amb = src;
        add_field_impl(dest, src_as_amb);
    }

    template<typename T>
    void
    add_field(const abstract_mapper<T> &dest, const T &src) const {
        row src_as_row(& get_database());
        dest.to_row(src, src_as_row);
        add_field_impl(dest, src_as_row);
    }

    template<typename T>
    void
    add_field(const abstract_mapper<T> &mapper) const {
        add_field(mapper, T());
    }

    void drop_field(const abstract_mapper_base &) const;

    template<typename T>
    void rename_field(const abstract_mapper<T> &before, const abstract_mapper<T> &after) const {
        rename_field_impl(before, after);
    }

    void set_field_type(const abstract_mapper_base &) const;

    void drop() const;
    void drop_if_exists() const;


    // --- Everything from here to end of class is for quince internal use only. ---

    void check_metadata(bool for_readonly_access = false) const;

    virtual const object_id &query_id() const override      { return _query_id; }
    virtual const std::string &name() const                 { return _name; }
    virtual const std::string &basename() const             { return _binomen._local; }
    virtual const database &get_database() const override   { return _database; }

    bool is_open() const                                    { return _is_open; }
    const mapper_factory &get_mapper_factory() const        { return _mapper_factory; }
    const uint64_t table_id() const                         { return _table_id; }

    virtual void write_table_reference(sql &) const override;
    virtual column_id_set aliased_columns() const override;
    virtual void for_each_column(std::function<void(const column_mapper &)> op) const override;

    void rename_from(const std::string &old_local_name) const;

protected:
    template<typename Value>
    table_base(
        const database &db,
        const std::string &name,
        std::unique_ptr<const mapping_customization> mc,
        const Value * // dummy for type inference
    ) :
        _table_id(_query_id.get()),
        _database(db),
        _name(name),
        _binomen(binomen::split(name, db.get_default_enclosure())),
        _mapper_factory(own_or_null(mc), db.get_mapper_factory()),
        _value_mapper(own(make_value_mapper<Value>())),
        _sql_delete(own(_database.make_sql())),
        _is_open(false)
    {
        const_cast<sql&>(_sql_delete).write_delete_from(_binomen);
    }

    const abstract_mapper_base &get_value_mapper_base() const   { return _value_mapper; }
    const abstract_mapper_base &get_key_mapper_base() const     { return *_key_mapper; }

    virtual void specify_key_base(const abstract_mapper_base *);

    std::unique_ptr<sql> sql_insert(const row &input);

    virtual bool might_have_duplicate_rows() const override     { return false; }

private:
    friend class query_base;

    template<typename T>
    std::unique_ptr<abstract_mapper_base>
    make_value_mapper() const {
        std::unique_ptr<abstract_mapper_base> result = _mapper_factory.create<T>(boost::none);
        initialize_mapper(*result);
        result->_table_whose_value_mapper_i_am = this;
        return result;
    }

    template<typename Src> // abstract_mapper_base or row
    void
    update_with_no_output(
        const abstract_mapper_base &dest,
        const Src &src,
        const abstract_predicate &
    ) const;

    template<typename Src> // abstract_mapper_base or row
    result_stream
    update_with_stream_output(
        const abstract_mapper_base &dest,
        const Src &src,
        const abstract_predicate &,
        const abstract_mapper_base &returning,
        uint32_t fetch_size
    ) const;

    template<typename Src> // abstract_mapper_base or row
    std::unique_ptr<row>
    update_with_one_output(
        const abstract_mapper_base &dest,
        const Src &src,
        const abstract_predicate &,
        const abstract_mapper_base &returning
    ) const;

    void remove_where(const abstract_predicate &) const;
    void remove_existing_where(const abstract_predicate &) const;
    bool remove_if_exists_where(const abstract_predicate &) const;

    boost::optional<column_id> readback_id() const;

    template<typename Src> // abstract_mapper_base or row
    std::unique_ptr<sql>
    sql_update(
        const abstract_mapper_base &dest,
        const Src &src,
        const abstract_predicate &,
        const abstract_mapper_base *returning = nullptr
    ) const;

    virtual bool empty_impl(const abstract_predicate &) const = 0;

    virtual const serial_mapper * readback_mapper() const   { return nullptr; }

    void specify_index_impl(
        bool unique,
        std::vector<std::unique_ptr<const abstract_mapper_base>> &&
    );
    void specify_foreign_impl(
        std::unique_ptr<const abstract_mapper_base> foreign,
        const binomen &target_table,
        std::unique_ptr<const abstract_mapper_base> target_mapper
    );

    template<typename Src> // abstract_mapper_base or row
    void add_field_impl(const abstract_mapper_base &dest, const Src &src) const;

    void rename_field_impl(const abstract_mapper_base &before, const abstract_mapper_base &after) const;

    void constrain_as_not_null_where_appropriate(const abstract_mapper_base &) const;

    object_id _query_id;        // copies of *this have a different _query_id
    const uint64_t _table_id;   // copies of *this have the same _table_id
    const database &_database;
    const std::string _name;
    const binomen _binomen;
    const mapper_factory _mapper_factory;
    const abstract_mapper_base &_value_mapper;
    const abstract_mapper_base *_key_mapper = nullptr;
    std::vector<index_spec> _index_specs;
    std::vector<foreign_spec> _foreign_specs;

    const sql &_sql_delete;

    bool _is_open;
};

}

#endif
