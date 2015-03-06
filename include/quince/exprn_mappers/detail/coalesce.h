#ifndef QUINCE__exprn_mappers__detail__coalesce_h
#define QUINCE__exprn_mappers__detail__coalesce_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/exprn_mappers/function.h>


namespace quince {

template<typename Return, typename Fallback>
exprn_mapper<Return>
coalesce(const abstract_mapper<Return> &preferred, const Fallback &fallback) {
    return function<Return>("COALESCE", preferred, fallback);
}

}

#endif
