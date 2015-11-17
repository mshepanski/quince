#ifndef QUINCE__mappers__detail__user_defined_class_mapper_h
#define QUINCE__mappers__detail__user_defined_class_mapper_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/optional.hpp>
#include <boost/preprocessor.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <quince/detail/compiler_specific.h>
#include <quince/detail/util.h>
#include <quince/mappers/detail/class_mapper_base.h>


/*

Hello and welcome to the most cryptic file in quince's source code.

The purpose of this file is to define the macros QUINCE_DEFINE_CLASS_MAPPER_WITH_BASES and QUINCE_DEFINE_CLASS_MAPPER.
All the other definitions you see are helpers for those two; they are not used outside.

The code is made cryptic by two factors:

1. There are macros everywhere.
2. The code is generalized to deal with user classes that have mapped bases, which is not the most common case.

In this comment I want to show you what it would be like without those complications.

Consider this user code:

    struct point {
        float x;
        float y;
    };
    QUINCE_MAP_CLASS(point, (x)(y))

The macro QUINCE_MAP_CLASS is expanded by <quince/define_mapper.h>.  The part of its expansion that concerns us here is:

    QUINCE_DEFINE_CLASS_MAPPER(QUINCE_MAPPER_point, point, (x)(y))

The following is what it expands into, minus complications 1 and 2, and with:
- comments added,
- a couple of typedefs (x_mapper_type, y_mapper_type) added,
- an out-of-class method put back in class,
- some changes to whitespace:

    class QUINCE_MAPPER_point :
        public quince::typed_class_mapper_base<point>  // where most of the work is done, see <quince/mappers/detail/class_mapper_base.h>
    {
    public:
        typedef quince::typed_class_mapper_base<point> main_base;

        // The following types both turn out to be quince::exposed_mapper<float>,
        // and, since float is a polymorphically mapped type, that turns out to be quince::abstract_mapper<float>.
        //
        typedef quince::exposed_mapper_type<quince::member_from_ptr_to_member<decltype(&point::x)>> x_mapper_type;
        typedef quince::exposed_mapper_type<quince::member_from_ptr_to_member<decltype(&point::y)>> y_mapper_type;

        typedef point value_type;

        // Constructor that is used whenever *this is to be some table's value mapper, or part of some table's
        // value mapper:
        //
        QUINCE_MAPPER_point(const boost::optional<std::string> &name, const quince::mapper_factory &creator) :
            quince::abstract_mapper_base(name),
            main_base(name),
            x(main_base::add(&point::x, "x", creator)),
            y(main_base::add(&point::y, "y", creator))
        {}

        // Constructor that is used whenever *this is to be a collector class for select() or a function in the join() family:
        //
        QUINCE_MAPPER_point(const x_mapper_type &a_x, const y_mapper_type &a_y) :
            quince::abstract_mapper_base(boost::none),
            main_base(boost::none),
            x(main_base::adopt_member(&point::x, a_x)),
            y(main_base::adopt_member(&point::y, a_y))
        {}

        virtual ~QUINCE_MAPPER_point()  {}
       
        virtual std::unique_ptr<cloneable>
        clone_impl() const override {
            return quince::make_unique<QUINCE_MAPPER_point>(*this);
        }

        template<typename DelayInstantiation = void>
        static void
        static_forbid_optionals() {
            x_mapper_type::static_forbid_optionals();
            y_mapper_type::static_forbid_optionals();
        }

        // The members of the mapper are the mappers of the members:
        //
        const x_mapper_type &x;
        const y_mapper_type &y;
    };

*/

#define QUINCE_DECLARE_BASE(dummy1, dummy2, BASE) \
    , public quince::class_mapper<BASE>

#define QUINCE_CTOR_ARGUMENT(MEMBER_NAME) \
    BOOST_PP_CAT(a_, MEMBER_NAME)

#define QUINCE_INIT_BASE(dummy1, dummy2, BASE) \
    , other_base<BASE>(name, creator)

#define QUINCE_ADD_MEMBER(dummy, CLASS_TYPE, MEMBER_NAME) \
    , MEMBER_NAME(main_base::add(&CLASS_TYPE::MEMBER_NAME, BOOST_PP_STRINGIZE(MEMBER_NAME), creator))

