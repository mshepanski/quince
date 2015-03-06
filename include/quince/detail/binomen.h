#ifndef QUINCE__mappers__detail__binomen_h
#define QUINCE__mappers__detail__binomen_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <initializer_list>
#include <string>
#include <boost/optional.hpp>


/*
    Everything in this file is for quince internal use only.
*/

namespace quince {

// http://lmgtfy.com/?q=binomen&l=1

struct binomen {
    // _enclosure is the name of a dbns structure in which tables belong.
    //  E.g. it's a schema name in postgresql, and a database name in sqlite.
    //
    boost::optional<std::string> _enclosure;

    // Currently _local is always a table name, but in future it might be use
    // as the name of an index or anything else that is local to an enclosure.
    //
    std::string _local;

    static binomen split(const std::string &full, const boost::optional<std::string> &default_enclosure = boost::none);

private:
    // Private so you can't accidentally call it when you mean to call split().
    //
    binomen(const boost::optional<std::string> &enclosure, const std::string &local);
};

}

#endif
