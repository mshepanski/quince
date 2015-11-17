//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/exprn_mappers/scalar.h>
#include <quince/detail/query_base.h>
#include <quince/detail/sql.h>
#include <quince/detail/util.h>
#include <quince/query.h>

using std::unique_ptr;


namespace quince {

unique_ptr<const abstract_expressionist>
make_scalar_subquery_expressionist(
    unique_ptr<const query_base> query
) {
    struct expressionist : public abstract_expressionist {
        const query_base &_query;

        expressionist(unique_ptr<const query_base> &query) :
            _query(own(query))
        {}

        virtual void write_expression(sql &cmd) const override  { cmd.write_scalar_subquery(_query); }
        virtual column_id_set imports() const override          { return _query.imports(); }
    };

    return quince::make_unique<expressionist>(query);
}

}