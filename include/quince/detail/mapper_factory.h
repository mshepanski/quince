#ifndef QUINCE__mappers__detail__mapper_factory_h
#define QUINCE__mappers__detail__mapper_factory_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <type_traits>
#include <vector>
#include <quince/detail/compiler_specific.h>
#include <quince/mappers/detail/exposed_mapper_type.h>
#include <quince/mapping_customization.h>


/*
    Everything in this file is for quince internal use only.
*/

namespace quince {

class mapper_factory {
public:
    mapper_factory(const mapper_factory &) = default;

    mapper_factory(const mapping_customization *first_or_null, const mapper_factory &others) :
        _customization(construct(first_or_null, others._customization))
    {}

    mapper_factory(const mapping_customization *first_or_null, const mapping_customization *second_or_null) :
        _customization(construct(first_or_null, second_or_null))
    {}

    template<typename T>
    typename std::enable_if<
        ! is_polymorphically_mapped<T>::value,
        std::unique_ptr<exposed_mapper_type<T>>
    >::type
    create(const boost::optional<std::string> &name) const {
        return std::make_unique<static_mapper_type<T>>(name, *this);
    }

    template<typename T>
    typename std::enable_if<
        is_polymorphically_mapped<T>::value,
        std::unique_ptr<exposed_mapper_type<T>>
    >::type
    create(const boost::optional<std::string> &name) const {
        for (const mapping_customization *customization: _customization)
            if (std::unique_ptr<abstract_mapper<T>> m = customization->create<T>(name, *this))
                return std::move(m);
        abort();
    }

private:
    static std::vector<const mapping_customization *>
    construct(const mapping_customization *first_or_null, const std::vector<const mapping_customization *> &others) {
        std::vector<const mapping_customization *> result;
        if (first_or_null)  result.push_back(first_or_null);
        result.insert(result.end(), others.begin(), others.end());
        return result;
    }

    static std::vector<const mapping_customization *>
    construct(const mapping_customization *first_or_null, const mapping_customization *second_or_null) {
        std::vector<const mapping_customization *> result;
        if (first_or_null)  result.push_back(first_or_null);
        if (second_or_null) result.push_back(second_or_null);
        return result;
    }

    const std::vector<const mapping_customization *> _customization;
};

}

#endif
