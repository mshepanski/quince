#ifndef QUINCE__exprn_mappers__exists_h
#define QUINCE__exprn_mappers__exists_h

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

#include <quince/detail/abstract_query.h>
#include <quince/detail/query_base.h>
#include <quince/exprn_mappers/detail/exprn_mapper.h>


namespace quince {

std::unique_ptr<const abstract_expressionist>
make_exists_expressionist(std::unique_ptr<const query_base> query);

template<typename T>
predicate
exists(const abstract_query<T> &query) {
    return predicate(make_exists_expressionist(clone(query.limit(1))));
}

}

#endif
