#ifndef QUINCE__mappers__numeric_cast_mapper_h
#define QUINCE__mappers__numeric_cast_mapper_h

/*
    Copyright 2014 Michael Shepanski

    This file is part of the quince library.

    Quince is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Quince is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with quince.  If not, see <http://www.gnu.org/licenses/>.
*/

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
        return std::make_unique<numeric_cast_mapper<NewCxxType, OldMapper>>(*this);
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
