#ifndef QUINCE__exprn_mappers__detail__postfix_h
#define QUINCE__exprn_mappers__detail__postfix_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/exprn_mappers/detail/exprn_mapper.h>


namespace quince {

std::unique_ptr<const abstract_expressionist>
make_postfix_expressionist(std::unique_ptr<const abstract_mapper_base> operand, const std::string &op);

}

#endif
