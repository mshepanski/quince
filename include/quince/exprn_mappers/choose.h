#ifndef QUINCE__exprn_mappers__choose_h
#define QUINCE__exprn_mappers__choose_h

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

#include <memory>
#include <type_traits>
#include <quince/exprn_mappers/detail/exprn_mapper.h>


namespace quince {

class abstract_expressionist;

struct unique_case_clause {
    unique_case_clause(const unique_case_clause &) = delete;
    unique_case_clause &operator=(const unique_case_clause &) = delete;

    unique_case_clause(unique_case_clause &&that) :
        _when(std::move(that._when)),
        _then(std::move(that._then))
    {}

    unique_case_clause(
        std::unique_ptr<const abstract_mapper_base> when,
        std::unique_ptr<const abstract_mapper_base> then
    ) :
        _when(std::move(when)),
        _then(std::move(then))
    {}

    std::unique_ptr<const abstract_mapper_base> _when;
    std::unique_ptr<const abstract_mapper_base> _then;
};

template<typename When, typename Then>
struct case_clause : private object_owner {
    case_clause(
        std::unique_ptr<abstract_mapper<When>> when,
        std::unique_ptr<abstract_mapper<Then>> then
    ) :
        _when(&own(when)),
        _then(&own(then))
    {}

    // implicit conversion from a default value
    template<typename Default>
    case_clause(const Default &default_) :
        _when(nullptr),
        _then(&own(make_new_mapper_checked<Then>(default_)))
    {}

    const abstract_mapper<When> *_when;
    const abstract_mapper<Then> *_then;
};

template<typename>
struct is_case_clause : public std::false_type
{};

template<typename When, typename Then>
struct is_case_clause<case_clause<When, Then>> : public std::true_type
{};


// Free function rather than method so we get implicit conversion to the argument type on the way in.
//
template<typename When, typename Then>
unique_case_clause
clone_parts(const case_clause<When, Then> &c) {
    std::unique_ptr<const abstract_mapper_base> w;
    if (c._when)  w = clone(*c._when);
    return std::move(unique_case_clause(std::move(w), clone(*c._then)));
}

template<typename When, typename Then>
auto
when(const When &w, const Then &t) ->
    case_clause<
        decltype(make_new_mapper(w)->value_declval()),
        decltype(make_new_mapper(t)->value_declval())
    > {
    return { make_new_mapper(w), make_new_mapper(t) };
}

std::unique_ptr<const abstract_expressionist>
make_case_expressionist(
    std::unique_ptr<const abstract_mapper_base> switch_,  // can be nullptr
    std::vector<unique_case_clause> &&clauses,
    std::unique_ptr<const abstract_mapper_base> backstop  // the default default
);


template<typename When, typename Then>
std::vector<unique_case_clause>
clone_all_parts(const std::vector<case_clause<When, Then>> &clauses) {
    return transform(clauses, &clone_parts<When, Then>);
}


template<typename Then>
exprn_mapper<Then>
choose(const std::vector<case_clause<bool, Then>> &clauses) {
    return exprn_mapper<Then>(make_case_expressionist(
        nullptr,
        clone_all_parts(clauses),
        make_new_mapper(Then())
    ));
}

template<typename Then, typename... OtherClauses>
exprn_mapper<Then>
choose(
    const case_clause<bool, Then> &clause,
    const OtherClauses &... other_clauses
) {
    return choose(std::vector<case_clause<bool, Then>>{clause, other_clauses...});
}

template<typename Switch, typename When, typename Then>
exprn_mapper<Then>
choose(
    const Switch &switch_,
    const std::vector<case_clause<When, Then>> &clauses
) {
    return exprn_mapper<Then>(make_case_expressionist(
        make_new_mapper_checked<When>(switch_),
        clone_all_parts(clauses),
        make_new_mapper(Then())
    ));
}

template<typename Switch, typename When, typename Then, typename... OtherClauses>
typename std::enable_if<
    ! is_case_clause<Switch>::value,
    exprn_mapper<Then>
>::type
choose(
    const Switch &switch_,
    const case_clause<When, Then> &clause,
    const OtherClauses &... other_clauses
) {
    return choose(switch_, std::vector<case_clause<When, Then>>{clause, other_clauses...});
}

}

#endif
