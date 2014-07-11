#ifndef QUINCE__mappers__detail__abstract_query_h
#define QUINCE__mappers__detail__abstract_query_h

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

#include <quince/detail/abstract_query_base.h>
#include <quince/detail/compiler_specific.h>
#include <quince/mappers/detail/exposed_mapper_type.h>
#include <quince/exceptions.h>


namespace quince {

template<typename> class abstract_mapper;
typedef abstract_mapper<bool> abstract_predicate;
class grouping;
template<typename> class query;
template<typename> class query_iterator;
template<typename> class junction;
template<typename> class conditional_junction;

enum class conditional_junction_type { inner, left, right, full };


// abstract_query is the interface that is implemented by everything that is a query in the
// broad sense, i.e. a query, table, serial_table, table_alias, junction, or conditional_junction.
//
// See the discussion at http://quince-lib.com/queries.html.
//
// The doco says "I use the noun “query”, in normal font, for objects of various C++ types"
// (i.e. the types just listed).  Here in the code I don't have a normal font, so I use the
// word "query" for class query only.  When I want to speak of queries in normal-font sense,
// I will say "abstract_query".
//
template<typename Value>
class abstract_query : public virtual abstract_query_base {
protected:
    // wrapped() creates a query in which *this is the FROM clause, and the rest of the query
    // simply passes on its output; i.e. the returned query has the same output as *this.
    //
    // This has two distinct purposes:
    //
    //  1.  To promote a non-query to a query.  (That's what the calls to wrapped() from this
    //      class do.)
    //
    //  2.  To add an extra layer to a query, in preparation for adding clauses that could not
    //      validly be added otherwise.  E.g. if you have foo.limit(5) and you want to apply
    //      skip(3), then foo.limit(5) needs to be wrapped first.  That's what the calls to
    //      wrapped in the query class do.
    //
    query<Value>
    wrapped() const {
        return query<Value>(*this);
    }

    // Same as wrapped() but returns the result by unique_ptr rather than by value.
    //
    std::unique_ptr<query<Value>>
    unique_wrapped() const {
        return std::make_unique<query<Value>>(*this);
    }

public:
    // All of these public members are described at
    // http://quince-lib.com/queries.html and http://quince-lib.com/tables/manipulation.html

    typedef exposed_mapper_type<Value> value_mapper;

    virtual ~abstract_query()  {}

    typedef query_iterator<Value> iterator;
    typedef iterator const_iterator;

    virtual const value_mapper &get_value_mapper() const = 0;
    const value_mapper * operator->() const  { return &get_value_mapper(); }
    const value_mapper & operator*() const   { return get_value_mapper(); }

    // The following virtuals all have overrides in class query, and those overrides do the real work.
    // All *these* methods do is handle the case where *this is a non-query (e.g. a table), by
    // converting it to a query and delegating.
    //
    virtual uint64_t size() const                                       { return wrapped().size(); }
    virtual bool empty() const                                          { return wrapped().empty(); }
    virtual query<Value> limit(uint32_t n) const                        { return wrapped().limit(n); }
    virtual query<Value> skip(uint32_t n) const                         { return wrapped().skip(n); }
    virtual query<Value> fetch_size(uint32_t n) const                   { return wrapped().fetch_size(n); }
    virtual query<Value> where(const abstract_predicate &p) const       { return wrapped().where(p); }
    virtual query<Value> where(bool b) const                            { return wrapped().where(b); }
    virtual iterator begin() const                                      { return wrapped().begin(); }
    virtual iterator end() const                                        { return wrapped().end(); }
    virtual boost::optional<Value> get() const                          { return wrapped().get(); }
    virtual query<Value> distinct() const                               { return wrapped().distinct(); }
    virtual query<Value> union_(const query<Value> &rhs) const          { return wrapped().union_(rhs); }
    virtual query<Value> union_all(const query<Value> &rhs) const       { return wrapped().union_all(rhs); }
    virtual query<Value> intersect(const query<Value> &rhs) const       { return wrapped().intersect(rhs); }
    virtual query<Value> intersect_all(const query<Value> &rhs) const   { return wrapped().intersect_all(rhs); }
    virtual query<Value> except(const query<Value> &rhs) const          { return wrapped().except(rhs); }
    virtual query<Value> except_all(const query<Value> &rhs) const      { return wrapped().except_all(rhs); }
    virtual void remove()                                               { wrapped().remove(); }
    virtual void remove_existing()                                      { wrapped().remove_existing(); }
    virtual bool remove_if_exists()                                     { return wrapped().remove_if_exists(); }
    virtual std::string to_string() const                               { return wrapped().to_string(); }

    // distinct_on(), group(), order(), update() and select() would be virtuals, operating just
    // like the virtuals directly above, except for the fact that templated functions can't be virtual.
    // So I achieve the same result with code: I check whether *this is a query, and if it is
    // I delegate immediately to the corresponding method in the query class; otherwise I promote
    // the non-query to a query and delegate.
    //
    template<typename... Args>
    query<Value>
    distinct_on(Args &&... args) const {
        if (auto q = dynamic_cast<const query<Value> *>(this))
            return q->distinct_on(std::forward<Args>(args)...);
        return wrapped().distinct_on(std::forward<Args>(args)...);
    }

