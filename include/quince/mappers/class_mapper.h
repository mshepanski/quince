#ifndef QUINCE__mappers__class_mapper_h
#define QUINCE__mappers__class_mapper_h

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
