//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/util.h>
#include <quince/exprn_mappers/cast.h>
#include <quince/query.h>

using std::unique_ptr;


namespace quince {

typedef column_type (database::*column_type_finder)() const;


unique_ptr<const abstract_expressionist>
make_cast_expressionist(
    unique_ptr<const abstract_mapper_base> arg,
    column_type_finder ctf
) {
    struct expressionist : public abstract_expressionist {
        const abstract_mapper_base &_arg;
        column_type_finder _ctf;

        expressionist(unique_ptr<const abstract_mapper_base> &arg, column_type_finder ctf) :
            _arg(own(arg)),
            _ctf(ctf)
        {}

        virtual void
        write_expression(sql &cmd) const override {
            cmd.write_cast(_arg.only_column(), (cmd.get_database().*_ctf)());
        }

        virtual column_id_set
        imports() const override {
            return _arg.imports();
        }
    };

    return quince::make_unique<expressionist>(arg, ctf);
}

}