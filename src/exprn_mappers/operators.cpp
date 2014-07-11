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

#include <quince/exprn_mappers/detail/infix.h>
#include <quince/exprn_mappers/detail/prefix.h>
#include <quince/exprn_mappers/operators.h>
#include <quince/query.h>

using boost::optional;


namespace quince {

namespace {
    const predicate
    make_concrete(const abstract_predicate &ap) {
        if (const auto * concrete = dynamic_cast<const predicate *>(&ap))
            return *concrete;
        else
            return predicate(ap);
    }
}

predicate
operator&&(const abstract_predicate &lhs, const abstract_predicate &rhs) {
    return
        a_priori_true(lhs) ?    make_concrete(rhs) :
        a_priori_true(rhs) ?    make_concrete(lhs) :
        a_priori_false(lhs) ?   predicate(false) :
        a_priori_false(rhs) ?   predicate(false) :
                                predicate(make_infix_expressionist(" AND ", clone(lhs), clone(rhs)));
}

predicate
operator||(const abstract_predicate &lhs, const abstract_predicate &rhs) {
    return
        a_priori_false(lhs) ?   make_concrete(rhs) :
        a_priori_false(rhs) ?   make_concrete(lhs) :
        a_priori_true(lhs) ?    predicate(true) :
        a_priori_true(rhs) ?    predicate(true) :
                                predicate(make_infix_expressionist(" OR ", clone(lhs), clone(rhs)));
}

predicate
operator&&(bool lhs, const abstract_predicate &rhs) {
    return predicate(lhs) && rhs;
}

predicate
operator||(bool lhs, const abstract_predicate &rhs) {
    return predicate(lhs) || rhs;
}

predicate
operator==(bool lhs, const abstract_predicate &rhs) {
    return lhs ? make_concrete(rhs) : !rhs;
}

predicate
operator!=(bool lhs, const abstract_predicate &rhs) {
    return (!lhs) ==  rhs;
}

predicate
operator&&(const abstract_predicate &lhs, bool rhs) {
    return lhs && predicate(rhs);
}

predicate
operator||(const abstract_predicate &lhs, bool rhs) {
    return lhs || predicate(rhs);
}

predicate
operator==(const abstract_predicate &lhs, bool rhs) {
    return rhs == lhs;
}

predicate
operator!=(const abstract_predicate &lhs, bool rhs) {
    return rhs != lhs;
}

predicate
operator!(const abstract_predicate &operand) {
    if (const optional<bool> value = operand.a_priori_value())
        return predicate(! value);
    else
        return predicate(make_prefix_expressionist("NOT", clone(operand)));
}
}
