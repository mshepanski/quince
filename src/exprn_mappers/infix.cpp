//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/sql.h>
#include <quince/detail/util.h>
#include <quince/exprn_mappers/detail/infix.h>
#include <quince/query.h>

using std::string;
using std::unique_ptr;


namespace quince {

unique_ptr<const abstract_expressionist>
make_infix_expressionist(
    const string &op,
    unique_ptr<const abstract_mapper_base> lhs,
    unique_ptr<const abstract_mapper_base> rhs
) {
    struct expressionist : public abstract_expressionist {
        const string _op;
        const abstract_mapper_base &_lhs;
        const abstract_mapper_base &_rhs;

        expressionist(
            const string &op,
            unique_ptr<const abstract_mapper_base> &lhs,
            unique_ptr<const abstract_mapper_base> &rhs
        ) :
#ifdef __clang__
            _op(op.c_str()),
#else
            _op(op),
#endif
            _lhs(own(lhs)),
            _rhs(own(rhs))
        {}

        virtual void write_expression(sql &cmd) const override  { cmd.write_infix_exprn(_lhs.only_column(), _op, _rhs.only_column()); }
        virtual column_id_set imports() const override          { return set_union(_lhs.imports(), _rhs.imports()); }
    };

    return quince::make_unique<expressionist>(op, lhs, rhs);
}

}
