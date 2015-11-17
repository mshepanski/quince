#ifndef QUINCE__exprn_mappers__detail__exprn_mapper_h
#define QUINCE__exprn_mappers__detail__exprn_mapper_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <type_traits>
#include <vector>
#include <boost/optional.hpp>
#include <boost/utility/base_from_member.hpp>
#include <quince/detail/compiler_specific.h>
#include <quince/detail/sql.h>
#include <quince/mappers/detail/abstract_mapper.h>
#include <quince/mappers/detail/column_mapper.h>
#include <quince/database.h>
#include <quince/exceptions.h>


namespace quince {

template<typename> class exprn_mapper;
typedef exprn_mapper<bool> predicate;


// Fallback template, which can be overriden by any non-templated overloads
// defined via QUINCE_DEFINE_SERVER_ONLY_TYPE (below).
//
template<typename T>
void QUINCE_from_cell_via_adl(const database &db, const cell &src, T &dest) {
    db.from_cell(src, dest);
}

class abstract_expressionist : protected object_owner, private boost::noncopyable {
public:
    virtual ~abstract_expressionist()  {}

    virtual void write_expression(sql &) const = 0;
    virtual column_id_set imports() const = 0;

    virtual boost::optional<std::pair<const abstract_mapper_base *, bool>> dissect_as_order_specification() const;

protected:
    std::vector<const abstract_mapper_base *>
    own_all(std::vector<std::unique_ptr<const abstract_mapper_base>> &&);
};


class exprn_mapper_base : public column_mapper {
public:
    ~exprn_mapper_base();

    virtual void write_expression(sql &) const override;
    virtual column_id_set imports() const override;

protected:
    explicit exprn_mapper_base(std::unique_ptr<const abstract_expressionist>);

    const abstract_expressionist &get_expressionist() const;
    
    static std::unique_ptr<const abstract_expressionist>
    make_delegating_expressionist(const abstract_mapper_base &);

    virtual void allow_all_null() const override                { abort(); }
    virtual column_type get_column_type(bool) const override    { abort(); }
    virtual void for_each_persistent_column(std::function<void (const persistent_column_mapper &)>) const override
                                                                { abort(); }

private:
    static std::unique_ptr<const abstract_expressionist>
    make_delegating_expressionist(std::unique_ptr<const abstract_mapper_base> arg);

    virtual std::pair<const abstract_mapper_base *, bool>
    dissect_as_order_specification() const override;
    const abstract_expressionist *_expressionist;
};


QUINCE_SUPPRESS_MSVC_DOMINANCE_WARNING

template<typename Return>
class general_exprn_mapper : public virtual abstract_mapper<Return>, public exprn_mapper_base
{
public:
    general_exprn_mapper(const Return &value) :
        abstract_mapper_base(boost::none),
        abstract_mapper<Return>(boost::none),
        exprn_mapper_base(make_value_expressionist(value)),
        _a_priori_value(value)
    {}

    general_exprn_mapper(const abstract_mapper<Return> &delegate) :
        abstract_mapper_base(boost::none),
        abstract_mapper<Return>(boost::none),
        exprn_mapper_base(make_delegating_expressionist(delegate)),
        _a_priori_value(delegate.a_priori_value())
    {}

    explicit general_exprn_mapper(std::unique_ptr<const abstract_expressionist> &&expressionist) :
        abstract_mapper_base(boost::none),
        abstract_mapper<Return>(boost::none),
        exprn_mapper_base(std::move(expressionist))
    {}

    virtual void
    from_row(const row &src, Return &dest) const override {
        const database &database = src.get_database();
        const cell * const cell = src.find_cell(alias());
        if (cell == nullptr)  throw missing_column_exception(alias());

        QUINCE_from_cell_via_adl(database, *cell, dest);
    }

    virtual const boost::optional<Return> &
    a_priori_value() const override {
        return _a_priori_value;
    }

protected:
    virtual void to_row(const Return &, row &) const override   { abort(); }

private:
    template<typename T>
    struct value_expressionist : public abstract_expressionist {
        const T _value;

        explicit value_expressionist(const T &v) : _value(v)    {}
        virtual void write_expression(sql &cmd) const override  { cmd.write_value(_value); }
        virtual column_id_set imports() const override          { return {}; }
    };

    template<typename T>
    std::unique_ptr<const abstract_expressionist>
    make_value_expressionist(const T &value) {
        return quince::make_unique<value_expressionist<T>>(value);
    }

    boost::optional<Return> _a_priori_value;
};

template<typename Return>
class exprn_mapper : public general_exprn_mapper<Return> {
public:
    exprn_mapper(const Return &value) :
        abstract_mapper_base(boost::none),
        abstract_mapper<Return>(boost::none),
        general_exprn_mapper<Return>(value)
    {}

    exprn_mapper(const abstract_mapper<Return> &delegate) :
        abstract_mapper_base(boost::none),
        abstract_mapper<Return>(boost::none),
        general_exprn_mapper<Return>(delegate)
    {}

    explicit exprn_mapper(std::unique_ptr<const abstract_expressionist> &&expressionist) :
        abstract_mapper_base(boost::none),
        abstract_mapper<Return>(boost::none),
        general_exprn_mapper<Return>(std::move(expressionist))
    {}

