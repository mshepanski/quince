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
#include <quince/exprn_mappers/constant.h>
#include <quince/exprn_mappers/detail/exprn_mapper.h>
#include <quince/query.h>

using std::make_unique;
using std::string;
using std::unique_ptr;


namespace quince {

unique_ptr<const abstract_expressionist>
make_constant_expressionist(
    const string &sql_constant_exprn
) {
    struct expressionist : public abstract_expressionist {
        const string _sql_constant_exprn;

        explicit expressionist(const string &sql_constant_exprn) :
#ifdef __clang__
            _sql_constant_exprn(sql_constant_exprn.c_str())
#else
            _sql_constant_exprn(sql_constant_exprn)
#endif
        {}

        virtual void write_expression(sql &cmd) const override  { cmd.write(_sql_constant_exprn); }

        virtual column_id_set
        imports() const override {
            return _sql_constant_exprn == "*"
                ? column_id_set{ wildcard_column_id }
                : column_id_set();
        }
    };

    return make_unique<expressionist>(sql_constant_exprn);
}

}