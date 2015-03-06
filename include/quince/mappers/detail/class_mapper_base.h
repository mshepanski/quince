#ifndef QUINCE__mappers__detail__class_mapper_base_h
#define QUINCE__mappers__detail__class_mapper_base_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/optional.hpp>
#include <quince/detail/compiler_specific.h>
#include <quince/detail/mapper_factory.h>
#include <quince/mappers/detail/abstract_mapper.h>
#include <quince/mappers/detail/exposed_mapper_type.h>
#include <quince/exceptions.h>


/*
    This file defines typed_class_mapper_base<T>, which is the base of all the class mappers
    (which you get by invoking QUINCE_MAP_CLASS and QUINCE_MAP_CLASS_WITH_BASES),
    and class_mapper_base, which the base of all those typed_class_mapper_base<T>s.

    Between them these classes do most of the work of the class mappers, but it is all
    for quince internal use only.
*/

namespace quince {

class class_mapper_base : public virtual abstract_mapper_base {

    // Everything in this class is for quince internal use only.

public:
    explicit class_mapper_base(const boost::optional<std::string> &name);

    virtual ~class_mapper_base()  {}

    virtual void allow_all_null() const override;

    virtual column_id_set imports() const override;

    virtual void for_each_column(std::function<void(const column_mapper &)>) const override;    
    virtual void for_each_persistent_column(std::function<void(const persistent_column_mapper &)>) const override;

protected:
    void adopt_base(const class_mapper_base &base);

    template<typename ChildCxxType>
    const exposed_mapper_type<ChildCxxType> &
    add(
        const std::string &child_name,
        const mapper_factory &creator
    ) {
        return adopt_member(creator.create<ChildCxxType>(full_child_name(child_name)));
    }

    template<typename ChildMapper>
    const ChildMapper &
    adopt_member(const ChildMapper &child_mapper) {
        return adopt_member(clone(child_mapper));
    }

private:
    template<typename ChildMapper>
    const ChildMapper &
    adopt_member(std::unique_ptr<ChildMapper> child_mapper) {
        const ChildMapper &owned = own(child_mapper);
        adopt_untyped(owned);
        return owned;
    }

    std::vector<const class_mapper_base *> _bases;
    std::vector<const abstract_mapper_base *> _children;

    void adopt_untyped(const abstract_mapper_base &child);

    std::string full_child_name(const std::string &given_name) const;
};


QUINCE_SUPPRESS_MSVC_DOMINANCE_WARNING

template<typename CxxClassType>
class typed_class_mapper_base :
    public abstract_mapper<CxxClassType>,
    public class_mapper_base
{

    // Everything in this class is for quince internal use only.

public:
    virtual ~typed_class_mapper_base()  {}

    virtual void from_row(const row &src, CxxClassType &dest) const override {
        for (const auto mc : _member_correspondences)
            mc->from_row(src, dest);
    }

    virtual void to_row(const CxxClassType &src, row &dest) const override {
        for (const auto mc : _member_correspondences)
            mc->to_row(src, dest);
    }

    template<typename CxxMemberType>
    const abstract_mapper<CxxMemberType> &
    lookup(CxxMemberType CxxClassType::*ptr_to_member) const {
        for (const auto mcb : _member_correspondences)
            if (const auto mc = dynamic_cast<const member_correspondence<CxxMemberType>*>(mcb))
                if (mc->_ptr_to_member == ptr_to_member)
                    return mc->_child_mapper;
        throw unmapped_member_exception();
    }

protected:
    typed_class_mapper_base(const boost::optional<std::string> &name) :
        abstract_mapper_base(name),
        abstract_mapper<CxxClassType>(name),
        class_mapper_base(name)
    {}

    virtual std::unique_ptr<cloneable>
    clone_impl() const override {
        return std::make_unique<typed_class_mapper_base<CxxClassType>>(*this);
    }

    template<typename CxxMemberType>
    const exposed_mapper_type<CxxMemberType> &
    add(
        CxxMemberType CxxClassType::*ptr_to_member,
        const std::string &member_name,
        const mapper_factory &creator
    ) {
        auto &result = class_mapper_base::add<CxxMemberType>(member_name, creator);
        _member_correspondences.push_back(
            &own(std::make_unique<member_correspondence<CxxMemberType>>(ptr_to_member, result))
        );
        return result;
    }

    template<typename ChildMapper>
    const ChildMapper &
    adopt_member(
        typename ChildMapper::value_type (CxxClassType::*ptr_to_member),
        const ChildMapper &mapper
    ) {
        typedef typename ChildMapper::value_type cxx_member_type;

        const ChildMapper &result = class_mapper_base::adopt_member(mapper);
        _member_correspondences.push_back(
            &own(std::make_unique<member_correspondence<cxx_member_type>>(ptr_to_member, result))
        );
        return result;
    }

private:
    struct member_correspondence_base {
        virtual ~member_correspondence_base()  {}

        virtual void from_row(const row &src, CxxClassType &dest) const = 0;
        virtual void to_row(const CxxClassType &src, row &dest) const = 0;
    };

    template<typename CxxMemberType>
    struct member_correspondence : public member_correspondence_base {
        typedef CxxMemberType CxxClassType::*ptr_to_member;

        member_correspondence(ptr_to_member ptm, const abstract_mapper<CxxMemberType> &child_mapper) :
            _ptr_to_member(ptm),
            _child_mapper(child_mapper)
        {}

        virtual void from_row(const row &src, CxxClassType &dest) const override {
            _child_mapper.from_row(src, dest.*_ptr_to_member);
        }

        virtual void to_row(const CxxClassType &src, row &dest) const override {
            _child_mapper.to_row(src.*_ptr_to_member, dest);
        }

        const ptr_to_member _ptr_to_member;
        const abstract_mapper<CxxMemberType> &_child_mapper;
    };

    std::vector<const member_correspondence_base *> _member_correspondences;
};

QUINCE_UNSUPPRESS_MSVC_WARNING

}

#endif
