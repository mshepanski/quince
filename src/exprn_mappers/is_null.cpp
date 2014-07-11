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
#include <quince/exprn_mappers/detail/is_null.h>
#include <quince/query.h>

using std::make_unique;
using std::unique_ptr;


namespace quince {

unique_ptr<const abstract_expressionist>
make_is_null_expressionist(
    unique_ptr<const abstract_mapper_base> arg
) {
    struct expressionist : public abstract_expressionist {
        const abstract_mapper_base &_arg;

        explicit expressionist(unique_ptr<const abstract_mapper_base> &arg) :
            _arg(own(arg))
        {}

        virtual void write_expression(sql &cmd) const override  { cmd.write_are_all_null(_arg); }
        virtual column_id_set imports() const override          { return _arg.imports(); }
    };

    return make_unique<expressionist>(arg);
}

predicate
is_null(const abstract_mapper_base &arg) {
    return predicate(make_is_null_expressionist(clone(arg)));
}

}