    // Defined in grouping.h to avoid a circular header dependency.
    //
    template<typename... Args>
    grouping
    group(Args && ...) const;

    template<typename... Args>
    query<Value>
    order(Args &&... args) const {
        if (auto q = dynamic_cast<const query<Value> *>(this))
            return q->order(std::forward<Args>(args)...);
        return wrapped().order(std::forward<Args>(args)...);
    }

    // TODO generalize this to delegate to versions of update that return a value
    template<typename... Args>
    void
    update(Args &&... args) {
        if (auto q = dynamic_cast<query<Value> *>(this))
            q->update(std::forward<Args>(args)...);
        wrapped().update(std::forward<Args>(args)...);
    }

    template<typename Mapper>
    query<typename Mapper::value_type>
    select(const Mapper &mapper) const {
        if (auto q = dynamic_cast<const query<Value> *>(this))
            return q->select(mapper);
        return wrapped().select(mapper);
    }

    template<typename Collector, typename... Mappers>
    query<Collector>
    select(const Mappers &... ms) const {
        if (auto q = dynamic_cast<const query<Value> *>(this))
            return q->template select<Collector>(ms...);
        return wrapped().template select<Collector>(ms...);
    }

    template<typename... Mappers>
    query<std::tuple<typename Mappers::value_type...>>
    select(const Mappers &... ms) const {
        if (auto q = dynamic_cast<const query<Value> *>(this))
            return q->select(ms...);
        return wrapped().select(ms...);
    }

    // Functions in the join() family build junctions or conditional_junctions out of abstract_querys,
    // so there is no need of any conversion to query.
    //
    template<typename V>
    query<V>
    jump(const abstract_query<V> &that, const abstract_predicate &pred) const {
        return inner_join(that, pred).select(*that);
    }

    template<typename Collector, typename V>
    junction<Collector>
    join(const abstract_query<V> &that) const {
        return junction<Collector>(*this, that);
    }

    template<typename Collector, typename V>
    conditional_junction<Collector>
    inner_join(const abstract_query<V> &that, const abstract_predicate &pred) const {
        return conditional_junction<Collector>(
            *this,
            that,
            pred,
            conditional_junction_type::inner,
            (Value *) nullptr,
            (V *) nullptr
        );
    }

    template<typename Collector, typename V>
    conditional_junction<Collector>
    left_join(const abstract_query<V> &that, const abstract_predicate &pred) const {
        return conditional_junction<Collector>(
            *this,
            that,
            pred,
            conditional_junction_type::inner,
            (Value *) nullptr,
            (boost::optional<V> *) nullptr
        );
    }

    template<typename Collector, typename V>
    conditional_junction<Collector>
    right_join(const abstract_query<V> &that, const abstract_predicate &pred) const {
        const conditional_junction_type implementation_type =
            get_database().supports_join(conditional_junction_type::right)
                ? conditional_junction_type::right
                : conditional_junction_type::left;

        return conditional_junction<Collector>(
            *this,
            that,
            pred,
            implementation_type,
            (boost::optional<Value> *) nullptr,
            (V *) nullptr,
            implementation_type == conditional_junction_type::left
        );
    }

    template<typename Collector, typename V>
    conditional_junction<Collector>
    full_join(const abstract_query<V> &that, const abstract_predicate &pred) const {
        if (get_database().supports_join(conditional_junction_type::full))
            return conditional_junction<Collector>(
                *this,
                that,
                pred,
                conditional_junction_type::full,
                (boost::optional<Value> *) nullptr,
                (boost::optional<V> *) nullptr
            );
        else
            throw unsupported_exception();
    }

    template<typename V>
    junction<std::tuple<Value, V>>
    join(const abstract_query<V> &that) const {
        return join<std::tuple<Value, V>>(that);
    }

    template<typename V>
    conditional_junction<std::tuple<Value, V>>
    inner_join(const abstract_query<V> &that, const abstract_predicate &pred) const {
        return inner_join<std::tuple<Value, V>>(that, pred);
    }

    template<typename V>
    conditional_junction<std::tuple<Value, boost::optional<V>>>
    left_join(const abstract_query<V> &that, const abstract_predicate &pred) const {
        return left_join<std::tuple<Value, boost::optional<V>>>(that, pred);
    }

    template<typename V> 
    conditional_junction<std::tuple<boost::optional<Value>, V>>
    right_join(const abstract_query<V> &that, const abstract_predicate &pred) const {
        return right_join<std::tuple<boost::optional<Value>, V>>(that, pred);
    }

    template<typename V>
    conditional_junction<std::tuple<boost::optional<Value>, boost::optional<V>>>
    full_join(const abstract_query<V> &that, const abstract_predicate &pred) const {
        return full_join<std::tuple<boost::optional<Value>, boost::optional<V>>>(that, pred);
    }

    template<typename T>
    T
    evaluate(const exprn_mapper<T> &exprn) const {
        const boost::optional<T> result = select(exprn).get();
        if (! result)  throw no_row_exception();
        return * result;
    }
};

}

#endif