#define QUINCE_ADOPT_BASE(dummy1, dummy2, BASE) \
    main_base::adopt_base(boost::implicit_cast<const typename other_base<BASE>::main_base&>(*this));

#define QUINCE_MEMBER_MAPPER_TYPE(CLASS_TYPE, MEMBER_NAME) \
    quince::exposed_mapper_type<quince::member_from_ptr_to_member<decltype(&CLASS_TYPE::MEMBER_NAME)>>

#define QUINCE_DECLARE_CTOR_ARGUMENT(dummy, CLASS_TYPE, MEMBER_NAME) \
    const QUINCE_MEMBER_MAPPER_TYPE(CLASS_TYPE, MEMBER_NAME) &QUINCE_CTOR_ARGUMENT(MEMBER_NAME),

#define QUINCE_ADOPT_MEMBER(dummy, CLASS_TYPE, MEMBER_NAME) \
    , MEMBER_NAME(main_base::adopt_member(&CLASS_TYPE::MEMBER_NAME, QUINCE_CTOR_ARGUMENT(MEMBER_NAME)))

#define QUINCE_BASE_FROM_ROW(dummy1, dummy2, BASE) \
    this->quince::class_mapper<BASE>::from_row(src, dest);

#define QUINCE_BASE_TO_ROW(dummy1, dummy2, BASE) \
    this->quince::class_mapper<BASE>::to_row(src, dest);

#define QUINCE_BASE_FOR_EACH_COLUMN(dummy1, dummy2, BASE) \
    this->quince::class_mapper<BASE>::for_each_column(op);

#ifdef _MSC_VER
  // Longwinded form to avoid "fatal error C1001: An internal error has occurred in the compiler."
  //
# define QUINCE_USING_BASE_LOOKUP(dummy1, dummy2, BASE) \
    using decltype(QUINCE_static_lookup_class_mapper_type(std::declval<BASE>()))::lookup;
#else
# define QUINCE_USING_BASE_LOOKUP(dummy1, dummy2, BASE) \
    using quince::class_mapper<BASE>::lookup;
#endif

#define QUINCE_STATIC_FORBID_OPTIONALS_FROM_BASE(dummy1, dummy2, BASE) \
    quince::class_mapper<BASE>::static_forbid_optionals();

#define QUINCE_STATIC_FORBID_OPTIONALS_FROM_MEMBER(dummy, CLASS_TYPE, MEMBER_NAME) \
    QUINCE_MEMBER_MAPPER_TYPE(CLASS_TYPE, MEMBER_NAME)::static_forbid_optionals();

#define QUINCE_DEFINE_MEMBER(dummy, CLASS_TYPE, MEMBER_NAME) \
    const QUINCE_MEMBER_MAPPER_TYPE(CLASS_TYPE, MEMBER_NAME) &MEMBER_NAME;


