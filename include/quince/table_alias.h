#ifndef QUINCE__table_alias_h
#define QUINCE__table_alias_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/compiler_specific.h>
#include <quince/detail/mapper_factory.h>
#include <quince/table.h>


namespace quince {

template<typename> class abstract_query;

// Base class of all the table_alias<T>s.
//
class table_alias_base : protected object_owner, public table_interface {

    // Everything in this class is for quince internal use only.

public:
    virtual ~table_alias_base();

    virtual const object_id &query_id() const override          { return _query_id; }
    virtual const std::string &name() const override            { return _name; }
    virtual const std::string &basename() const override        { return _name; }
    virtual const database &get_database() const override       { return _table.get_database(); }

    virtual void write_table_reference(sql &) const override;
    virtual column_id_set aliased_columns() const override;
    virtual void for_each_column(std::function<void(const column_mapper &)>) const override;

protected:
    template<typename Value>
    table_alias_base(
        std::unique_ptr<const table_base> table,
        const Value *
    ) :
        _name(generate_name()),
        _table(own(table)),
        _value_mapper(own(make_value_mapper<Value>()))
    {}

    const table_base &get_table_base() const                    { return _table; }
    const abstract_mapper_base &get_value_mapper_base() const   { return _value_mapper; }

    virtual bool might_have_duplicate_rows() const override     { return false; }

private:
    template<typename T>
    std::unique_ptr<abstract_mapper_base>
    make_value_mapper() const {
        std::unique_ptr<abstract_mapper_base> result = _table.get_mapper_factory().create<T>(boost::none);
        initialize_mapper(*result);
        return result;
    }

    static std::string generate_name();
    object_id _query_id;
    const std::string _name;
    const table_base &_table;
    const abstract_mapper_base &_value_mapper;
};


QUINCE_SUPPRESS_MSVC_DOMINANCE_WARNING

// See http://quince-lib.com/queries/join/self_joins.html#queries.join.self_joins.and_the_solution_table_alias
//
// Most of the public features of table_alias are inherited from abstract_query.
//
template<typename Value>
class table_alias : public table_alias_base, public abstract_query<Value> {
public:
    table_alias<Value>
    alias() const {
        return table_alias(get_table_base());
    }


    // --- Everything from here to end of class is for quince internal use only. ---

    typedef typename abstract_query<Value>::value_mapper value_mapper;

    virtual const value_mapper &
    get_value_mapper() const override {
        return dynamic_cast<const value_mapper &>(get_value_mapper_base());
    }

    virtual std::unique_ptr<cloneable>
    clone_impl() const override {
        return quince::make_unique<table_alias<Value>>(*this);
    }

private:
    friend class general_table<Value>;

    explicit
    table_alias(const table_base &table) :
        table_alias_base(clone(table), (Value*)nullptr)
    {}
};

QUINCE_UNSUPPRESS_MSVC_WARNING

}

#endif
