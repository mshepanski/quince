#ifndef QUINCE__exprn_mappers__in_h
#define QUINCE__exprn_mappers__in_h

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

#include <quince/detail/abstract_query.h>
#include <quince/detail/query_base.h>
#include <quince/exprn_mappers/detail/exprn_mapper.h>


namespace quince {

std::unique_ptr<const abstract_expressionist>
make_in_expressionist(
    std::unique_ptr<const abstract_mapper_base> lhs,
    std::vector<std::unique_ptr<const abstract_mapper_base>> &&rhs
);

std::unique_ptr<const abstract_expressionist>
make_in_expressionist(
    std::unique_ptr<const abstract_mapper_base> lhs,
    std::unique_ptr<const query_base> rhs
);


template<typename T, typename Lhs>
predicate
in(const Lhs &lhs, const std::vector<const abstract_mapper<T> *> &rhs) {
    exposed_mapper_type<T>::static_forbid_optionals();

    return predicate(make_in_expressionist(
        make_new_mapper_checked<T>(lhs),
        clone_all(rhs)
    ));
}

template<typename Lhs, typename... Rhses>
predicate
in(const Lhs &lhs, const Rhses &... rhses) {
    typedef decltype(make_new_mapper(lhs)->value_declval()) value_type;
    exposed_mapper_type<value_type>::static_forbid_optionals();

    return predicate(make_in_expressionist(
        make_new_mapper(lhs),
        make_new_mappers_checked<value_type>(rhses...)
    ));
}

template<typename T, typename Lhs>
predicate
in_unchecked(const Lhs &lhs, const query<T> &rhs) {
    return predicate(make_in_expressionist(
        make_new_mapper_checked<T>(lhs),
        clone(rhs)
    ));
}

template<typename T, typename Lhs>
predicate
in(const Lhs &lhs, const query<T> &rhs) {
    exposed_mapper_type<T>::static_forbid_optionals();
    return in_unchecked(lhs, rhs);
}

template<typename T, typename Lhs>
predicate
in(const Lhs &lhs, const abstract_query<T> &rhs) {
    return in(lhs, rhs.wrapped());
}

template<typename T, typename Lhs>
predicate
in(const Lhs &lhs, const query<T> &&rhs) {
    return in<T, Lhs>(lhs, rhs);
}

template<typename T, typename Lhs>
predicate
in(const Lhs &lhs, const abstract_query<T> &&rhs) {
    return in<T, Lhs>(lhs, rhs);
}

}

#endif
