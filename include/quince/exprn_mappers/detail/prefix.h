#ifndef QUINCE__exprn_mappers__detail__prefix_h
#define QUINCE__exprn_mappers__detail__prefix_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/exprn_mappers/detail/exprn_mapper.h>


namespace quince {

std::unique_ptr<const abstract_expressionist>
make_prefix_expressionist(const std::string &op, std::unique_ptr<const abstract_mapper_base> arg);


struct endomorphic_prefix_op {
    std::string _sql_op;

    template<typename T>
    exprn_mapper<T>
    operator()(const abstract_mapper<T> &operand) const {
        return exprn_mapper<T>(make_prefix_expressionist(_sql_op, clone(operand)));
    }
};

}

#endif
