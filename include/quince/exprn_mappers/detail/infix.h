#ifndef QUINCE__exprn_mappers__detail__infix_h
#define QUINCE__exprn_mappers__detail__infix_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <type_traits>
#include <quince/exprn_mappers/detail/exprn_mapper.h>


namespace quince {

std::unique_ptr<const abstract_expressionist>
make_infix_expressionist(
    const std::string &op,
    std::unique_ptr<const abstract_mapper_base> lhs,
    std::unique_ptr<const abstract_mapper_base> rhs
);


struct endomorphic_infix_op {
    std::string _sql_op;

    template<typename T>
    exprn_mapper<T>
    operator()(const abstract_mapper<T> &lhs, const abstract_mapper<T> &rhs) const {
        return exprn_mapper<T>(make_infix_expressionist(
            _sql_op,
            clone(lhs),
            clone(rhs)
        ));
    }

    template<typename T, typename Rhs>
    exprn_mapper<T>
    operator()(
        const abstract_mapper<T> &lhs,
        const Rhs &rhs,
        typename std::enable_if<! std::is_base_of<abstract_mapper<T>, Rhs>::value>::type * = nullptr
    ) const {
        return exprn_mapper<T>(make_infix_expressionist(
            _sql_op,
            clone(lhs),
            make_new_mapper_checked<T>(rhs)
        ));
    }

    template<typename T, typename Lhs>
    exprn_mapper<T>
    operator()(
        const Lhs &lhs,
        const abstract_mapper<T> &rhs,
        typename std::enable_if<! std::is_base_of<abstract_mapper<T>, Lhs>::value>::type * = nullptr
    ) const {
        return exprn_mapper<T>(make_infix_expressionist(
            _sql_op,
            make_new_mapper_checked<T>(lhs),
            clone(rhs)
        ));
    }
};

template<typename Return>
struct symmetric_infix_op {
    std::string _sql_op;

    template<typename T>
    exprn_mapper<Return>
    operator()(const abstract_mapper<T> &lhs, const abstract_mapper<T> &rhs) const {
        return exprn_mapper<Return>(make_infix_expressionist(
            _sql_op,
            clone(lhs),
            clone(rhs)
        ));
    }

    template<typename T, typename Rhs>
    exprn_mapper<Return>
    operator()(
        const abstract_mapper<T> &lhs,
        const Rhs &rhs,
        typename std::enable_if<! std::is_base_of<abstract_mapper<T>, Rhs>::value>::type * = nullptr
    ) const {
        return exprn_mapper<Return>(make_infix_expressionist(
            _sql_op,
            clone(lhs),
            make_new_mapper_checked<T>(rhs)
        ));
    }

    template<typename T, typename Lhs>
    exprn_mapper<Return>
    operator()(
        const Lhs &lhs,
        const abstract_mapper<T> &rhs,
        typename std::enable_if<! std::is_base_of<abstract_mapper<T>, Lhs>::value>::type * = nullptr
    ) const {
        return exprn_mapper<Return>(make_infix_expressionist(
            _sql_op,
            make_new_mapper_checked<T>(lhs),
            clone(rhs)
        ));
    }
};

typedef symmetric_infix_op<bool> symmetric_infix_predicate;

}

#endif
