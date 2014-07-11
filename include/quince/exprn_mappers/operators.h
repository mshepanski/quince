#ifndef QUINCE__exprn_mappers__operators_h
#define QUINCE__exprn_mappers__operators_h

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

#include <quince/detail/util.h>
#include <quince/exprn_mappers/detail/collective_comparison.h>
#include <quince/exprn_mappers/detail/lexicographic_comparison.h>
#include <quince/exprn_mappers/detail/prefix.h>


namespace quince {

#ifdef __GNUC__
  static const char * const plus = "+";
  static const char * const dash = "-";
  QUINCE_OVERLOAD_UNARY_OPERATOR(+, endomorphic_prefix_op{plus});
  QUINCE_OVERLOAD_UNARY_OPERATOR(-, endomorphic_prefix_op{dash});
#else
  QUINCE_OVERLOAD_UNARY_OPERATOR(+, endomorphic_prefix_op{"+"});
  QUINCE_OVERLOAD_UNARY_OPERATOR(-, endomorphic_prefix_op{"-"});
#endif

struct addition_op : public endomorphic_infix_op {
    addition_op() :
        endomorphic_infix_op({"+"}),
        strcat_op({"||"})
    {}

    using endomorphic_infix_op::operator();

    exprn_mapper<std::string>
    operator()(const abstract_mapper<std::string> &lhs, const abstract_mapper<std::string> &rhs) const {
        return strcat_op(lhs, rhs);
    }

    template<typename Rhs>
    exprn_mapper<std::string>
    operator()(const abstract_mapper<std::string> &lhs, const Rhs &rhs) const {
        return strcat_op(lhs, rhs);
    }

    template<typename Lhs>
    exprn_mapper<std::string>
    operator()(const Lhs &lhs, const abstract_mapper<std::string> &rhs) const {
        return strcat_op(lhs, rhs);
    }

private:
    const endomorphic_infix_op strcat_op;
};

QUINCE_OVERLOAD_BINARY_OPERATOR(+, addition_op())

#ifdef __GNUC__
  static const char * const star = "*";
  static const char * const slash = "/";
  static const char * const percent = "%";
  static const char * const amper = "&";
  static const char * const bar = "|";
  QUINCE_OVERLOAD_BINARY_OPERATOR(-, endomorphic_infix_op{dash})
  QUINCE_OVERLOAD_BINARY_OPERATOR(*, endomorphic_infix_op{star})
  QUINCE_OVERLOAD_BINARY_OPERATOR(/, endomorphic_infix_op{slash})
  QUINCE_OVERLOAD_BINARY_OPERATOR(%, endomorphic_infix_op{percent})
  QUINCE_OVERLOAD_BINARY_OPERATOR(&, endomorphic_infix_op{amper})
  QUINCE_OVERLOAD_BINARY_OPERATOR(|, endomorphic_infix_op{bar})
#else
  QUINCE_OVERLOAD_BINARY_OPERATOR(-, endomorphic_infix_op{"-"})
  QUINCE_OVERLOAD_BINARY_OPERATOR(*, endomorphic_infix_op{"*"})
  QUINCE_OVERLOAD_BINARY_OPERATOR(/, endomorphic_infix_op{"/"})
  QUINCE_OVERLOAD_BINARY_OPERATOR(%, endomorphic_infix_op{"%"})
  QUINCE_OVERLOAD_BINARY_OPERATOR(&, endomorphic_infix_op{"&"})
  QUINCE_OVERLOAD_BINARY_OPERATOR(|, endomorphic_infix_op{"|"})
#endif

QUINCE_OVERLOAD_BINARY_OPERATOR(==, (lexicographic_comparison_op{relation::equal, relation::equal}));
QUINCE_OVERLOAD_BINARY_OPERATOR(!=, (lexicographic_comparison_op{relation::not_equal, relation::not_equal}));
QUINCE_OVERLOAD_BINARY_OPERATOR(<,  (lexicographic_comparison_op{relation::less, relation::greater}));
QUINCE_OVERLOAD_BINARY_OPERATOR(>,  (lexicographic_comparison_op{relation::greater, relation::less}));
QUINCE_OVERLOAD_BINARY_OPERATOR(<=, (lexicographic_comparison_op{relation::less_or_equal, relation::greater_or_equal}));
QUINCE_OVERLOAD_BINARY_OPERATOR(>=, (lexicographic_comparison_op{relation::greater_or_equal, relation::less_or_equal}));

QUINCE_OVERLOAD_BINARY_OPERATOR(==, (collective_comparison_op{relation::equal, relation::equal}));
QUINCE_OVERLOAD_BINARY_OPERATOR(!=, (collective_comparison_op{relation::not_equal, relation::not_equal}));
QUINCE_OVERLOAD_BINARY_OPERATOR(<,  (collective_comparison_op{relation::less, relation::greater}));
QUINCE_OVERLOAD_BINARY_OPERATOR(>,  (collective_comparison_op{relation::greater, relation::less}));
QUINCE_OVERLOAD_BINARY_OPERATOR(<=, (collective_comparison_op{relation::less_or_equal, relation::greater_or_equal}));
QUINCE_OVERLOAD_BINARY_OPERATOR(>=, (collective_comparison_op{relation::greater_or_equal, relation::less_or_equal}));


predicate operator&&(const abstract_predicate &lhs, const abstract_predicate &rhs);
predicate operator||(const abstract_predicate &lhs, const abstract_predicate &rhs);

predicate operator&&(bool lhs, const abstract_predicate &rhs);
predicate operator||(bool lhs, const abstract_predicate &rhs);
predicate operator==(bool lhs, const abstract_predicate &rhs);  // for optimization purposes only
predicate operator!=(bool lhs, const abstract_predicate &rhs);  // for optimization purposes only

predicate operator&&(const abstract_predicate &lhs, bool rhs);
predicate operator||(const abstract_predicate &lhs, bool rhs);
predicate operator==(const abstract_predicate &lhs, bool rhs);  // for optimization purposes only
predicate operator!=(const abstract_predicate &lhs, bool rhs);  // for optimization purposes only

predicate operator!(const abstract_predicate &);

}

#endif
