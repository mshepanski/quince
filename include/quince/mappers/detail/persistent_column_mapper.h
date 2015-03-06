#ifndef QUINCE__mappers__detail__persistent_column_mapper_h
#define QUINCE__mappers__detail__persistent_column_mapper_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/optional.hpp>
#include <quince/detail/column_id.h>
#include <quince/mappers/detail/column_mapper.h>


namespace quince {

class database;
class sql;
class table_interface;


/*
    Base class for every mapper that represents one column in a table (as opposed to
    multiple columns in a table, or the result of a server-side computation).
*/
class persistent_column_mapper : public column_mapper {

    // Everything in this class is for quince internal use only.

public:
    explicit persistent_column_mapper(const boost::optional<std::string> &name);

    virtual column_id_set imports() const override;

    const table_interface &table() const;
    const std::string &table_basename() const;

    void set_table(const table_interface *);

    virtual void for_each_persistent_column(std::function<void(const persistent_column_mapper &)>) const override;

    virtual void write_expression(sql &) const;

    void check_compatibility(const database &) const;

private:
    const table_interface *_table;
};

}

#endif
