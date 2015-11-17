//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <assert.h>
#include <quince/exprn_mappers/detail/exprn_mapper.h>
#include <quince/detail/sql.h>
#include <quince/detail/util.h>
#include <quince/query.h>

using boost::optional;
using std::unique_ptr;
using std::string;
using std::vector;


namespace quince {

optional<std::pair<const abstract_mapper_base *, bool>>
abstract_expressionist::dissect_as_order_specification() const {
    return boost::none;
}

vector<const abstract_mapper_base *>
abstract_expressionist::own_all(vector<unique_ptr<const abstract_mapper_base>> &&v) {
    return transform(
        std::move(v),
        [this](unique_ptr<const abstract_mapper_base> &m)  { return &own(m); }
    );
}


exprn_mapper_base::~exprn_mapper_base()
{}

void
exprn_mapper_base::write_expression(sql &cmd) const {
    assert(cmd.alias_is_wanted(id()));  // if this fails then I haven't thought through the optimization properly
    _expressionist->write_expression(cmd);
}

column_id_set
exprn_mapper_base::imports() const {
    return set_union<column_id>(_expressionist->imports(), { id() });
}

exprn_mapper_base::exprn_mapper_base(unique_ptr<const abstract_expressionist> e) :
    abstract_mapper_base(boost::none),
    column_mapper(boost::none),
    _expressionist(&own(e))
{}

const abstract_expressionist &
exprn_mapper_base::get_expressionist() const {
    return *_expressionist;
}

std::unique_ptr<const abstract_expressionist>
exprn_mapper_base::make_delegating_expressionist(const abstract_mapper_base &mapper) {
    return make_delegating_expressionist(clone(mapper));
}

unique_ptr<const abstract_expressionist>
exprn_mapper_base::make_delegating_expressionist(
    unique_ptr<const abstract_mapper_base> delegate
) {
    struct expressionist : public abstract_expressionist {
        const abstract_mapper_base &_delegate;

        expressionist(unique_ptr<const abstract_mapper_base> &delegate) :
            _delegate(own(delegate))
        {}

        virtual void write_expression(sql &cmd) const override  { cmd.write_evaluation(_delegate.only_column()); }
        virtual column_id_set imports() const override          { return _delegate.imports(); }
    };

    return quince::make_unique<expressionist>(delegate);
}

std::pair<const abstract_mapper_base *, bool>
exprn_mapper_base::dissect_as_order_specification() const {
    if (const optional<std::pair<const abstract_mapper_base *, bool>> p = _expressionist->dissect_as_order_specification())
        return *p;
    else
        return abstract_mapper_base::dissect_as_order_specification();
}

}