#ifndef QUINCE__mappers__abstract_mapper_base_h
#define QUINCE__mappers__abstract_mapper_base_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/abstract_column_sequence.h>
#include <quince/detail/clone.h>
#include <quince/detail/column_id.h>
#include <quince/detail/object_owner.h>


namespace quince {

class persistent_column_mapper;
class sql;
template<typename> class exprn_mapper;
typedef exprn_mapper<bool> predicate;
class query_base;
class table_base;


// Base class of all mappers and exprn_mappers.
//
class abstract_mapper_base :
    public cloneable,
    protected object_owner,
    public abstract_column_sequence
{

    // Everything in this class is for quince internal use only.

public:
    abstract_mapper_base(const boost::optional<std::string> &name);
    virtual ~abstract_mapper_base()  {};

    bool has_name() const;
    std::string name() const;
    void set_name(const std::string &);

    bool can_be_all_null() const;
    virtual void allow_all_null() const;

    // Return the ids of all columns c such that, when write_expression() generates the SQL
    // to evaluate this mapper, it would be advantageous if the SQL context had already defined
    // an SQL column alias for c.
    //
    virtual column_id_set imports() const = 0;

    // If *this is a persistent mapper, i.e. if it represents some columns in a table rather than
    // the result of a server-side computation, then for_each_persistent_column(op) applies
    // op to the mappers for each of those columns.
    //
    virtual void for_each_persistent_column(std::function<void(const persistent_column_mapper &)>) const = 0;

    virtual void write_expression(sql &) const  {}

    virtual std::pair<const abstract_mapper_base *, bool> dissect_as_order_specification() const;

    static void static_forbid_optionals()  {}

protected:
    void forbid_all_null() const;

private:
    friend class query_base;
    friend class table_base;

    // build_match_tester() does an obscure job, for one use-case internal to quince.
    //
    // Given the preconditions that:
    //
    //  1. *this is a mapper applicable to q --Let's say it's q->foo--, and
    //  2. *this does not use optional_mappers at any level
    //
    // then build_match_tester() sets result to the predicate:
    //
    //      in(q->foo, q.select(q->foo))
    //
    // Then, if q->foo is also a mapper applicable to something else, e.g. table t,
    // (because it just so happens that t's mapper object is identical to foo's),
    // then result can be used like so:
    //
    //      t.where(result)
    //
    // to find rows of t whose foo member matches the foo members anywhere in t's
    // output.
    //
    // The only reason for result parameter rather than a return is to duck
    // cyclic header dependency problems.
    //
    virtual void build_match_tester(const query_base &q, predicate &result) const = 0;

    boost::optional<std::string> _name;
    const table_base *_table_whose_value_mapper_i_am;
    mutable bool _can_be_all_null;
};

template<typename Mapper>
static std::vector<std::unique_ptr<const abstract_mapper_base>>
clone_all(const std::vector<Mapper *> &mappers) {
    return transform(
        mappers,
        [](const Mapper *m) -> std::unique_ptr<const abstract_mapper_base> { return clone(*m); }
    );
}

}

#endif
