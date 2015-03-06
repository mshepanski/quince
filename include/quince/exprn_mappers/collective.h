#ifndef QUINCE__exprn_mappers__collective_h
#define QUINCE__exprn_mappers__collective_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/clone.h>
#include <quince/detail/compiler_specific.h>
#include <quince/detail/object_owner.h>
#include <quince/mappers/detail/exposed_mapper_type.h>


namespace quince {

class query_base;
template<typename> class abstract_query;
template<typename> class query;


enum class collective_type { all, some };

class collective_base : public cloneable, private object_owner {
public:
    const collective_type get_type() const;
    const query_base &get_query() const;

protected:
    collective_base(collective_type type, std::unique_ptr<const query_base> query);

    collective_type _type;
    const query_base *_query;
};


template<typename T>
class collective : public collective_base {
public:
    collective(collective_type t, const query<T> &q) :
        collective_base(t, clone(q))
    {}

    collective(collective_type t, const abstract_query<T> &q) :
        collective(t, q.wrapped())
    {}

    virtual std::unique_ptr<cloneable>
    clone_impl() const override {
        return std::make_unique<collective<T>>(*this);
    }
};

template<typename Q>
collective<typename Q::value_mapper::value_type>
all(const Q &q) {
    Q::value_mapper::static_forbid_optionals();
    return { collective_type::all, q };
}

template<typename Q>
collective<typename Q::value_mapper::value_type>
some(const Q &q) {
    Q::value_mapper::static_forbid_optionals();
    return { collective_type::some, q };
}

}

#endif
