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
#include <quince/exprn_mappers/choose.h>
#include <quince/query.h>

using boost::optional;
using std::make_unique;
using std::unique_ptr;
using std::vector;


namespace quince {

unique_ptr<const abstract_expressionist>
make_case_expressionist(
    unique_ptr<const abstract_mapper_base> switch_,
    vector<unique_case_clause> &&clauses,
    unique_ptr<const abstract_mapper_base> backstop
) {
    struct expressionist : public abstract_expressionist {
        typedef std::pair<const abstract_mapper_base *, const abstract_mapper_base *> clause;

        optional<const abstract_mapper_base &> _switch;
        vector<clause> _clauses;
        optional<const abstract_mapper_base &> _default;

        explicit expressionist(
            unique_ptr<const abstract_mapper_base> &switch_,
            vector<unique_case_clause> &clauses,
            unique_ptr<const abstract_mapper_base> &backstop
        ) {
            if (switch_)
                _switch = own(switch_);

            for (unique_case_clause &c: clauses)
                if (c._when)
                    _clauses.push_back({ &own(c._when), &own(c._then) });
                else
                    _default = own(c._then);

            if (!_default)
                _default = own(backstop);
        }

        virtual void
        write_expression(sql &cmd) const override {
            cmd.write_case(
                optional_only_column(_switch),
                transform(_clauses, &only_columns),
                optional_only_column(_default)
            );
        }

        virtual column_id_set
        imports() const override {
            column_id_set result;
            if (_switch)
                add_to_set(result, _switch->imports());
            for (const auto c: _clauses) {
                add_to_set(result, c.first->imports());
                add_to_set(result, c.second->imports());
            }
            if (_default)
                add_to_set(result, _default->imports());
            return result;
        }

    private:
        static std::pair<const column_mapper *, const column_mapper *>
        only_columns(const clause &c) {
            return { &c.first->only_column(), &c.second->only_column() };
        }

        static optional<const column_mapper &>
        optional_only_column(optional<const abstract_mapper_base &> m) {
            if (m)  return m->only_column();
            else    return boost::none;
        }
    };

    return make_unique<expressionist>(switch_, clauses, backstop);
}

}
