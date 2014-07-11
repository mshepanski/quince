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
#include <quince/exprn_mappers/function.h>
#include <quince/query.h>

using std::make_unique;
using std::string;
using std::unique_ptr;
using std::vector;


namespace quince {

unique_ptr<const abstract_expressionist>
make_function_call_expressionist(
    const string &function_name,
    vector<unique_ptr<const abstract_mapper_base>> &&args)
{
    struct expressionist : public abstract_expressionist {
        const string _function_name;
        const vector<const abstract_mapper_base *> _args;

        expressionist(const string &function_name, vector<unique_ptr<const abstract_mapper_base>> &&args) :
#ifdef __clang__
            _function_name(function_name.c_str()),
#else
            _function_name(function_name),
#endif
            _args(own_all(std::move(args)))
        {}

        virtual void
        write_expression(sql &cmd) const override {
            cmd.write_function_call(
                _function_name,
                transform(_args, [](const abstract_mapper_base *m)  { return &m->only_column();} )
            );
        }

        virtual column_id_set
        imports() const override {
            column_id_set result;
            for (const auto a: _args)  add_to_set(result, a->imports());
            return result;
        }
    };

    return make_unique<expressionist>(function_name, std::move(args));
}

}
