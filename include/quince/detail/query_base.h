#ifndef QUINCE__mappers__detail__query_base_h
#define QUINCE__mappers__detail__query_base_h

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

#include <string>
#include <vector>
#include <quince/detail/abstract_query.h>
#include <quince/detail/object_id.h>
#include <quince/detail/session.h>
#include <quince/detail/table_base.h>
#include <quince/exprn_mappers/detail/exprn_mapper.h>


namespace quince {

class grouping;
class query_iterator_base;
template<typename T> class query;


// For quince internal use only:
//
enum class combination_type { union_, intersect, except };


// query_base is the base class of all the query<T>s.  It provides all of a query's public
// methods that don't depend on the value type, as well as some methods that are for quince
// internal use only.
//
class query_base : public virtual abstract_query_base, protected object_owner {
public:
    // All of these public members are described at
    // http://quince-lib.com/queries.html and http://quince-lib.com/tables/manipulation.html

    template<typename Mapper>
    query<typename Mapper::value_type>
    select(const Mapper &mapper) const {
        return query<typename Mapper::value_type>(*selectable_equivalent(), mapper);
    }

    template<typename Collector, typename... Mappers>
    query<Collector>
    select(const Mappers &... mappers) const {
        return select(static_mapper_type<Collector>(mappers...));
    }

    template<typename... Mappers>
    query<std::tuple<typename Mappers::value_type...>>
    select(const Mappers &... mappers) const {
        // MSVC won't let me say the obvious thing:
        //      typedef std::tuple<typename Mappers::value_type...> tuple_type;
        // so:
        //
        typedef decltype(std::make_tuple(mappers.value_declval()...)) tuple_type;
        return select<tuple_type>(mappers...);
    }

    // Defined in grouping.h to avoid a circular header dependency.
    //
    template<typename... T>
    grouping
    group(const abstract_mapper<T> &... mappers) const;

    template<typename T>
    void
    update(const abstract_mapper<T> &dest, const abstract_mapper<T> &src) {
        const abstract_mapper_base &src_as_amb = src;
        update_impl(dest, src_as_amb);
    }

    template<typename T>
    void
    update(const abstract_mapper<T> &dest, const T &src) {
        row src_as_row(& get_database());
        dest.to_row(src, src_as_row);
        update_impl(dest, src_as_row);
    }

    template<typename T, typename Result, typename Source>
    Result
    update(const abstract_mapper<T> &dest, const Source &src, const abstract_mapper<Result> &result_mapper) {
        const table_base &t = table();
        return t.update_with_one_output(dest, *make_new_mapper_checked<T>(src), predicate_applicable_to(t), result_mapper);
    }

    template<typename T, typename Result>
    Result
    update(const abstract_mapper<T> &dest, const T &src, const abstract_mapper<Result> &result_mapper) {
        return update(dest, exprn_mapper<T>(src), result_mapper);
    }

    void remove();
    void remove_existing();
    bool remove_if_exists();
    
    std::string to_string() const;


    // --- Everything from here to end of class is for quince internal use only. ---

    virtual ~query_base() = 0;

    virtual const database &get_database() const override;
    virtual void write_table_reference(sql &) const override;
    virtual column_id_set aliased_columns() const override;
    virtual bool a_priori_empty() const override;
    virtual bool might_have_duplicate_rows() const override;
    virtual const object_id &query_id() const override  { return _query_id; }

    // true iff this query's output is the same, disregarding order, as the output of
    // _from.where(_predicate).
    //
    bool is_predicational() const;

    // true iff this query has any set-theoretic combinations at the top level
    // (i.e. those buried inside _from, or subqueries, don't count).
    //
    bool is_combined() const;

    // true iff this query's value mapper was taken without modification from _from.
    //
    bool value_mapper_is_inherited() const;

    // true iff this query could be used as the right-hand side of a set-theoretic
    // combination, without breaking the SQL formation rules that apply on _database.
    //
    bool is_valid_combination_rhs() const;

    // Write the SQL select statement for this query in any context, assuming that the
    // required output columns have been chosen by the caller and recorded in the
    // internal state of cmd, using sql::wanted_alias_scope. (See <quince/detail/sql.h>)
    //
    void write_select(sql &cmd) const;

    // Write the SQL select statement for this query assuming there is no enclosing
    // SQL context, so it will produce all the columns needed by its value mapper.
    //
    void write_maximal_select(sql &cmd) const;

    // Returns the ids of all columns c such that, when write_maximal_select() generates
    // the SQL for this query, it would be advantageous if the SQL context had already
    // defined an SQL column alias for c.
    //
    column_id_set imports() const;

