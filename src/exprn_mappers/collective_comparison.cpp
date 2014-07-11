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

#include <quince/detail/compiler_specific.h>
#include <quince/detail/sql.h>
#include <quince/exprn_mappers/detail/collective_comparison.h>
#include <quince/query.h>

using std::make_unique;
using std::unique_ptr;


namespace quince {

unique_ptr<const abstract_expressionist>
make_collective_comparison_expressionist(
    relation r,
    unique_ptr<const abstract_mapper_base> lhs,
    unique_ptr<const collective_base> rhs
) {
    struct expressionist : public abstract_expressionist {
        const relation _relation;
        const abstract_mapper_base &_lhs;
        const collective_base &_rhs;

        expressionist(
            relation r,
            unique_ptr<const abstract_mapper_base> &lhs,
            unique_ptr<const collective_base> &rhs
        ) :
            _relation(r),
            _lhs(own(lhs)),
            _rhs(own(rhs))
        {}

        virtual void write_expression(sql &cmd) const override  { cmd.write_collective_comparison(_relation, _lhs, _rhs); }
        virtual column_id_set imports() const override          { return set_union(_lhs.imports(), _rhs.get_query().imports()); }
    };

    return make_unique<expressionist>(r, lhs, rhs);
}

}
