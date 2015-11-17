#ifndef QUINCE__mappers__direct_mapper_h
#define QUINCE__mappers__direct_mapper_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/compiler_specific.h>
#include <quince/detail/row.h>
#include <quince/detail/util.h>
#include <quince/mappers/detail/abstract_mapper.h>
#include <quince/mappers/detail/persistent_column_mapper.h>


namespace quince {

class mapper_factory;

QUINCE_SUPPRESS_MSVC_DOMINANCE_WARNING


//  direct_mapper<CxxType> is a concrete mapper class that represents CxxType as a single SQL column type,
//  according to the backend library's basic conversions.
//
//  direct_mapper<CxxType> does not actually invoke the backend library's basic conversions.
//  It copies a C++ value to or from a cell within a row object, and it's up to the backend library
//  to convert between the row and the DBMS's "on-the-wire" formats.
//
template<typename CxxType>
class direct_mapper : public abstract_mapper<CxxType>, public persistent_column_mapper {

    // Everything in this class is for quince internal use only.

public:
    using abstract_mapper_base::name;

    explicit direct_mapper(const boost::optional<std::string> &name, const mapper_factory &) :
        abstract_mapper_base(name),
        abstract_mapper<CxxType>(name),
        persistent_column_mapper(name)
    {}

    virtual std::unique_ptr<cloneable>
    clone_impl() const override {
        return quince::make_unique<direct_mapper<CxxType>>(*this);
    }

    virtual void
    from_row(const row &src, CxxType &dest) const override {
        check_compatibility(src.get_database());
        if (! src.get<CxxType>(alias(), dest))
            throw missing_column_exception(alias());
    }

    virtual void
    to_row(const CxxType &src, row &dest) const override {
        check_compatibility(dest.get_database());
        dest.add(name(), src);
    }

    virtual column_type
    get_column_type(bool is_generated) const override {
        return quince::get_column_type<CxxType>();
    }

    virtual void
    for_each_column(std::function<void(const column_mapper &)> op) const override {
        return op(*this);
    }
};

QUINCE_UNSUPPRESS_MSVC_WARNING

}

#endif
