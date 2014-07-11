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

#include <quince/detail/compiler_specific.h>
#include <quince/detail/sql.h>
#include <quince/detail/query_base.h>
#include <quince/exprn_mappers/exists.h>
#include <quince/query.h>

using std::make_unique;
using std::unique_ptr;


namespace quince {

unique_ptr<const abstract_expressionist>
make_exists_expressionist(
    unique_ptr<const query_base> query
) {
    struct expressionist : public abstract_expressionist {
        const query_base &_query;

        expressionist(unique_ptr<const query_base> &query) :
            _query(own(query))
        {}

        virtual void write_expression(sql &cmd) const override  { cmd.write_subquery_exists(_query); }
        virtual column_id_set imports() const override          { return _query.imports(); }
    };

    return make_unique<expressionist>(query);
}


predicate
exists(const query_base &query) {
    return predicate(make_exists_expressionist(clone(query)));
}

}