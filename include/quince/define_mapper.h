#ifndef QUINCE__mappers__define_mapper_h
#define QUINCE__mappers__define_mapper_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/mappers/detail/exposed_mapper_type.h>
#include <quince/mappers/detail/user_defined_class_mapper.h>
#include <quince/mappers/class_mapper.h>


// Compose an obscure name for the mapper class for some user class.
// This obscure name will be defined in the same namespace as the user class.
//
#define QUINCE_USER_DEFINED_MAPPER_NAME(CLASS_TYPE) \
    BOOST_PP_CAT(QUINCE_MAPPER_, CLASS_TYPE)


// If a user class's mapped members or mapped bases are not public,
// then it should invoke this macro to let quince see them.
//
#define QUINCE_IS_MY_FRIEND(CLASS_TYPE) \
    friend class QUINCE_USER_DEFINED_MAPPER_NAME(CLASS_TYPE); \
    friend class serial_table<CLASS_TYPE>;


/*
    Each of the following macros does two things:

    1. It defines a mapper class for the user's class CLASS_TYPE.  It is defined in
       the namespace where the macro is invoked, which must be the namespace where
       CLASS_TYPE is defined.

    2. It sets things up so that that type can be referred to as quince::class_mapper<CLASS_TYPE>.
*/
#define QUINCE_MAP_CLASS_WITH_BASES(CLASS_TYPE, BASES, MEMBERS) \
    QUINCE_DEFINE_CLASS_MAPPER_WITH_BASES(QUINCE_USER_DEFINED_MAPPER_NAME(CLASS_TYPE), CLASS_TYPE, BASES, MEMBERS) \
    \
    QUINCE_DEFINE_CLASS_MAPPER_TYPE(CLASS_TYPE, QUINCE_USER_DEFINED_MAPPER_NAME(CLASS_TYPE))

#define QUINCE_MAP_CLASS(CLASS_TYPE, MEMBERS) \
    QUINCE_DEFINE_CLASS_MAPPER(QUINCE_USER_DEFINED_MAPPER_NAME(CLASS_TYPE), CLASS_TYPE, MEMBERS) \
    \
    QUINCE_DEFINE_CLASS_MAPPER_TYPE(CLASS_TYPE, QUINCE_USER_DEFINED_MAPPER_NAME(CLASS_TYPE))

#endif
