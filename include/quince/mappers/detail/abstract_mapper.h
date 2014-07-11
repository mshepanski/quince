#ifndef QUINCE__mappers__abstract_mapper_h
#define QUINCE__mappers__abstract_mapper_h

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

#include <quince/detail/clone.h>
#include <quince/detail/column_id.h>
#include <quince/detail/object_owner.h>
#include <quince/mappers/detail/abstract_mapper_base.h>
#include <quince/mappers/detail/exposed_mapper_type.h>


namespace quince {

class row;
template<typename> class query;

// If CxxType is any mapped type, then abstract_mapper<CxxType> is the interface that
// its mapper class must implement, and any exprn_mapper<CxxType> must implement.
//
template<typename CxxType>
class abstract_mapper : public virtual abstract_mapper_base {

    // Everything in this class is for quince internal use only.

public:
    typedef CxxType value_type;
    CxxType value_declval() const;  // never defined, never called, only mentioned in declspec(...) contexts.

    abstract_mapper(const boost::optional<std::string> &name) :
        abstract_mapper_base(name)
    {}

    virtual ~abstract_mapper()  {}

    virtual void to_row(const CxxType &src, row &dest) const = 0;
    virtual void from_row(const row &src, CxxType &dest) const = 0;

    CxxType
    from_row(const row &src) const {
        CxxType dest;
        from_row(src, dest);
        return dest;
    }

    // If the result of evaluating this mapper is known without consulting the DBMS,
    // the a_priori_value() returns that known value; otherwise it returns boost::none.
    // E.g.:
    //          *exprn_mapper<int32_t>(6).a_priori_value() == 6
    //
    virtual const boost::optional<CxxType> &
    a_priori_value() const {
        return none;
    }

protected:
    // Defined in query_base.h to avoid a circular header dependency:
    //
    virtual void build_match_tester(const query_base &q, predicate &result) const override;

private:
    static const boost::optional<CxxType> none;
};

template<typename CxxType> const boost::optional<CxxType> abstract_mapper<CxxType>::none = boost::none;


typedef abstract_mapper<bool> abstract_predicate;


inline bool
a_priori_true(const abstract_predicate &pred) {
    return pred.a_priori_value().get_value_or(false);
}

inline bool
a_priori_false(const abstract_predicate &pred) {
    const bool could_be_true = pred.a_priori_value().get_value_or(true);
    return ! could_be_true;
};

}

#endif
