#ifndef QUINCE__mappers__class_mapper_h
#define QUINCE__mappers__class_mapper_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// This macro creates an association, at compile time, between a user's class type and
// its mapper type: an association that can be looked up via ADL.
// 
// (Note that the various overloads of QUINCE_static_lookup_class_mapper_type() are only
// ever declared, and referenced via decltype() -- never defined.)
//
#define QUINCE_DEFINE_CLASS_MAPPER_TYPE(USER_CLASS, MAPPER_TYPE) \
    MAPPER_TYPE QUINCE_static_lookup_class_mapper_type(USER_CLASS);

namespace quince {

// Look up an association created by QUINCE_DEFINE_CLASS_MAPPER_TYPE().
// (Any user class defined with QUINCE_MAP_CLASS or QUINCE_MAP_CLASS_WITH_BASES
// wlll have such an association.)
//
template<typename UserClass>
using class_mapper = decltype(QUINCE_static_lookup_class_mapper_type(std::declval<UserClass>()));

}

#endif
