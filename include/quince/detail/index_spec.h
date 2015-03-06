#ifndef QUINCE__mappers__detail__index_spec_h
#define QUINCE__mappers__detail__index_spec_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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
