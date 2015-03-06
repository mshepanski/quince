//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <assert.h>
#include <quince/detail/table_interface.h>
#include <quince/mappers/detail/persistent_column_mapper.h>


namespace quince {

void
table_interface::initialize_mapper(abstract_mapper_base &mapper) const {
    size_t n = 0;
    bool have_defaulted_column_name = false;
    mapper.for_each_persistent_column([&](const persistent_column_mapper &pcm) {
        // const_cast is OK because mapper, and hence pcm, has only just
        // been constructed; i.e. this is a sort of two-phase initialization.
        //
        persistent_column_mapper &target = const_cast<persistent_column_mapper&>(pcm);

        if (! target.has_name()) {
            target.set_name("value");
            have_defaulted_column_name = true;
        }
        target.set_table(this);
        n++;
    });
    if (have_defaulted_column_name)  assert(n == 1);
}

}
