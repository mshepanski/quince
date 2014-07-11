#ifndef QUINCE__mappers__detail__row_h
#define QUINCE__mappers__detail__row_h

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

#include <map>
#include <set>
#include <stdint.h>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <quince/detail/cell.h>


/*
    Everything in this file is for quince internal use only.
*/

namespace quince {

class database;
class abstract_column_sequence;


// A row represents data going in and out of a query in an intermediate form, between mappers and
// basic conversions.
//
// When data is going outwards, it is converted from some mapped type T to a row by T's mapper
// (which may invoke other mappers to help it, all the way down to one or more direct_mappers
// that actually put the data into the row).  Then the backend library converts the row to the
// "on-the-wire" formats that the database vendor's client library (e.g. libpq, sqlite3) expects.
//
// The process is reversed when data is going inwards, from the DBMS to the application.
//
class row {
public:
    explicit row(const database *);

    const database &get_database() const;

    template<typename CxxType>
    void
    add(const CxxType &value) {
        add_cell(cell(value));
    }

    template<typename CxxType>
    void
    add(const std::string name, const CxxType &value) {
        add_cell(cell(value), name);
    }

    void delete_if_exists(const std::string &name);

    template<typename CxxType>
    bool
    get(const std::string &name, CxxType &value) const {
        const cell *const c = find_cell(name);
        if (!c || ! c->has_value())  return false;

        c->get(value);
        return true;
    }

    template<typename CxxType>
    void
    get(CxxType &value) const {
        only_cell().get(value);
    }

    template<typename CxxType>
    CxxType
    get() const {
        CxxType result;
        get(result);
        return(result);
    }

    void add_cell(const cell &);
    void add_cell(const cell &, const std::string &name);
    void add_cells(const std::vector<cell> &);
    void add_cells(const row &);

    std::vector<cell> values() const;

    boost::optional<row> pick(const std::vector<std::string> &names) const;

    const cell &only_cell() const;
    const cell *find_cell(const std::string &name) const;

private:
    typedef std::map<std::string, uint32_t> map;

    const database *_database;
    std::vector<cell> _cells;
    map _map;
};

uint64_t
get_as_count(const row &row);

}

#endif
