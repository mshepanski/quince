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