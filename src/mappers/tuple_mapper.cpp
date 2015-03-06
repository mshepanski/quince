//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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
