#ifndef QUINCE__exprn_mappers__constant_h
#define QUINCE__exprn_mappers__constant_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/exprn_mappers/detail/exprn_mapper.h>

namespace quince {

std::unique_ptr<const abstract_expressionist>
make_constant_expressionist(const std::string &sql_constant_exprn);

template<typename T>
exprn_mapper<T>
constant(const std::string &sql_constant_exprn) {
    return exprn_mapper<T>(make_constant_expressionist(sql_constant_exprn));
}

}

#endif
