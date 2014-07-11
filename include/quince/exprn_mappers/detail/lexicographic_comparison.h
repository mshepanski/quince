#ifndef QUINCE__exprn_mappers__detail__lexicographic_comparison_h
#define QUINCE__exprn_mappers__detail__lexicographic_comparison_h

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

#include <quince/detail/compiler_specific.h>
#include <quince/detail/sql.h>
#include <quince/exprn_mappers/detail/exprn_mapper.h>
#include <quince/exprn_mappers/detail/infix.h>


namespace quince {

enum class relation {
    equal,
    not_equal,
    greater,
    less,
    greater_or_equal,
    less_or_equal
};

template<typename T>
struct extrinsic_comparison_expressionist : public abstract_expressionist {
    const relation _relation;
    const abstract_mapper<T> &_lhs;
    const T _rhs;

    extrinsic_comparison_expressionist(relation r, std::unique_ptr<abstract_mapper<T>> &lhs, const T &rhs) :
        _relation(r),
        _lhs(own(lhs)),
        _rhs(rhs)
    {}

    virtual void
    write_expression(sql &cmd) const override {
        row r(&cmd.get_database());
        _lhs.to_row(_rhs, r);
        cmd.write_extrinsic_comparison(_relation, _lhs, r);
    }

    virtual column_id_set
    imports() const override {
        return _lhs.imports();
    }
};

template<typename T>
std::unique_ptr<const abstract_expressionist>
make_extrinsic_comparison_expressionist(
    relation r,
    std::unique_ptr<abstract_mapper<T>> lhs,
    const T &rhs
) {
    return std::make_unique<extrinsic_comparison_expressionist<T>>(r, lhs, rhs);
}

std::unique_ptr<const abstract_expressionist>
make_intrinsic_comparison_expressionist(
    relation r,
    std::unique_ptr<const abstract_mapper_base> lhs,
    std::unique_ptr<const abstract_mapper_base> rhs
);


struct lexicographic_comparison_op {
    lexicographic_comparison_op(relation r, relation converse_r) :
        _r(r),
        _converse_r(converse_r),
        _infix_op({sql::relop(r)})
    {}

    template<typename T>
    predicate
    operator()(const abstract_mapper<T> &lhs, const abstract_mapper<T> &rhs) const {
        exposed_mapper_type<T>::static_forbid_optionals();
        return predicate(make_intrinsic_comparison_expressionist(_r, clone(lhs), clone(rhs)));
    }

    template<typename T>
    predicate
    operator()(const abstract_mapper<T> &lhs, const T &rhs) const {
        exposed_mapper_type<T>::static_forbid_optionals();
        return predicate(make_extrinsic_comparison_expressionist(_r, clone(lhs), rhs));
    }

    template<typename T>
    predicate
    operator()(const T &lhs, const abstract_mapper<T> &rhs) const {
        exposed_mapper_type<T>::static_forbid_optionals();
        return predicate(make_extrinsic_comparison_expressionist(_converse_r, clone(rhs), lhs));
    }

    template<typename T>
    predicate
    operator()(const T &lhs, const exprn_mapper<T> &rhs) const {
        exposed_mapper_type<T>::static_forbid_optionals();
        return _infix_op(lhs, rhs);
    }

    template<typename T>
    predicate
    operator()(const exprn_mapper<T> &lhs, const T &rhs) const {
        exposed_mapper_type<T>::static_forbid_optionals();
        return _infix_op(lhs, rhs);
    }

private:
    const relation _r;
    const relation _converse_r;
    const symmetric_infix_op<bool> _infix_op;
};

}

#endif
