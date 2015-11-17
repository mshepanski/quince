//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/util.h>
#include <quince/exprn_mappers/detail/lexicographic_comparison.h>
#include <quince/query.h>

using std::unique_ptr;


namespace quince {

unique_ptr<const abstract_expressionist>
make_intrinsic_comparison_expressionist(
    relation r,
    unique_ptr<const abstract_mapper_base> lhs,
    unique_ptr<const abstract_mapper_base> rhs
) {
    struct expressionist : public abstract_expressionist {
        const relation _relation;
        const abstract_mapper_base &_lhs;
        const abstract_mapper_base &_rhs;

        expressionist(
            relation r,
            unique_ptr<const abstract_mapper_base> &lhs,
            unique_ptr<const abstract_mapper_base> &rhs
        ) :
            _relation(r),
            _lhs(own(lhs)),
            _rhs(own(rhs))
        {}

        virtual void write_expression(sql &cmd) const override  { cmd.write_intrinsic_comparison(_relation, _lhs, _rhs); }
        virtual column_id_set imports() const override          { return set_union(_lhs.imports(), _rhs.imports()); }
    };

    return quince::make_unique<expressionist>(r, lhs, rhs);
}

}
