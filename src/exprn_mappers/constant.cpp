//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/sql.h>
#include <quince/detail/util.h>
#include <quince/exprn_mappers/constant.h>
#include <quince/exprn_mappers/detail/exprn_mapper.h>
#include <quince/query.h>

using std::string;
using std::unique_ptr;


namespace quince {

unique_ptr<const abstract_expressionist>
make_constant_expressionist(
    const string &sql_constant_exprn
) {
    struct expressionist : public abstract_expressionist {
        const string _sql_constant_exprn;

        explicit expressionist(const string &sql_constant_exprn) :
#ifdef __clang__
            _sql_constant_exprn(sql_constant_exprn.c_str())
#else
            _sql_constant_exprn(sql_constant_exprn)
#endif
        {}

        virtual void write_expression(sql &cmd) const override  { cmd.write(_sql_constant_exprn); }

        virtual column_id_set
        imports() const override {
            return _sql_constant_exprn == "*"
                ? column_id_set{ wildcard_column_id }
                : column_id_set();
        }
    };

    return quince::make_unique<expressionist>(sql_constant_exprn);
}

}