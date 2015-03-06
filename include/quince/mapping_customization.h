#ifndef QUINCE__mapping_customization_h
#define QUINCE__mapping_customization_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <typeindex>
#include <unordered_map>
#include <quince/detail/clone.h>
#include <quince/detail/compiler_specific.h>
#include <quince/detail/util.h>


namespace quince {

class abstract_mapper_base;
template<typename> class abstract_mapper;
class mapper_factory;


// See http://quince-lib.com/custom/deployment.html
//
class mapping_customization : public cloneable {
public:
    explicit mapping_customization() :
        _custom_makers(1)       // workaround a bug that affects empty unordered_maps in VS
    {}

    template<typename T, typename MapperType>
    void
    customize() {
        static_assert(std::is_base_of<abstract_mapper<T>, MapperType>::value, "");

        _custom_makers[typeid(T)] =
            [](const boost::optional<std::string> &name, const mapper_factory &creator) {
                return std::make_unique<MapperType>(name, creator);
            };
    }


    // --- Everything from here to end of class is for quince internal use only. ---

    virtual std::unique_ptr<cloneable>
    clone_impl() const override {
        return std::make_unique<mapping_customization>(*this);
    }

private:
    friend class mapper_factory;

    template<typename T>
    std::unique_ptr<abstract_mapper<T>>
    create(const boost::optional<std::string> &name, const mapper_factory &creator) const {
        if (boost::optional<const maker &> custom = lookup(_custom_makers, typeid(T)))
            return downcast_mapper_ptr<T>((*custom)(name, creator));
        else
            return nullptr;
    }

    template<typename T>
    static std::unique_ptr<abstract_mapper<T>>
    downcast_mapper_ptr(std::unique_ptr<abstract_mapper_base> base) {
        return std::unique_ptr<abstract_mapper<T>>(
            dynamic_cast<abstract_mapper<T>*>(base.release())
        );
    }

    typedef std::function<
        std::unique_ptr<abstract_mapper_base>(
            const boost::optional<std::string> &name,
            const mapper_factory &creator
        )
    > maker;
    typedef std::unordered_map<std::type_index, maker> map;

    std::unordered_map<std::type_index, maker> _custom_makers;
};

};

#endif
