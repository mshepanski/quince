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
