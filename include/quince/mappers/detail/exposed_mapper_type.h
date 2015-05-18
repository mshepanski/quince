#ifndef QUINCE__mappers__detail__exposed_mapper_type_h
#define QUINCE__mappers__detail__exposed_mapper_type_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <stdint.h>
#include <string>
#include <vector>
#include <type_traits>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <quince/serial.h>
#include <quince/detail/column_type.h>
#include <quince/mappers/detail/static_mapper_type.h>


/*
    The purpose of this file is to define the alias template "exposed_mapper_type".
    All the other definitions you see are helpers for exposed_mapper_type; they are not used outside.

    For any mapped type T, exposed_mapper_type<T> is the mapper type for T that will be
    visible to application code.  There are two cases:

    1.  If T is an polymorphically mapped type (i.e. an arithmetic type, serial,
        std::string, vector<uint8_t>, or an artefact of QUINCE_DEFINE_SERVER_ONLY_TYPE)
        then exposed_mapper_type<T> is an alias of abstract_mapper<T>.
    
    2.  It T is a statically mapped type (i.e. a boost::optional, std::tuple, or
        a mapped user-defined class), then exposed_mapper_type<T> is an alias of
        static_mapper_type<T>.
*/

namespace quince {

template<typename> class abstract_mapper;

// TODO: Look for a more future-proof way to make the static decision between static and
// polymorphic mapping.
//
// What I want to say is: if static_mapper_type<T> is defined, then T is statically mapped,
// otherwise it's polymorphically mapped.  But I've wasted too much of my life trying to
// say that.

template<typename T>
class is_polymorphically_mapped : public std::integral_constant<
    bool,
        std::is_arithmetic<T>::value
    ||  std::is_same<T, serial>::value
    ||  std::is_same<T, std::string>::value
    ||  std::is_same<T, timestamp>::value
    ||  std::is_same<T, std::vector<uint8_t>>::value
    ||  std::is_same<T, boost::posix_time::ptime>::value
    ||  std::is_empty<T>::value  // for types defined by QUINCE_DEFINE_SERVER_ONLY_TYPE
>
{};

template<typename T, typename Enable = void> struct exposed_mapper_type_trait;

template<typename T>
struct exposed_mapper_type_trait<
    T,
    typename std::enable_if<is_polymorphically_mapped<T>::value>::type
> {
    typedef abstract_mapper<T> type;
};

template<typename T>
struct exposed_mapper_type_trait<
    T,
    typename std::enable_if<! is_polymorphically_mapped<T>::value>::type
> {
    typedef static_mapper_type<T> type;
};


template<typename T>
using exposed_mapper_type = typename exposed_mapper_type_trait<T>::type;


}
#endif
