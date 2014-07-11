#ifndef QUINCE__mappers__detail__column_id_h
#define QUINCE__mappers__detail__column_id_h

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

#include <set>
#include <stdint.h>
#include <string>


/*
    Everything in this file is for quince internal use only.
*/

namespace quince {

typedef int64_t column_id;
typedef std::set<column_id> column_id_set;

column_id next_column_id();
static const column_id wildcard_column_id = static_cast<column_id>(-1);

}

#endif
