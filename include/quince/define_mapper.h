#ifndef QUINCE__mappers__define_mapper_h
#define QUINCE__mappers__define_mapper_h

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
