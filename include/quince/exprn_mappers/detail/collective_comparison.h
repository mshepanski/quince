#ifndef QUINCE__exprn_mappers__detail__collective_comparison_h
#define QUINCE__exprn_mappers__detail__collective_comparison_h

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
