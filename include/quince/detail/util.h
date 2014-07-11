#ifndef QUINCE__mappers__detail__util_h
#define QUINCE__mappers__detail__util_h

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

#include <algorithm>
#include <assert.h>
#include <memory>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>
#include <boost/implicit_cast.hpp>
#include <boost/optional.hpp>


/*
    Everything in this file is for quince internal use only.
*/

namespace quince {

template<typename T>
std::set<T>
to_set(const std::vector<T> &v) {
    std::set<T> result;
    result.insert(v.begin(), v.end());
    return result;
}

template<typename T>
void
add_to_set(std::set<T> &target, const std::set<T> &addition) {
    target.insert(addition.begin(), addition.end());
}

template<typename T>
std::set<T>
set_union(const std::set<T> &a, const std::set<T> &b) {
    std::set<T> result = a;
    add_to_set(result, b);
    return result;
}

template<typename T>
std::set<T>
set_difference(const std::set<T> &a, const std::set<T> &b) {
    std::set<T> result;
    std::set_difference(a.begin(), a.end(), b.begin(), b.end(), std::inserter(result, result.end()));
    return result;
}

template<typename T>
bool
is_subset(const std::set<T> &sub, const std::set<T> &super) {
    return set_difference(sub, super).empty();
}

template<typename Map>
boost::optional<const typename Map::mapped_type &>
lookup(const Map &map, const typename Map::key_type &key) {
    const auto iter = map.find(key);
    if (iter == map.end())  return boost::none;
    else                    return iter->second;
}

template<typename InputIter, typename Op>
auto
transform_impl(InputIter begin, InputIter end, const Op &op) -> std::vector<decltype(op(*begin))> {
    typedef decltype(op(*begin)) Out;
    std::vector<Out> result;
    std::transform(begin, end, std::back_inserter(result), op);
    return result;
}

template<typename In, typename Op>
auto
transform(const std::vector<In> &in, const Op &op) -> std::vector<decltype(op(in.front()))> {
    return transform_impl(in.begin(), in.end(), op);
}

template<typename In, typename Op>
auto
transform(std::vector<In> &&in, const Op &op) -> std::vector<decltype(op(in.front()))> {
    return transform_impl(in.begin(), in.end(), op);
}

template<typename Out, typename... In>
std::vector<std::unique_ptr<Out>>
make_unique_ptr_vector(std::unique_ptr<In>... in) {
    std::unique_ptr<Out> unique_ptrs[] = { std::move(in)... };

    return std::vector<std::unique_ptr<Out>>(
        std::make_move_iterator(std::begin(unique_ptrs)),
        std::make_move_iterator(std::end(unique_ptrs))
    );
}

// A universalizable_set<T> does a similar job to std::set<T>, with one difference.
//
// a universalizable_set<T> can have the value "universal", which is kind of the opposite
// of an empty set.  Just as, with an empty set, the question "Is X a member?" always has
// the answer "no", with a universal set, that question always has the answer "yes".
//
template<typename T>
class universalizable_set {
public:
    universalizable_set(const std::set<T> &s = {}) :
        _set(s)
    {}

    static universalizable_set<T>
    universal() {
        return universalizable_set<T>(boost::none);
    }

    bool
    is_universal() const {
        return !_set;
    }

    std::set<T>
    to_set() const {
        assert(! is_universal());
        return *_set;
    }

    typename std::set<T>::size_type
    count(typename std::set<T>::value_type v) const {
        return is_universal() ? 1 : _set->count(v);
    }

    void
    insert(const universalizable_set<T> &that) {
        if (is_universal())
            return;
        else if (that.is_universal())
            *this = universal();
        else
            add_to_set(*_set, that.to_set());
    }

private:
    universalizable_set(boost::none_t) : _set(boost::none)  {}

    // We use _set == boost::none to represent the universal set.
    //
    boost::optional<std::set<T>> _set;
};

template<typename>
struct is_optional : public std::false_type
{};

template<typename T>
struct is_optional<boost::optional<T>> : public std::true_type
{};


template<typename PtrToMember>
struct member_from_ptr_to_member_trait;

template<typename Class, typename CxxMemberType>
struct member_from_ptr_to_member_trait<CxxMemberType Class::*> {
    typedef CxxMemberType type;
};

template<typename PtrToMember>
using member_from_ptr_to_member = typename member_from_ptr_to_member_trait<PtrToMember>::type;


#ifdef _MSC_VER
  //
  //  See:
  //    https://connect.microsoft.com/VisualStudio/feedback/details/753170/cannot-overload-template-functions-differing-only-by-unknown-type-return-type
  //    http://stackoverflow.com/questions/22040500/templated-definitions-of-operator-overloads
  //
  //
# define WORKAROUND_MSVC_BUG_753170(TYPE)  typename std::conditional<true, TYPE, std::integral_constant<int, __COUNTER__>>::type
#else
# define WORKAROUND_MSVC_BUG_753170(TYPE)  TYPE
#endif


// Overload the unary operator OPERATOR with all the overloads of FUNCTOR's operator()().
//
#define QUINCE_OVERLOAD_UNARY_OPERATOR(OPERATOR, FUNCTOR) \
    template<typename Operand> \
    WORKAROUND_MSVC_BUG_753170(decltype(FUNCTOR(std::declval<Operand>()))) \
    operator OPERATOR(const Operand &operand) { \
        return FUNCTOR(operand); \
    }

// Overload the binary operator OPERATOR with all the overloads of FUNCTOR's operator()().
//
#define QUINCE_OVERLOAD_BINARY_OPERATOR(OPERATOR, FUNCTOR) \
    template<typename Operand1, typename Operand2> \
    WORKAROUND_MSVC_BUG_753170(decltype(FUNCTOR(std::declval<Operand1>(), std::declval<Operand2>()))) \
    operator OPERATOR(const Operand1 &operand1, const Operand2 &operand2) { \
        return FUNCTOR(operand1, operand2); \
    }

}

#endif
