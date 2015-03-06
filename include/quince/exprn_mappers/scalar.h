#ifndef QUINCE__exprn_mappers__scalar_h
#define QUINCE__exprn_mappers__scalar_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/abstract_query.h>
#include <quince/detail/query_base.h>
#include <quince/exprn_mappers/detail/exprn_mapper.h>


namespace quince {

std::unique_ptr<const abstract_expressionist>
make_scalar_subquery_expressionist(std::unique_ptr<const query_base> query);

template<typename T>
exprn_mapper<T>
scalar(const abstract_query<T> &query) {
    return exprn_mapper<T>(make_scalar_subquery_expressionist(clone(query.limit(1))));
}

}

#endif
