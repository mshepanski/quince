//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/sql.h>
#include <quince/detail/query_base.h>
#include <quince/detail/util.h>
#include <quince/exprn_mappers/in.h>
#include <quince/query.h>

using std::unique_ptr;
using std::vector;


namespace quince {

unique_ptr<const abstract_expressionist>
make_in_expressionist(
    unique_ptr<const abstract_mapper_base> lhs,
    vector<unique_ptr<const abstract_mapper_base>> &&rhs
) {
    struct expressionist : public abstract_expressionist {
        const abstract_mapper_base &_lhs;
        const vector<const abstract_mapper_base *> _rhs;

        expressionist(
            unique_ptr<const abstract_mapper_base> &lhs,
            vector<unique_ptr<const abstract_mapper_base>> &&rhs
        ) :
            _lhs(own(lhs)),
            _rhs(own_all(std::move(rhs)))
        {}

        virtual void
        write_expression(sql &cmd) const override {
            cmd.write_in(
                _lhs.only_column(),
                transform(_rhs, [](const abstract_mapper_base * m)  {return &m->only_column();} )
            );
        }

        virtual column_id_set
        imports() const override {
            column_id_set result = _lhs.imports();
            for (const auto r: _rhs)  add_to_set(result, r->imports());
            return result;
        }
    };

    return quince::make_unique<expressionist>(lhs, std::move(rhs));
}


unique_ptr<const abstract_expressionist>
make_in_expressionist(
    unique_ptr<const abstract_mapper_base> lhs,
    unique_ptr<const query_base> rhs
) {
    struct expressionist : public abstract_expressionist {
        const abstract_mapper_base &_lhs;
        const query_base &_rhs;

        expressionist(
            unique_ptr<const abstract_mapper_base> &lhs,
            unique_ptr<const query_base> &rhs
        ) :
            _lhs(own(lhs)),
            _rhs(own(rhs))
        {}

        virtual void write_expression(sql &cmd) const override  { cmd.write_in(_lhs.only_column(), _rhs); }
        virtual column_id_set imports() const override          { return set_union(_lhs.imports(), _rhs.imports()); }
    };

    return quince::make_unique<expressionist>(lhs, rhs);
}

}
