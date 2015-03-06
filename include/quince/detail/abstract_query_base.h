#ifndef QUINCE__mappers__detail__abstract_query_base_h
#define QUINCE__mappers__detail__abstract_query_base_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/noncopyable.hpp>
#include <set>
#include <quince/detail/abstract_column_sequence.h>
#include <quince/detail/util.h>
#include <quince/mappers/detail/column_mapper.h>
#include <quince/mappers/detail/exposed_mapper_type.h>
#include <quince/detail/clone.h>


/*
    Everything in this file is for quince internal use only.
*/

namespace quince {

typedef int64_t column_id;
typedef std::set<column_id> column_id_set;
class database;
class object_id;
class sql;
template<typename> class query;
class table_base;


// Base class of all the abstract_query<T>s.
//
class abstract_query_base : public cloneable, private abstract_column_sequence {

    // Everything in this class is for quince internal use only.

public:
    virtual ~abstract_query_base()  {}

    virtual const database &get_database() const = 0;
    virtual void write_table_reference(sql &) const = 0;
    virtual column_id_set aliased_columns() const = 0;
    virtual bool a_priori_empty() const  { return false; }
    virtual bool might_have_duplicate_rows() const = 0;
    virtual const object_id &query_id() const = 0;

    virtual column_id_set imports() const {
        column_id_set result;
        for_each_column([&](const column_mapper &c) {
            add_to_set(result, c.imports());
        });
        return result;
    }

    // The columns that my base class abstract_column_sequence traverses are
    // the columns that this query exports (to any outer query, or to the final
    // result stream if this is the outermost).  That may not be obvious, so
    // for readability I force the code to say exports() in order to get at
    // that base class.
    //
    const abstract_column_sequence &exports() const { return *this; }
};

}

#endif
