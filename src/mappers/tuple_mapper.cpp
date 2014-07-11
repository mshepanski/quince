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

#include <quince/mappers/tuple_mapper.h>

using boost::optional;
using std::string;


namespace quince {

tuple_mapper_base::tuple_mapper_base(const optional<string> &name) :
    abstract_mapper_base(name),
    _created_element_count(0)
{
    abstract_mapper_base::allow_all_null();
}

string
tuple_mapper_base::next_created_element_name() {
    const string result = name() + "<" + std::to_string(_created_element_count++) + ">";
    return result;
}

}
