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
#include <quince/exprn_mappers/detail/prefix.h>
#include <quince/query.h>

using boost::optional;
using std::make_unique;
using std::string;
using std::unique_ptr;


namespace quince {

unique_ptr<const abstract_expressionist>
make_prefix_expressionist(
    const string &op,
    unique_ptr<const abstract_mapper_base> arg
) {
    struct expressionist : public abstract_expressionist {
        const string _op;
        const abstract_mapper_base &_operand;

        expressionist(const string &op, unique_ptr<const abstract_mapper_base> &operand) :
#ifdef __clang__
            _op(op.c_str()),
#else
            _op(op),
#endif
            _operand(own(operand))
        {}

        virtual void write_expression(sql &cmd) const override  { cmd.write_prefix_exprn(_op, _operand.only_column()); }
        virtual column_id_set imports() const override          { return _operand.imports(); }

        virtual optional<std::pair<const abstract_mapper_base *, bool>>
        dissect_as_order_specification() const override {
            if (_op == sql::unary_plus_operator)
                return std::make_pair(&_operand, false);

            else if (_op == sql::unary_minus_operator)
                return std::make_pair(&_operand, true);

            else
                return boost::none;
        }
    };

    return make_unique<expressionist>(op, arg);
}

}