    virtual std::unique_ptr<cloneable>
    clone_impl() const override {
        return quince::make_unique<exprn_mapper<Return>>(*this);
    }
};

template<typename Content>
class exprn_mapper<boost::optional<Content>> :
    public optional_mapper<Content>,
    public general_exprn_mapper<boost::optional<Content>>
{
private:
    typedef boost::optional<Content> return_type;
    typedef optional_mapper<Content> base_optional_mapper;
    typedef general_exprn_mapper<return_type> base_exprn_mapper;

    static_assert(
        std::is_base_of<exposed_mapper_type<Content>, exprn_mapper<Content>>::value,
        "Error in logic justifying use of optional_mapper<Content>'s constructor"
    );

public:
    exprn_mapper(const boost::optional<std::string> &name, const mapper_factory &creator) :
        abstract_mapper_base(boost::none),
        abstract_mapper<return_type>(boost::none),
        base_optional_mapper(exprn_mapper<Content>(name, creator)),
        base_exprn_mapper(this->make_delegating_expressionist(content_mapper()))
    {}

    exprn_mapper(const exposed_mapper_type<Content> &content_mapper) :
        abstract_mapper_base(boost::none),
        abstract_mapper<return_type>(boost::none),
        base_optional_mapper(exprn_mapper<Content>(content_mapper)),
        base_exprn_mapper(this->make_delegating_expressionist(content_mapper()))
    {}

    exprn_mapper(std::unique_ptr<const abstract_expressionist> &&expressionist) :
        abstract_mapper_base(boost::none),
        abstract_mapper<return_type>(boost::none),
        base_optional_mapper(exprn_mapper<Content>(std::move(expressionist))),
        base_exprn_mapper(this->make_delegating_expressionist(content_mapper()))
    {}

    //template<typename Arg>
    //exprn_mapper(Arg &&arg) :
    //    abstract_mapper_base(boost::none),
    //    abstract_mapper<return_type>(boost::none),
    //    base_optional_mapper(exprn_mapper<Content>(std::forward<Arg>(arg))),
    //    base_exprn_mapper(make_delegating_expressionist(content_mapper()))
    //{}

    virtual std::unique_ptr<cloneable>
    clone_impl() const override {
        return quince::make_unique<exprn_mapper<return_type>>(*this);
    }

    virtual const exprn_mapper<Content> &
    operator*() const override {
        return content_mapper();
    }

    virtual const exprn_mapper<Content> *
    operator->() const override {
        return &content_mapper();
    }

    virtual void
    from_row(const row &src, return_type &dest) const override {
        base_exprn_mapper::from_row(src, dest);
    }
    virtual void
    to_row(const return_type &src, row &dest) const override {
        base_exprn_mapper::to_row(src, dest);
    }
    virtual column_id_set
    imports() const override {
        return base_exprn_mapper::imports();
    }
    virtual const boost::optional<return_type> &
    a_priori_value() const override {
        return base_exprn_mapper::a_priori_value();
    }
    virtual void
    allow_all_null() const override { 
        base_exprn_mapper::allow_all_null();
    }
    virtual column_type
    get_column_type(bool is_generated) const override {
        return base_exprn_mapper::get_column_type(is_generated);
    }
    virtual void
    for_each_column(std::function<void(const column_mapper &)> op) const override {
        base_exprn_mapper::for_each_column(op);
    }
    virtual void
    for_each_persistent_column(std::function<void (const persistent_column_mapper &)> op) const override {
        base_exprn_mapper::for_each_persistent_column(op);
    }

private:
    const exprn_mapper<Content> &
    content_mapper() const {
        return dynamic_cast<const exprn_mapper<Content>&>(base_optional_mapper::operator*());
    }
};

QUINCE_UNSUPPRESS_MSVC_WARNING


template<typename T>
std::unique_ptr<abstract_mapper<T>>
make_new_mapper(
    const T &value,
    typename std::enable_if<! std::is_base_of<abstract_mapper_base, T>::value>::type * = nullptr
) {
    return quince::make_unique<exprn_mapper<T>>(value);
}

inline std::unique_ptr<abstract_mapper<std::string>>
make_new_mapper(const char *chars) {
    return make_new_mapper(std::string(chars));
}

template<typename Mapper>
std::unique_ptr<Mapper>
make_new_mapper(
    const Mapper &mapper,
    typename std::enable_if<std::is_base_of<abstract_mapper_base, Mapper>::value>::type * = nullptr
) {
    return clone(mapper);
}

template<typename Target, typename X>
std::unique_ptr<abstract_mapper<Target>>
make_new_mapper_checked(const X &x) {
    return boost::implicit_cast<std::unique_ptr<abstract_mapper<Target>>>(make_new_mapper(x));
}

template<typename... Xs>
std::vector<std::unique_ptr<const abstract_mapper_base>>
make_new_mappers(const Xs &...xs) {
    return make_unique_ptr_vector<const abstract_mapper_base>(make_new_mapper(xs)... );
}

template<typename Target, typename... Xs>
std::vector<std::unique_ptr<const abstract_mapper_base>>
make_new_mappers_checked(const Xs &...xs) {
    return make_unique_ptr_vector<const abstract_mapper_base>(make_new_mapper_checked<Target>(xs)...);
}

}


#define QUINCE_DEFINE_SERVER_ONLY_TYPE(NAME) \
    struct NAME {}; \
    \
    quince::column_type \
    inline QUINCE_get_column_type_via_adl(NAME *) { \
        throw server_only_exception(); \
    } \
    \
    inline void \
    QUINCE_from_cell_via_adl(const quince::database &, const quince::cell &, NAME &) { \
        throw server_only_exception(); \
    }

#endif
