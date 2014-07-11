#ifndef QUINCE__mappers__detail__junction_h
#define QUINCE__mappers__detail__junction_h

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

#include <tuple>
#include <quince/detail/abstract_query_base.h>
#include <quince/detail/compiler_specific.h>
#include <quince/detail/object_id.h>
#include <quince/exprn_mappers/detail/exprn_mapper.h>
#include <quince/mappers/optional_mapper.h>
#include <quince/exceptions.h>


/*
    This file defines a hierarchy of "junction" classes, which are what all the functions
    in the join() family return.

    As far as I know, there is never a need for application code to mention these classes,
    since the objects that join() etc. return are always subclasses of some abstract_query<T>
    and implicitly convertible to query<T>.  See the advice at
    http://quince-lib.com/queries.html#queries.queries_in_general

    On the other hand, this file also defines the free function join(), which is for public
    use.  Scroll down to the bottom.
*/

namespace quince {

template<typename> class abstract_query;


class junction_base : public virtual abstract_query_base, protected object_owner {

    // Everything in this class is for quince internal use only.

public:
    virtual void write_table_reference(sql &) const override;
    virtual column_id_set aliased_columns() const override;
    virtual void for_each_column(std::function<void(const column_mapper &)> op) const override;

    virtual const object_id &query_id() const override      { return _query_id; }
    virtual const database &get_database() const override   { return _database; }

    virtual bool a_priori_empty() const override;
    virtual bool might_have_duplicate_rows() const override;

protected:
    template<typename T, typename... Ts>
    junction_base(T first, Ts... others) :
        _database(first->get_database()),
        _joinees({&adopt(std::move(first)), &adopt(std::move(others))...})
    {}

    const abstract_query_base &lhs() const;
    const abstract_query_base &rhs() const;

private:
    const abstract_query_base &adopt(std::unique_ptr<abstract_query_base> query);

    object_id _query_id;
    const database &_database;
    std::vector<const abstract_query_base*> _joinees;
};


class conditional_junction_base : public junction_base {

    // Everything in this class is for quince internal use only.

public:
    virtual void write_table_reference(sql &) const override;
    virtual bool a_priori_empty() const override;

protected:
    conditional_junction_base(
        std::unique_ptr<abstract_query_base> lhs,
        std::unique_ptr<abstract_query_base> rhs,
        const abstract_predicate &pred,
        conditional_junction_type type
    ) :
        junction_base(std::move(lhs), std::move(rhs)),
        _predicate(pred),
        _type(type)
    {}

    template<typename T>
    static const exposed_mapper_type<T> &
    from_non_optional(
        const exposed_mapper_type<T> &m,
        T*  //dummy to indicate destination type -- might even be an optional (rare case)
    ) {
        return m;
    }

    template<typename T>
    static const optional_mapper<T>
    from_non_optional(
        const exposed_mapper_type<T> &m,
        boost::optional<T> *    // dummy to indicate destination type
    ) {
        if (m.can_be_all_null())  throw ambiguous_nulls_exception();
        return optional_mapper<T>(m);
    }

private:
    const predicate _predicate;
    conditional_junction_type _type;
};


QUINCE_SUPPRESS_MSVC_DOMINANCE_WARNING

template<typename Collector>
class junction :
    public junction_base,
    public abstract_query<Collector>
{

    // Everything in this class is for quince internal use only.

public:
    typedef static_mapper_type<Collector> value_mapper_type;

    template<typename T, typename... Ts>
    explicit junction(const abstract_query<T> &first, const abstract_query<Ts> &... others) :
        junction_base(clone(first), clone(others)...),
        _value_mapper{first.get_value_mapper(), others.get_value_mapper()...}
    {}

    virtual const value_mapper_type &get_value_mapper() const override  { return _value_mapper; }

    virtual std::unique_ptr<cloneable>
    clone_impl() const override {
        return std::make_unique<junction<Collector>>(*this);
    }

private:
    const value_mapper_type _value_mapper;
};


template<typename Collector>
class conditional_junction :
    public conditional_junction_base,
    public abstract_query<Collector>
{

    // Everything in this class is for quince internal use only.

public:
    typedef static_mapper_type<Collector> value_mapper_type;

    template<typename NonoptionalL, typename NonoptionalR, typename L, typename R>
    conditional_junction(
        const abstract_query<NonoptionalL> &lhs,
        const abstract_query<NonoptionalR> &rhs,
        const abstract_predicate &pred,
        conditional_junction_type type,
        L *,    // dummy to indicate left component of destination type
        R *,    // dummy to indicate right component of destination type
        bool swap_lr = false
    ) :
        conditional_junction_base(
            swap_lr ? clone<abstract_query_base>(rhs) : clone<abstract_query_base>(lhs),
            swap_lr ? clone<abstract_query_base>(lhs) : clone<abstract_query_base>(rhs),
            pred,
            type
        ),
        _value_mapper{
            from_non_optional(lhs.get_value_mapper(), (L*)nullptr),
            from_non_optional(rhs.get_value_mapper(), (R*)nullptr)
        }
    {}

    virtual const value_mapper_type &get_value_mapper() const override  { return _value_mapper; }

    virtual std::unique_ptr<cloneable>
    clone_impl() const override {
        return std::make_unique<conditional_junction<Collector>>(*this);
    }

private:
    const value_mapper_type _value_mapper;
};

QUINCE_UNSUPPRESS_MSVC_WARNING


// The join() free function, with and without a collector class.  See:
// http://quince-lib.com/queries/join.html#queries.join.join.as_a_free_function and
// http://quince-lib.com/queries/join/collector.html

template<typename Collector, typename... Ts>
junction<Collector>
join(const abstract_query<Ts> &... joinees) {
    return junction<Collector>(joinees...);
}

template<typename... Ts>
junction<std::tuple<Ts...>>
join(const abstract_query<Ts> &... joinees) {
    return join<std::tuple<Ts...>>(joinees...);
}

}

#endif
