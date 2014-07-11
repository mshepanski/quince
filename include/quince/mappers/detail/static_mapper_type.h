#ifndef QUINCE__mappers__detail__static_mapper_type_h
#define QUINCE__mappers__detail__static_mapper_type_h

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

#include <tuple>
#include <type_traits>
#include <boost/optional.hpp>
#include <quince/detail/column_type.h>
#include <quince/mappers/class_mapper.h>


/*
    The purpose of this file is to define the alias template "static_mapper_type".
    All the other definitions you see are helpers for static_mapper_type; they are not used outside.

    static_mapper_type<T> is defined whenever T is a statically mapped type (i.e. a boost::optional,
    std::tuple, or a mapped user-defined class).  Then static_mapper_type<T> is T's concrete mapper type.
*/

namespace quince {

class class_mapper_base;
template<typename> class optional_mapper;
template<typename...> class tuple_mapper;


template<typename Class>
struct static_mapper_type_trait {
    typedef class_mapper<Class> type;

    static_assert(std::is_base_of<class_mapper_base, type>::value, "");
};

template<typename T>
struct static_mapper_type_trait<boost::optional<T>> {
    typedef optional_mapper<T> type;
};

template<typename... Es>
struct static_mapper_type_trait<std::tuple<Es...>> {
    typedef tuple_mapper<Es...> type;
};


template<typename T>
using static_mapper_type = typename static_mapper_type_trait<T>::type;

}
#endif
