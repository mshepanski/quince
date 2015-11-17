//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/sql.h>
#include <quince/detail/util.h>
#include <quince/exprn_mappers/detail/prefix.h>
#include <quince/query.h>

using boost::optional;
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

    return quince::make_unique<expressionist>(op, arg);
}

}
