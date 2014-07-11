#ifndef QUINCE__mappers__detail__table_interface_h
#define QUINCE__mappers__detail__table_interface_h

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
