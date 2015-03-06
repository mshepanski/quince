//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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