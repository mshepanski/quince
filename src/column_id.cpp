//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <atomic>
#include <quince/detail/column_id.h>


namespace quince {

namespace {
    std::atomic<int_fast64_t> counter(0);
}

column_id
next_column_id() {
    return ++counter;
}

}