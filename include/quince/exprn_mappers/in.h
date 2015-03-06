#ifndef QUINCE__exprn_mappers__in_h
#define QUINCE__exprn_mappers__in_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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
