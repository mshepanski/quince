//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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
