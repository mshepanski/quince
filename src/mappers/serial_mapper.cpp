//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <memory>
#include <quince/mappers/detail/abstract_mapper.h>
#include <quince/detail/util.h>
#include <quince/mappers/serial_mapper.h>
#include <quince/exprn_mappers/detail/exprn_mapper.h>

using boost::optional;
using std::string;
using std::unique_ptr;


namespace quince {

serial_mapper::serial_mapper(const optional<string> &name, const mapper_factory &creator) :
    abstract_mapper_base(name),
    abstract_mapper<serial>(name),
    direct_mapper<int64_t>(name, creator)
{}

unique_ptr<cloneable>
serial_mapper::clone_impl() const {
    return quince::make_unique<serial_mapper>(*this);
}

void
serial_mapper::from_row(const row &src, serial &dest) const {
    int64_t int64;
    if (src.get<int64_t>(alias(), int64))
        dest.assign(int64);
    else
        dest.clear();
}

void
serial_mapper::to_row(const serial &src, row &dest) const {
    if (src.has_value())
        dest.add(name(), src.value());
    else
        dest.add(name(), boost::none);
}


column_type
serial_mapper::get_column_type(bool is_generated) const {
    if (is_generated)  return column_type::big_serial;
    
    return direct_mapper<int64_t>::get_column_type(false);
}

void
serial_mapper::for_each_column(std::function<void(const column_mapper &)> op) const {
    return direct_mapper<int64_t>::for_each_column(op);
}

void
serial_mapper::build_match_tester(const query_base &qb, predicate &result) const {
    abstract_mapper<serial>::build_match_tester(qb, result);
}

}
