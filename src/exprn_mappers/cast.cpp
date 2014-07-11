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
#include <quince/exprn_mappers/cast.h>
#include <quince/query.h>

using std::make_unique;
using std::unique_ptr;


namespace quince {

typedef column_type (database::*column_type_finder)() const;


unique_ptr<const abstract_expressionist>
make_cast_expressionist(
    unique_ptr<const abstract_mapper_base> arg,
    column_type_finder ctf
) {
    struct expressionist : public abstract_expressionist {
        const abstract_mapper_base &_arg;
        column_type_finder _ctf;

        expressionist(unique_ptr<const abstract_mapper_base> &arg, column_type_finder ctf) :
            _arg(own(arg)),
            _ctf(ctf)
        {}

        virtual void
        write_expression(sql &cmd) const override {
            cmd.write_cast(_arg.only_column(), (cmd.get_database().*_ctf)());
        }

        virtual column_id_set
        imports() const override {
            return _arg.imports();
        }
    };

    return make_unique<expressionist>(arg, ctf);
}

}