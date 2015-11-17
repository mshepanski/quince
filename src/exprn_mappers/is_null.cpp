//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/sql.h>
#include <quince/detail/util.h>
#include <quince/exprn_mappers/detail/is_null.h>
#include <quince/query.h>

using std::unique_ptr;


namespace quince {

unique_ptr<const abstract_expressionist>
make_is_null_expressionist(
    unique_ptr<const abstract_mapper_base> arg
) {
    struct expressionist : public abstract_expressionist {
        const abstract_mapper_base &_arg;

        explicit expressionist(unique_ptr<const abstract_mapper_base> &arg) :
            _arg(own(arg))
        {}

        virtual void write_expression(sql &cmd) const override  { cmd.write_are_all_null(_arg); }
        virtual column_id_set imports() const override          { return _arg.imports(); }
    };

    return quince::make_unique<expressionist>(arg);
}

predicate
is_null(const abstract_mapper_base &arg) {
    return predicate(make_is_null_expressionist(clone(arg)));
}

}
