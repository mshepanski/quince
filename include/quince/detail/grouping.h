#ifndef QUINCE__mappers__detail__grouping_h
#define QUINCE__mappers__detail__grouping_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/abstract_query.h>
#include <quince/detail/query_base.h>


namespace quince {

template<typename> class query;
class table_base;

// class grouping is what q.group(...) returns.  You should never have to mention this class by name,
// just call its select() method straight away: q.group(...).select(...).  See the discussion here:
// http://quince-lib.com/queries/select/with_group.html
//
class grouping : private object_owner {
public:
    grouping(
        std::unique_ptr<const query_base> query,
        std::vector<std::unique_ptr<const abstract_mapper_base>> &&group_by
    ) :
        _query(own(query))
    {
        // TODO: refactor so I can use the code currently in abstract_expressionist::own_all().

        for (auto &g: group_by) _group_by.push_back(&own(g));
    }

    template<typename Mapper>
    query<typename Mapper::value_type>
    select(const Mapper &mapper) const {
        return query<typename Mapper::value_type>(
            * _query.selectable_equivalent(),
            mapper,
            clone_all(_group_by)
        );
    }

    template<typename Collector, typename... Mappers>
    query<Collector>
    select(const Mappers &... mappers) const {
        return select(static_mapper_type<Collector>(mappers...));
    }

    template<typename... Mappers>
    query<std::tuple<typename Mappers::value_type...>>
    select(const Mappers &... mappers) const {
        // MSVC won't let me say the obvious thing:
        //      typedef std::tuple<typename Mappers::value_type...> tuple_type;
        // so:
        //
        typedef decltype(std::make_tuple(mappers.value_declval()...)) tuple_type;
        return select<tuple_type>(mappers...);
    }

private:
    const query_base &_query;
    std::vector<const abstract_mapper_base *> _group_by;
};


/*
    The following functions are defined here, away from their own classes, to avoid cyclic
    header dependencies.
*/

template<typename... Ts>
grouping
query_base::group(const abstract_mapper<Ts> &... ms) const {
    return grouping(
        clone(*this),
        make_unique_ptr_vector<const abstract_mapper_base>(clone(ms)...)
    );
}

template<typename Value>
template<typename...Args>
grouping
abstract_query<Value>::group(Args && ... args) const {
    if (auto q = dynamic_cast<const query<Value> *>(this))
        return q->group(std::forward<Args>(args)...);
    return wrapped().group(std::forward<Args>(args)...);
}

}

#endif
