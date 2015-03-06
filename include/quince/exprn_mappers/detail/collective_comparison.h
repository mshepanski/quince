#ifndef QUINCE__exprn_mappers__detail__collective_comparison_h
#define QUINCE__exprn_mappers__detail__collective_comparison_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/exprn_mappers/detail/exprn_mapper.h>
#include <quince/exprn_mappers/collective.h>


namespace quince {

enum class relation;

std::unique_ptr<const abstract_expressionist>
make_collective_comparison_expressionist(
    relation r,
    std::unique_ptr<const abstract_mapper_base> lhs,
    std::unique_ptr<const collective_base> rhs
);

template<typename T, typename Lhs>
predicate
collective_comparison(relation r, const Lhs &lhs, const collective<T> &rhs) {
    exposed_mapper_type<T>::static_forbid_optionals();
    return predicate(make_collective_comparison_expressionist(
        r,
        make_new_mapper_checked<T>(lhs),
        clone(rhs)
    ));
}

struct collective_comparison_op {
    relation _r;
    relation _converse_r;

    template<typename T, typename Lhs>
    predicate
    operator()(const Lhs &lhs, const collective<T> &rhs) const {
        return collective_comparison(_r, lhs, rhs);
    }

    template<typename T, typename Rhs>
    predicate
    operator()(const collective<T> &lhs, const Rhs &rhs) const {
        return collective_comparison(_converse_r, rhs, lhs);
    }
};

}

#endif
