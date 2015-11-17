#ifndef QUINCE__query_h
#define QUINCE__query_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <assert.h>
#include <boost/optional.hpp>
#include <quince/detail/abstract_query.h>
#include <quince/detail/compiler_specific.h>
#include <quince/detail/query_base.h>
#include <quince/detail/util.h>
#include <quince/exprn_mappers/in.h>
#include <quince/exprn_mappers/functions.h>
#include <quince/database.h>


namespace quince {


QUINCE_SUPPRESS_MSVC_DOMINANCE_WARNING

template<typename Value>
class query : public query_base, public abstract_query<Value> {
public:
    // typedefs for STL compliance:
    //
    typedef Value value_type;
    typedef query_iterator<Value> iterator;
    typedef iterator const_iterator;
    typedef typename abstract_query<Value>::value_mapper value_mapper;

    // This is the implicit conversion ctor, which creates a query that has from as its _from clause.
    // Not to be confused with the copy ctor, which is compiler-generated.
    //
    query(const abstract_query<Value> &from) :
        query_base(from),
        _value_mapper(own(clone(from.get_value_mapper())))
    {}

    // All the following public functions are described at http://quince-lib.com/queries.html

    using query_base::select;
    using query_base::group;
    using query_base::update;

    virtual uint64_t
    size() const override {
        return a_priori_empty() ? 0ULL : countable_equivalent().evaluate(quince::count_all);
    }

    virtual bool
    empty() const override {
        return a_priori_empty() || countable_equivalent().evaluate(quince::empty);
    }

    // TODO: In order to avoid unnecessary query copies, I want to implement limit() as
    // a pair of methods like so:
    //
    //      query
    //      limit(uint32_t n_rows) && {
    //          set_limit(n_rows);
    //          return *this;
    //      }
    //
    //      virtual query
    //      limit(uint32_t n_rows) const override {
    //          limit(query<Value>(*this));
    //      }
    //
    // and similarly for all the other query methods that return a modified copy of *this.
    // However I am waiting for MSVC to support ref-qualified methods.
    //
    // (Caveat: the code above may be wrong --I haven't had the opportunity to compile it--
    // but you get the idea.)

    virtual query
    limit(uint32_t n_rows) const override {
        query<Value> result(*this);
        result.set_limit(n_rows);
        return result;
    }

    virtual query
    skip(uint32_t n_rows) const override {
        query<Value> result(*this);
        result.set_skip(n_rows);
        return result;
    }

    virtual query
    fetch_size(uint32_t n_rows) const override {
        query<Value> result(*this);
        result.set_fetch_size(n_rows);
        return result;
    }

    virtual query<Value>
    where(const abstract_predicate &pred) const override {
        query<Value> result = predicational_equivalent();
        result.add_constraint(pred);
        return result;
    }

    virtual query<Value>
    where(bool b) const override {
        return where(predicate(b));
    }

    template<typename... T>
    query<Value>
    order(const abstract_mapper<T> &... orders) const {
        query<Value> result = predicational_equivalent();
        result.add_orders(make_unique_ptr_vector<const abstract_mapper_base>(clone(orders)... ));
        return result;
    }

    iterator
    virtual begin() const override {
        iterator result(_value_mapper, get_database());
        init_iterator(result);
        if (! a_priori_empty())  result.advance();
        return result;
    }

    iterator
    virtual end() const override {
        return iterator(_value_mapper, get_database());
    }

    boost::optional<Value>
    virtual get() const override {
        if (const std::unique_ptr<row> r = fetch_row(get_database().get_session()))
            return _value_mapper.abstract_mapper<Value>::from_row(*r);
        else
            return boost::none;
    }

    query<Value>
    virtual distinct() const override {
        query<Value> result = trivial_equivalent();
        result.add_distinct();
        return result;
    }

    template<typename... T>
    query<Value>
    distinct_on(const abstract_mapper<T> &... distincts) const {
        query<Value> result = distinct_list()
            ? *this
            : distinct();
        result.add_distinct_on(make_unique_ptr_vector<const abstract_mapper_base>(clone(distincts)... ));
        return result;
    }

    virtual const value_mapper &
    get_value_mapper() const override {
        return _value_mapper;
    }

    virtual const abstract_mapper_base &
    get_value_mapper_base() const override {
        return _value_mapper;
    }

    virtual query<Value> union_(const query<Value> &rhs) const override         { return combine(combination_type::union_, false, rhs); }
    virtual query<Value> union_all(const query<Value> &rhs) const override      { return real_combine(combination_type::union_, true, rhs); }
    virtual query<Value> intersect(const query<Value> &rhs) const override      { return combine(combination_type::intersect, false, rhs); }
    virtual query<Value> intersect_all(const query<Value> &rhs) const override  { return combine(combination_type::intersect, true, rhs); }
    virtual query<Value> except(const query<Value> &rhs) const override         { return combine(combination_type::except, false, rhs); }
    virtual query<Value> except_all(const query<Value> &rhs) const override     { return combine(combination_type::except, true, rhs); }

    virtual void remove() override                  { query_base::remove(); }
    virtual void remove_existing() override         { query_base::remove_existing(); }
    virtual bool remove_if_exists() override        { return query_base::remove_if_exists(); }
    virtual std::string to_string() const override  { return query_base::to_string(); }


    // --- Everything from here to end of class is for quince internal use only. ---