    virtual std::unique_ptr<const query_base> selectable_equivalent() const = 0;

protected:
    // This is a conversion constructor, which creates a query that has from as its _from clause.
    // Not to be confused with the copy constructor, which is compiler generated.
    explicit query_base(const abstract_query_base &from);

    // I want to say:
    //
    //  virtual const abstract_mapper_base &get_value_mapper() const = 0;  // covariant
    //
    // and let the subclasses' get_value_mapper() methods override it.  However that leads to madness, by way of
    // https://connect.microsoft.com/VisualStudio/feedback/details/829684/covariant-with-virtual-inheritance
    // So I define a differently named, non-covariant method instead.
    //
    virtual const abstract_mapper_base &get_value_mapper_base() const = 0;

    void set_table(const table_base *);

    void add_constraint(const abstract_predicate &);
    void set_limit(uint32_t n_rows);
    void set_skip(uint32_t n_rows);
    void set_fetch_size(uint32_t n_rows);
    void set_group_by(std::vector<std::unique_ptr<const abstract_mapper_base >> &&);
    void add_distinct();
    void add_distinct_on(std::vector<std::unique_ptr<const abstract_mapper_base>> &&);
    void add_fake_combine(combination_type, const query_base &rhs);
    void add_real_combine(combination_type, bool all, const query_base &rhs);
    void set_value_mapper_is_inherited(bool);

    const boost::optional<std::vector<const abstract_mapper_base *>> &distinct_list() const;

    void add_orders(std::vector<std::unique_ptr<const abstract_mapper_base>> &&orders_hi_to_lo);
    void clear_orders();

    // true iff this query is eligible to be an operand of a fake set-theoretic combination
    // (i.e. a set theoretic combination that is optimized away).
    //
    bool is_fake_combinable() const;

    // true iff this query's output is the same, disregarding order, as the output of _from.
    //
    bool is_trivial() const;

    // Assuming that some set theoretic combination, with this query on the left and rhs on
    // the right, is allowed, return true iff the fake (i.e. optimized) version is possible.
    //
    bool allow_fake_combine_with(const query_base &rhs) const;

    // Do most of the work of query::begin(), i.e. translate the query into SQL,
    // execute it, and set up an iterator to receive the results.
    //
    void init_iterator(query_iterator_base &) const;

    // Do most of the work of query::get(), i.e. translate the query into SQL,
    // execute it, and receive its result (if any).
    //
    std::unique_ptr<row> fetch_row(const session &) const;

private:
    template<typename T, typename Src>  // Src is abstract_mapper_base or row
    void update_impl(const abstract_mapper<T> &dest, const Src &src) {
        const table_base &t = table();
        t.update_with_no_output(dest, src, predicate_applicable_to(t));
    }

    struct combination;

    predicate predicate_applicable_to(const table_base &) const;

    column_id_set additional_imports() const;

    const table_base &table() const;

    std::vector<const abstract_mapper_base *> all_orders_hi_to_lo() const;

    object_id _query_id;
    object_id::value_type _from_id;
    const abstract_query_base &_from;
    const database &_database;
    bool _value_mapper_is_inherited;
    predicate _predicate;
    boost::optional<std::vector<const abstract_mapper_base *>> _distinct_list;
    std::vector<const abstract_mapper_base *> _group_by;
    std::vector<const abstract_mapper_base *> _orders_lo_to_hi;
    boost::optional<uint32_t> _limit;
    uint32_t _offset;
    uint32_t _fetch_size;
    std::vector<const combination *> _combinations;
};


// query_iterator_base is the base class for all query_iterator<T>s.
//
class query_iterator_base {
public:
    query_iterator_base(const query_iterator_base &);
    ~query_iterator_base();

protected:
    explicit query_iterator_base(const database &);

    const row *advance();
    std::unique_ptr<const row> invalidate() const;

private:
    friend class query_base;
    template<typename Table> friend class iterator;

    void init(const result_stream &);
    const session &get_session() const;

    const database &_database;
    const session _session;  // As long as we hold this, _database will keep using the same session
    result_stream _result_stream;
    std::unique_ptr<const row> _row;
};


/*
    The following function is defined here, away from its own class, to avoid cyclic
    header dependencies.
*/

template<typename CxxType>
void
abstract_mapper<CxxType>::build_match_tester(const query_base &q, predicate &result) const {
    const auto &typed_mapper = dynamic_cast<const exposed_mapper_type<CxxType> &>(*this);

    result = in_unchecked(typed_mapper, q.select(typed_mapper));
}

}

#endif
