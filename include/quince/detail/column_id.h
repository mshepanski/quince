#ifndef QUINCE__mappers__detail__column_id_h
#define QUINCE__mappers__detail__column_id_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <set>
#include <stdint.h>
#include <string>


/*
    Everything in this file is for quince internal use only.
*/

namespace quince {

typedef int64_t column_id;
typedef std::set<column_id> column_id_set;

column_id next_column_id();
static const column_id wildcard_column_id = static_cast<column_id>(-1);

}

#endif
