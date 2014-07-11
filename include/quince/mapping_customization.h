#ifndef QUINCE__mapping_customization_h
#define QUINCE__mapping_customization_h

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
