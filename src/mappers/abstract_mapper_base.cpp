//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <assert.h>
#include <quince/mappers/detail/abstract_mapper_base.h>
#include <quince/query.h>

using std::string;


namespace quince {

abstract_mapper_base::abstract_mapper_base(const boost::optional<std::string> &name) :
    _name(name),
    _can_be_all_null(false)
{
    if (_name)  assert(! _name->empty());
}

bool
abstract_mapper_base::has_name() const {
    return bool(_name);
}

string
abstract_mapper_base::name() const {
    assert(_name);
    return *_name;
}

void
abstract_mapper_base::set_name(const string &name) {
    _name = name;
}

bool
abstract_mapper_base::can_be_all_null() const {
    return _can_be_all_null;
}

void
abstract_mapper_base::allow_all_null() const {
    _can_be_all_null = true;
}

std::pair<const abstract_mapper_base *, bool>
abstract_mapper_base::dissect_as_order_specification() const {
    return { this, false };
}

void
abstract_mapper_base::forbid_all_null() const {
    _can_be_all_null = false;
}

}