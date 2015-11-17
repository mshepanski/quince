//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/util.h>
#include <quince/exprn_mappers/choose.h>
#include <quince/query.h>

using boost::optional;
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

    return quince::make_unique<expressionist>(switch_, clauses, backstop);
}

}
