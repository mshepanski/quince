#ifndef QUINCE__exprn_mappers__cast_h
#define QUINCE__exprn_mappers__cast_h

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

#include <quince/exprn_mappers/detail/exprn_mapper.h>


namespace quince {

std::unique_ptr<const abstract_expressionist>
make_cast_expressionist(std::unique_ptr<const abstract_mapper_base> arg, column_type (database::*column_type_finder)() const);

template<typename T>
exprn_mapper<T>
cast(const abstract_mapper_base &arg) {
    return exprn_mapper<T>(make_cast_expressionist(
        clone(arg),
        &database::only_column_type<T>
    ));
}

}

#endif
