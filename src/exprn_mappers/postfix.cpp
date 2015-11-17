//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/util.h>
#include <quince/detail/sql.h>
#include <quince/exprn_mappers/detail/postfix.h>
#include <quince/query.h>

using std::string;
using std::unique_ptr;


namespace quince {

unique_ptr<const abstract_expressionist>
make_postfix_expressionist(
    unique_ptr<const abstract_mapper_base> operand,
    const string &op
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

        virtual void write_expression(sql &cmd) const override  { cmd.write_postfix_exprn(_operand.only_column(), _op); }
        virtual column_id_set imports() const override          { return _operand.imports(); }
    };

    return quince::make_unique<expressionist>(op, operand);
}

}
