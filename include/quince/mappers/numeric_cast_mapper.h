#ifndef QUINCE__mappers__numeric_cast_mapper_h
#define QUINCE__mappers__numeric_cast_mapper_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/cast.hpp>
#include <quince/detail/compiler_specific.h>
#include <quince/exprn_mappers/detail/exprn_mapper.h>


namespace quince {

class mapper_factory;

QUINCE_SUPPRESS_MSVC_DOMINANCE_WARNING

template<typename NewCxxType, typename OldMapper>
class numeric_cast_mapper : public abstract_mapper<NewCxxType>, public OldMapper
{

    // Everything in this class is for quince internal use only.

private:
    typedef typename OldMapper::value_type old_cxx_type;

public:
    explicit numeric_cast_mapper(const boost::optional<std::string> &name, const mapper_factory &creator) :
        abstract_mapper_base(name),
        abstract_mapper<NewCxxType>(name),
        OldMapper(name, creator)
    {}

    virtual std::unique_ptr<cloneable>
    clone_impl() const override {
        return quince::make_unique<numeric_cast_mapper<NewCxxType, OldMapper>>(*this);
    }

    virtual void from_row(const row &src, NewCxxType &dest) const override {
        old_cxx_type old;
        OldMapper::from_row(src, old);
        dest = boost::numeric_cast<NewCxxType>(old);
    }

    virtual void to_row(const NewCxxType &src, row &dest) const override {
        OldMapper::to_row(boost::numeric_cast<old_cxx_type>(src), dest);
    }

protected:
    virtual void build_match_tester(const query_base &qb, predicate &result) const override {
        abstract_mapper<NewCxxType>::build_match_tester(qb, result);
    }
};

QUINCE_UNSUPPRESS_MSVC_WARNING

}

#endif
