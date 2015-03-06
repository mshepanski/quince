//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/mappers/detail/column_mapper.h>

using boost::optional;
using std::string;


namespace quince {

column_mapper::column_mapper(const optional<string> &name) :
    abstract_mapper_base(name),
    _id(next_column_id()),
    _alias("r$" + std::to_string(_id))
{}

void
column_mapper::for_each_column(std::function<void(const column_mapper &)> op) const {
    op(*this);
}

}