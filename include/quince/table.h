#ifndef QUINCE__table_h
#define QUINCE__table_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <tuple>
#include <quince/detail/binomen.h>
#include <quince/detail/abstract_query.h>
#include <quince/detail/compiler_specific.h>
#include <quince/detail/table_base.h>
#include <quince/mapping_customization.h>


namespace quince {

class sql;
class database;
template<typename> class table_alias;
template<typename...> class tuple_mapper;
template<typename> class query;


QUINCE_SUPPRESS_MSVC_DOMINANCE_WARNING


// general_table<Value> is the common base of table<Value> and serial_table<Value>.
//
template<typename Value>
class general_table : public table_base, public abstract_query<Value> {
public:
    typedef typename abstract_query<Value>::value_mapper value_mapper;

    table_alias<Value>
    alias() const {
        return table_alias<Value>(*this);
    }


    // --- Everything from here to end of class is for quince internal use only. ---

    virtual const value_mapper &get_value_mapper() const override  { return _value_mapper; }

protected:
    general_table(
        const database &db,
        const std::string &name,
        const boost::optional<mapping_customization> &mc = boost::none
    ) :
        table_base(db, name, clone_or_null(mc), (Value *)nullptr),
        _value_mapper(dynamic_cast<const value_mapper &>(get_value_mapper_base()))
    {}

    template<typename PtrToKeyMember>
    general_table(
        const database &db,
        const std::string &name,
        PtrToKeyMember ptkm,
        const boost::optional<mapping_customization> &mc = boost::none
    ) :
        general_table<Value>(db, name, mc)
    {
        specify_key_from_ptkm(ptkm);
    }


    template<typename PtrToKeyMember>
    void
    specify_key_from_ptkm(PtrToKeyMember ptkm) {
        exposed_mapper_type<member_from_ptr_to_member<PtrToKeyMember>>::static_forbid_optionals();
        specify_key_base(&_value_mapper.lookup(ptkm));
    }

    template<typename T>
    void
    specify_key(const abstract_mapper<T> &key_mapper) {
        specify_key_impl(clone(key_mapper));
    }

    template<typename... T>
    void
    specify_key(const abstract_mapper<T> &... key_mapper_parts) {
        specify_key_impl(std::make_unique<tuple_mapper<T...>>(key_mapper_parts...));
    }

private:
    template<typename KeyMapper>
    void
    specify_key_impl(std::unique_ptr<KeyMapper> key_mapper) {
        exposed_mapper_type<typename KeyMapper::value_type>::static_forbid_optionals();
        specify_key_base(&own(key_mapper));
    }

    virtual bool
    empty_impl(const abstract_predicate &pred) const override {
        return this->where(pred).empty();
    }

    const value_mapper &_value_mapper;
};


// See http://quince-lib.com/tables.html
//
// Most of the public features are inherited from base classes.
// E.g. update() is in abstract_query, add_field() is in table_base.
//
template<typename Value>
class table : public general_table<Value> {
public:
    table(
        const database &db,
        const std::string &name,
        const boost::optional<mapping_customization> &mc = boost::none
    ) :
        general_table<Value>(db, name, mc)
    {}

    template<typename PtrToKeyMember>
    table(
        const database &db,
        const std::string &name,
        PtrToKeyMember ptkm,
        const boost::optional<mapping_customization > &mc = boost::none
    ) :
        general_table<Value>(db, name, ptkm, mc)
    {}

    void
    insert(const Value &value) {
        row input(& this->get_database());
        this->get_value_mapper().to_row(value, input);
        this->get_database().get_session()->exec(* this->sql_insert(input));
    }

    using general_table<Value>::specify_key;
    using general_table<Value>::specify_key_from_ptkm;


    // --- Everything from here to end of class is for quince internal use only. ---

    virtual std::unique_ptr<cloneable>
    clone_impl() const override {
        return std::make_unique<table<Value>>(*this);
    }

private:
    table(const database &, const std::string &, const mapping_customization &&) = delete;
    template<typename Ptkm> table(const database &, const std::string &, Ptkm, const mapping_customization &&) = delete;
};


// See http://quince-lib.com/tables.html
//
// Most of the public features are inherited from base classes.
// E.g. update() is in abstract_query, add_field() is in table_base.
//
template<typename Value>
class serial_table : public general_table<Value> {
public:
    typedef serial Value::*ptr_to_key_member;

    serial_table(
        const database &db,
        const std::string &name,
        const boost::optional<mapping_customization> &mc = boost::none
    ) :
        general_table<Value>(db, name, mc)
    {}

    template<typename ValueBase>
    serial_table(
        const database &db,
        const std::string &name,
        serial ValueBase::*ptr_to_key_member,
        const boost::optional<mapping_customization> &mc = boost::none
    ) :
        general_table<Value>(db, name, ptr_to_key_member, mc)
    {
        _ptr_to_key_member = ptr_to_key_member;
    }

    serial
    insert(const Value &value) {
        row input(&this->get_database());
        this->get_value_mapper().to_row(value, input);
        return this->get_database().insert_with_readback(this->sql_insert(input), * readback_mapper());
    }

    serial
    insert(Value &value) {
        const Value &const_ref = value;
        return value.*_ptr_to_key_member = insert(const_ref);
    }

    template<typename ValueBase>
    void
    specify_key_from_ptkm(serial ValueBase::*ptr_to_key_member) {
        _ptr_to_key_member = ptr_to_key_member;
        general_table<Value>::specify_key_from_ptkm(ptr_to_key_member);
    }


    // --- Everything from here to end of class is for quince internal use only. ---

    const serial_mapper *
    readback_mapper() const {
        return dynamic_cast<const serial_mapper *>(&this->get_key_mapper_base());
    }

    virtual std::unique_ptr<cloneable>
    clone_impl() const override {
        return std::make_unique<serial_table<Value>>(*this);
    }

private:
    virtual void
    specify_key_base(const abstract_mapper_base *key_mapper) override {
        table_base::specify_key_base(key_mapper);
    }

    serial Value::*_ptr_to_key_member = nullptr;
};

QUINCE_UNSUPPRESS_MSVC_WARNING

}

#endif
