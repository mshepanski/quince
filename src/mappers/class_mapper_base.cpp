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
#include <quince/mappers/detail/class_mapper_base.h>

using boost::optional;
using std::string;
using std::unique_ptr;
using std::vector;


namespace quince {

class_mapper_base::class_mapper_base(const optional<string> &name) :
    abstract_mapper_base(name)
{
    abstract_mapper_base::allow_all_null();
}

void
class_mapper_base::allow_all_null() const {
    abstract_mapper_base::allow_all_null();
    for (const auto b: _bases)  b->class_mapper_base::allow_all_null();
    for (const auto c: _children)  c->allow_all_null();
}

column_id_set
class_mapper_base::imports() const {
    return column_ids();
}

void
class_mapper_base::for_each_column(std::function<void(const column_mapper &)> op) const {
    for (const auto b: _bases)  b->class_mapper_base::for_each_column(op);
    for (const auto c: _children)  c->for_each_column(op);
}

void
class_mapper_base::for_each_persistent_column(std::function<void(const persistent_column_mapper &)> op) const {
    for (const auto b: _bases)  b->class_mapper_base::for_each_persistent_column(op);
    for (const auto c: _children)  c->for_each_persistent_column(op);
}

void
class_mapper_base::adopt_base(const class_mapper_base &base) {
    assert(dynamic_cast<const void *>(&base) == dynamic_cast<const void *>(this));
    _bases.push_back(&base);
    if (! base.can_be_all_null())  forbid_all_null();
}

void
class_mapper_base::adopt_untyped(const abstract_mapper_base &child) {
    _children.push_back(&child);
    if (! child.can_be_all_null())  forbid_all_null();
}

string
class_mapper_base::full_child_name(const string &given_name) const {
    return has_name()
        ? name() + "." + given_name
        : given_name;
}

}
