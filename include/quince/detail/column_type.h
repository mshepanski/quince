#ifndef QUINCE__mappers__detail__column_type_h
#define QUINCE__mappers__detail__column_type_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <stdint.h>
#include <string>
#include <type_traits>
#include <vector>
#include <boost/optional.hpp>
#include <quince/serial.h>
#include <quince/detail/timestamp.h>
#include <quince/detail/time_type.h>
#include <quince/detail/date_type.h>


/*
    Everything in this file is for quince internal use only.
*/

namespace quince {

// column_types are tags for C++ types that can be held in a cell.  The correspondence
// between C++ types and column_type values is established by the series of QUINCE_SPECIFY_COLUMN_TYPE
// lines below.
//
enum class column_type {
    boolean,
    small_int,
    integer,
    big_int,
    big_serial,
    floating_point,
    double_precision,
    string,
    timestamp,
    time_type,
    date_type,
    byte_vector,
    none
};

}

#define QUINCE_SPECIFY_COLUMN_TYPE(CXX_TYPE, COLUMN_TYPE) \
    quince::column_type \
    inline QUINCE_get_column_type_via_adl(CXX_TYPE *) { \
        return COLUMN_TYPE; \
    } \

namespace quince {

// These are the C++ types for which backend libraries are allowed to provide basic conversions,
// and hence they are the types for which direct_mappers are possible.  See:
// http://quince-lib.com/custom/predefined.html#custom.predefined.direct_mapper_t .
//
QUINCE_SPECIFY_COLUMN_TYPE(bool,                    column_type::boolean)
QUINCE_SPECIFY_COLUMN_TYPE(int16_t,                 column_type::small_int)
QUINCE_SPECIFY_COLUMN_TYPE(int32_t,                 column_type::integer)
QUINCE_SPECIFY_COLUMN_TYPE(int64_t,                 column_type::big_int)
QUINCE_SPECIFY_COLUMN_TYPE(serial,                  column_type::big_serial)
QUINCE_SPECIFY_COLUMN_TYPE(float,                   column_type::floating_point)
QUINCE_SPECIFY_COLUMN_TYPE(double,                  column_type::double_precision)
QUINCE_SPECIFY_COLUMN_TYPE(std::string,             column_type::string)
QUINCE_SPECIFY_COLUMN_TYPE(quince::timestamp,       column_type::timestamp)
QUINCE_SPECIFY_COLUMN_TYPE(quince::time_type,       column_type::time_type)
QUINCE_SPECIFY_COLUMN_TYPE(quince::date_type,       column_type::date_type)
QUINCE_SPECIFY_COLUMN_TYPE(std::vector<uint8_t>,    column_type::byte_vector)
QUINCE_SPECIFY_COLUMN_TYPE(boost::none_t,           column_type::none)


// get_column_type<T>() looks up the column_type c that was defined somewhere with
// QUINCE_SPECIFY_COLUMN(T, c).  Normally that "somewhere" is one of the lines above,
// but it can also be a pseudo-column type defined with QUINCE_DEFINE_SERVER_ONLY_TYPE
// (see <exprn_mappers/detail/exprn_mapper.h>).
//
template<typename T>
column_type get_column_type() {
    return QUINCE_get_column_type_via_adl((T*) nullptr);
}

}

#endif
