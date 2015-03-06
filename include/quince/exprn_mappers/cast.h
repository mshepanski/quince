#ifndef QUINCE__exprn_mappers__cast_h
#define QUINCE__exprn_mappers__cast_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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