    virtual std::unique_ptr<cloneable>
    clone_impl() const override {
        return quince::make_unique<query<Value>>(*this);
    }

private:
    friend class grouping;
    template<typename> friend class abstract_query;
    friend class query_base;
    template<typename> friend class query;

    // If q is some other query, and you call q.select(...), it comes here to make the new query:
    //
    query(const query_base &that, const value_mapper &selection) :
        query_base(that),
        _value_mapper(own(clone(selection)))
    {
        set_value_mapper_is_inherited(false);
    }

    // If q is some other query, and you call q.group(...).select(...), it comes here to make the new query:
    //
    query(const query_base &that, const value_mapper &selection, std::vector<std::unique_ptr<const abstract_mapper_base>> &&group_by) :
        query(that, selection)
    {
        set_group_by(std::move(group_by));
    }

    query<Value>
    combine(combination_type type, bool all, const query<Value> &rhs) const {
        if (rhs.get_database() != get_database())  throw cross_database_query_exception();
        if (! get_database().supports_combination(type, all))  throw unsupported_exception();

        return allow_fake_combine_with(rhs)
            ? fake_combine(type, rhs)
            : real_combine(type, all, rhs);
    }

    query<Value>
    fake_combine(combination_type type, const query<Value> &rhs) const {
        query<Value> result = *this;
        result.add_fake_combine(type, rhs);
        return result;
    }

    query<Value>
    real_combine(combination_type type, bool all, const query<Value> &rhs) const {
        query<Value> result = *this;
        result.add_real_combine(
            type,
            all,
            rhs.is_valid_combination_rhs() ? rhs : rhs.wrapped()
        );
        return result;
    }

    // return a query that has the same output, value type, and value mapper as *this,
    // but it's predicational.  (See the definition of is_predicational()).
    //
    query<Value>
    predicational_equivalent() const {
        return is_predicational() ? *this : this->wrapped();
    }

    // return a query that has the same output, value type, and value mapper as *this,
    // but it's suitable for evaluating count_all efficiently.
    //
    query<Value>
    countable_equivalent() const {
        query<Value> result = predicational_equivalent();
        result.clear_orders();
        return result;
    }

    // return a query that has the same output, value type, and value mapper as *this,
    // but it's possible to call .select() or .group(...).select(...) on it.
    //
    virtual std::unique_ptr<const query_base>
    selectable_equivalent() const override {
        if (is_combined() || ! value_mapper_is_inherited())
            return this->unique_wrapped();
        else
            return clone(*this);
    }

    // return a query that has the same output, value type, and value mapper as *this,
    // but it's trivial.  (See the definition of is_trivial()).
    //
    query<Value>
    trivial_equivalent() const {
        return is_trivial() ? *this : this->wrapped();
    }

    virtual void
    for_each_column(std::function<void(const column_mapper &)> op) const override {
        column_id_set seen;
        _value_mapper.for_each_column([&](const column_mapper &c) {
            const column_id r = c.id();
            op(c);
            seen.insert(r);
        });

        if (const auto &distincts = distinct_list())
            for (const auto d: *distincts) {
                d->for_each_column([&](const column_mapper &c) {
                    const column_id r = c.id();
                    if (! seen.count(r)) {
                        op(c);
                        seen.insert(r);
                    }
                });
            }
    }

    const value_mapper &_value_mapper;
};

QUINCE_UNSUPPRESS_MSVC_WARNING


// This is the class normally known as query<Value>::const_iterator:
//
template<typename Value>
class query_iterator : private query_iterator_base {
public:
    // typedefs for STL compliance:
    //
    typedef std::input_iterator_tag iterator_category;
    typedef Value value_type;
    typedef size_t difference_type;
    typedef const Value *pointer;
    typedef const Value &reference;

    // These public functions are described at
    // http://quince-lib.com/queries/executing.html#queries.executing.with_multi_record_output_begin_a

    query_iterator(const query_iterator &that) :
        query_iterator_base(that),
        _mapper(that._mapper),
        _value(that.invalidate())
    {}

    bool
    is_end() const {
        return !_value;
    }

    void
    operator++() {
        if (! is_end())  advance();
    }

    void
    operator++(int) {
        operator++();
    }

    const Value &operator*() const  { return *_value; }
    const Value *operator->() const { return _value.get(); }

    bool
    operator==(const query_iterator &that) const {
        assert(that.is_end());
        return is_end();
    }

    bool
    operator!=(const query_iterator &that) const {
        return ! operator==(that);
    }

private:
    friend class query<Value>;

    query_iterator(const abstract_mapper<Value> &mapper, const database &database) :
        query_iterator_base(database),
        _mapper(clone(mapper))
    {}

    void
    advance() {
        std::unique_ptr<Value> new_value;
        if (const row *row = query_iterator_base::advance()) {
            new_value = quince::make_unique<Value>();
            _mapper->from_row(*row, *new_value);
        }
        _value.reset(new_value.release());
    }

    std::unique_ptr<const Value>
    invalidate() const {
        auto &moveable = const_cast<std::unique_ptr<const Value>&>(_value);
        return std::move(moveable);
    }

    std::shared_ptr<const abstract_mapper<Value>> _mapper;

    // I'd like to replace the std::unique_ptr<Value> with boost::optional<Value>,
    // but only when boost::optional supports moves.
    //
    std::unique_ptr<const Value> _value;
};

}

#endif
