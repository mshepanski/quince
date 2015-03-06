#ifndef QUINCE__mappers__detail__table_interface_h
#define QUINCE__mappers__detail__table_interface_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <quince/detail/abstract_query_base.h>


/*
    Everything in this file is for quince internal use only.
*/

namespace quince {

class abstract_mapper_base;
class database;

// Base class of all tables, serial_tables, and table_aliases.
//
class table_interface : public virtual abstract_query_base {
public:
    virtual const std::string &name() const = 0;
    virtual const std::string &basename() const = 0;

protected:
    void initialize_mapper(abstract_mapper_base &) const;
};

}

#endif
