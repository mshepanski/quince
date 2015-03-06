#ifndef QUINCE__mappers__optional_mapper_h
#define QUINCE__mappers__optional_mapper_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <boost/optional.hpp>
#include <assert.h>
#include <boost/optional.hpp>
#include <quince/detail/column_type.h>
#include <quince/detail/compiler_specific.h>
#include <quince/detail/mapper_factory.h>
#include <quince/mappers/detail/abstract_mapper.h>
#include <quince/mappers/detail/persistent_column_mapper.h>
#include <quince/exprn_mappers/detail/coalesce.h>
#include <quince/exprn_mappers/detail/is_null.h>
#include <quince/exprn_mappers/choose.h>
#include <quince/exprn_mappers/functions.h>


namespace quince {

// optional_mapper<Content> is the mapper class for boost::optional<Content>
//
template<typename Content>
class optional_mapper : public virtual abstract_mapper<boost::optional<Content>> {
public:
    typedef boost::optional<Content> value_type;

    // These public functions are described at http://quince-lib.com/mapped_data_types/boost_optional.html

    virtual const exposed_mapper_type<Content> *
    operator->() const {
        return &_content;
    }

    virtual const exposed_mapper_type<Content> &
    operator*() const {
        return _content;
    }

    exprn_mapper<Content>
    get_value_or(const exprn_mapper<Content> &fallback) const {
        if (is_optimized() && _content.size() == 1)
            return coalesce(_content, fallback);
        else
            return choose(
                when(is_initialized(), _content),
                fallback
            );
    }

    exprn_mapper<Content>
    get_value_or(const Content &fallback) const {
        return get_value_or(exprn_mapper<Content>(fallback));
    }

    predicate
    is_initialized() const {
        if (is_optimized())
            return ! is_null(_content);
        else
            return *_flag;
    }


    // --- Everything from here to end of class is for quince internal use only. ---

    // Constructor that is used whenever *this is to be some table's value mapper, or part of some table's
    // value mapper:
    //
    optional_mapper(const boost::optional<std::string> &name, const mapper_factory &creator) :
        abstract_mapper_base(boost::none),
        abstract_mapper<value_type>(name),
        _content(this->own(creator.create<Content>(name))),
        _flag(
            _content.can_be_all_null()
                ? &this->own(creator.create<bool>(name.get_value_or("") + ".$opt"))
                : nullptr
        )
    {
        if (is_optimized())  allow_all_null();
    }

    // Constructor that is used whenever *this is to be a collector class for select() or a function in the join() family:
    //
    explicit optional_mapper(const exposed_mapper_type<Content> &content_mapper) :
        abstract_mapper_base(boost::none),
        abstract_mapper<value_type>(boost::none),
        _content(this->own(clone(content_mapper))),
        _flag(nullptr)
    {
        static_assert(! is_optional<Content>::value, "this constructor's arg must be a mapper of a non-optional type");
        assert(! content_mapper.can_be_all_null());
    }

    virtual std::unique_ptr<cloneable>
    clone_impl() const override {
        return std::make_unique<optional_mapper<Content>>(*this);
    }

    bool
    is_optimized() const {
        return _flag == nullptr;
    }

    virtual void
    from_row(const row &src, value_type &dest) const override {
        bool got_value;

        if (is_optimized()) {
            got_value = false;
            _content.for_each_column([&](const column_mapper &col) {
                const cell &c = *src.find_cell(col.alias());
                if (c.has_value() && c.type() != column_type::none)
                    got_value = true;
            });
        }
        else
            got_value = _flag->from_row(src);

        if (got_value)
            dest = _content.abstract_mapper<Content>::from_row(src);
        else
            dest = boost::none;
    }

    virtual void
    to_row(const value_type &src, row &dest) const override {
        const bool got_value = bool(src);

        if (! is_optimized())
            _flag->to_row(got_value, dest);

        if (got_value)
            _content.to_row(*src, dest);
        else
            _content.for_each_persistent_column([&](const persistent_column_mapper &p) {
                dest.add(p.name(), cell(boost::none));
            });
    }

    virtual column_id_set
    imports() const override {
        column_id_set result = _content.imports();
        if (! is_optimized())  add_to_set(result, _flag->imports());
        return result;
    }

    virtual void
    for_each_column(std::function<void(const column_mapper &)> op) const override {
        if (! is_optimized())  _flag->for_each_column(op);
        _content.for_each_column(op);
    }

    virtual void
    for_each_persistent_column(std::function<void(const persistent_column_mapper &)> op) const override {
        if (! is_optimized())  _flag->for_each_persistent_column(op);
        _content.for_each_persistent_column(op);
    }

    virtual void
    allow_all_null() const override {
        abstract_mapper_base::allow_all_null();
        if (is_optimized())
            _content.allow_all_null();
        else {
            _flag->allow_all_null();
            assert(_content.can_be_all_null());
        }
    }

    template<bool DelayInstantiation = false>
    static void
    static_forbid_optionals() {
        static_assert(DelayInstantiation, "");
    }

private:
    const exposed_mapper_type<Content> &_content;
    const abstract_mapper<bool> *_flag;
};

}

#endif
