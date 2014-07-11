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

#include <quince/exprn_mappers/collective.h>
#include <quince/detail/query_base.h>
#include <quince/query.h>

using std::unique_ptr;


namespace quince {

const collective_type
collective_base::get_type() const {
    return _type;
}

const query_base &
collective_base::get_query() const {
    return *_query;
}

collective_base::collective_base(
    collective_type type,
    unique_ptr<const query_base> query
) :
    _type(type),
    _query(&own(query))
{};

}