#ifndef QUINCE__mappers__detail__index_spec_h
#define QUINCE__mappers__detail__index_spec_h

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

#include <vector>


/*
    Everything in this file is for quince internal use only.
*/

namespace quince {

class abstract_mapper_base;

struct index_spec {
    std::vector<const abstract_mapper_base *> _mappers;
    bool _is_unique;

    // TODO: add fields for indexing method, storage parameters, and a predicate (for partial indexes).

    index_spec(const std::vector<const abstract_mapper_base *> &&m, bool u) :
        _mappers(m),
        _is_unique(u)
    {}
};

}

#endif