#define QUINCE_DEFINE_CLASS_MAPPER_WITH_BASES(MAPPER_TYPE_NAME, CLASS_TYPE, BASES, MEMBERS) \
    \
    QUINCE_SUPPRESS_MSVC_DOMINANCE_WARNING \
    \
    class MAPPER_TYPE_NAME : \
        public quince::typed_class_mapper_base<CLASS_TYPE> \
        BOOST_PP_SEQ_FOR_EACH(QUINCE_DECLARE_BASE, , BASES) \
    { \
    private: \
        template<typename B> \
        using other_base = quince::class_mapper<B>; \
        \
    public: \
        typedef quince::typed_class_mapper_base<CLASS_TYPE> main_base; \
        \
        typedef CLASS_TYPE value_type; \
        \
        MAPPER_TYPE_NAME( \
            const boost::optional<std::string> &name, \
            const quince::mapper_factory &creator \
        ) : \
            quince::abstract_mapper_base(name), \
            main_base(name) \
            BOOST_PP_SEQ_FOR_EACH(QUINCE_INIT_BASE, , BASES) \
            BOOST_PP_SEQ_FOR_EACH(QUINCE_ADD_MEMBER, CLASS_TYPE, MEMBERS) \
        { \
            BOOST_PP_SEQ_FOR_EACH(QUINCE_ADOPT_BASE, , BASES) \
        } \
        \
        /* The following ctor can only be defined when BASES is empty */ \
        MAPPER_TYPE_NAME( \
            BOOST_PP_SEQ_FOR_EACH(QUINCE_DECLARE_CTOR_ARGUMENT, CLASS_TYPE, MEMBERS) /* avoid BOOST_PP_SEQ_ENUM because MEMBERS can be empty */ \
            void * = nullptr /* something to follow the last comma */ \
        ); \
        \
        virtual ~MAPPER_TYPE_NAME()  {} \
        \
        virtual std::unique_ptr<cloneable> \
        clone_impl() const override { \
            return quince::make_unique<MAPPER_TYPE_NAME>(*this); \
        } \
        \
        virtual void \
        from_row(const quince::row &src, CLASS_TYPE &dest) const override { \
            BOOST_PP_SEQ_FOR_EACH(QUINCE_BASE_FROM_ROW, , BASES) \
            main_base::from_row(src, dest); \
        } \
        \
        virtual void \
        to_row(const CLASS_TYPE &src, quince::row &dest) const override { \
            BOOST_PP_SEQ_FOR_EACH(QUINCE_BASE_TO_ROW, , BASES) \
            main_base::to_row(src, dest); \
        } \
        \
        /* Looking up a pointer-to-member B::*f has two stages: the static stage, i.e. choosing \
         * among the following overloads of lookup() based on B, and then the dynamic stage, \
         * i.e. the search that the chosen lookup() function performs. \
         * \
         */ \
        BOOST_PP_SEQ_FOR_EACH(QUINCE_USING_BASE_LOOKUP, , BASES) \
        using main_base::lookup; \
        \
        /* The following delegating methods look like they could be usings instead, \
         * but they can't; I've tried. \
         */ \
        virtual void \
        allow_all_null() const override { \
            main_base::allow_all_null(); \
        } \
        \
        virtual void \
        for_each_column(std::function<void(const quince::column_mapper &)> op) const override { \
            main_base::for_each_column(op); \
        } \
        \
        virtual void \
        for_each_persistent_column(std::function<void(const quince::persistent_column_mapper &)> op) const override { \
            main_base::for_each_persistent_column(op); \
        } \
        \
        virtual quince::column_id_set \
        imports() const override { \
            return main_base::imports(); \
        } \
        \
        template<typename DelayInstantiation = void> \
        static void \
        static_forbid_optionals() { \
            BOOST_PP_SEQ_FOR_EACH(QUINCE_STATIC_FORBID_OPTIONALS_FROM_BASE, , BASES) \
            BOOST_PP_SEQ_FOR_EACH(QUINCE_STATIC_FORBID_OPTIONALS_FROM_MEMBER, CLASS_TYPE, MEMBERS) \
        } \
        \
        BOOST_PP_SEQ_FOR_EACH(QUINCE_DEFINE_MEMBER, CLASS_TYPE, MEMBERS); \
        \
    private: \
        virtual void \
        build_match_tester(const quince::query_base &qb, quince::predicate &result) const override { \
            main_base::build_match_tester(qb, result); \
        } \
    }; \
    \
    QUINCE_UNSUPPRESS_MSVC_WARNING


#define QUINCE_DEFINE_CLASS_MAPPER(MAPPER_TYPE_NAME, CLASS_TYPE, MEMBERS) \
    QUINCE_DEFINE_CLASS_MAPPER_WITH_BASES(MAPPER_TYPE_NAME, CLASS_TYPE, , MEMBERS) \
    \
    inline MAPPER_TYPE_NAME::MAPPER_TYPE_NAME ( \
        BOOST_PP_SEQ_FOR_EACH(QUINCE_DECLARE_CTOR_ARGUMENT, CLASS_TYPE, MEMBERS) \
        void * \
    ) : \
        quince::abstract_mapper_base(boost::none), \
        main_base(boost::none) \
        BOOST_PP_SEQ_FOR_EACH(QUINCE_ADOPT_MEMBER, CLASS_TYPE, MEMBERS) \
    {}

#endif
