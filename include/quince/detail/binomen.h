#ifndef QUINCE__mappers__detail__binomen_h
#define QUINCE__mappers__detail__binomen_h

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
