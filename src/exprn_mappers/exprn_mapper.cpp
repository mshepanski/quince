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
#include <quince/exprn_mappers/detail/exprn_mapper.h>
#include <quince/detail/compiler_specific.h>
#include <quince/detail/sql.h>
#include <quince/query.h>

using boost::optional;
using std::unique_ptr;
using std::make_unique;
using std::string;
using std::vector;


namespace quince {

optional<std::pair<const abstract_mapper_base *, bool>>
abstract_expressionist::dissect_as_order_specification() const {
    return boost::none;
}

vector<const abstract_mapper_base *>
abstract_expressionist::own_all(vector<unique_ptr<const abstract_mapper_base>> &&v) {
    return transform(
        std::move(v),
        [this](unique_ptr<const abstract_mapper_base> &m)  { return &own(m); }
    );
}


exprn_mapper_base::~exprn_mapper_base()
{}

void
exprn_mapper_base::write_expression(sql &cmd) const {
    assert(cmd.alias_is_wanted(id()));  // if this fails then I haven't thought through the optimization properly
    _expressionist->write_expression(cmd);
}

column_id_set
exprn_mapper_base::imports() const {
    return set_union<column_id>(_expressionist->imports(), { id() });
}

exprn_mapper_base::exprn_mapper_base(unique_ptr<const abstract_expressionist> e) :
    abstract_mapper_base(boost::none),
    column_mapper(boost::none),
    _expressionist(&own(e))
{}

const abstract_expressionist &
exprn_mapper_base::get_expressionist() const {
    return *_expressionist;
}

std::unique_ptr<const abstract_expressionist>
exprn_mapper_base::make_delegating_expressionist(const abstract_mapper_base &mapper) {
    return make_delegating_expressionist(clone(mapper));
}

unique_ptr<const abstract_expressionist>
exprn_mapper_base::make_delegating_expressionist(
    unique_ptr<const abstract_mapper_base> delegate
) {
    struct expressionist : public abstract_expressionist {
        const abstract_mapper_base &_delegate;

        expressionist(unique_ptr<const abstract_mapper_base> &delegate) :
            _delegate(own(delegate))
        {}

        virtual void write_expression(sql &cmd) const override  { cmd.write_evaluation(_delegate.only_column()); }
        virtual column_id_set imports() const override          { return _delegate.imports(); }
    };

    return make_unique<expressionist>(delegate);
}

std::pair<const abstract_mapper_base *, bool>
exprn_mapper_base::dissect_as_order_specification() const {
    if (const optional<std::pair<const abstract_mapper_base *, bool>> p = _expressionist->dissect_as_order_specification())
        return *p;
    else
        return abstract_mapper_base::dissect_as_order_specification();
}

}