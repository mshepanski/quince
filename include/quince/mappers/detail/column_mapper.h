#ifndef QUINCE__mappers__detail__column_mapper_h
#define QUINCE__mappers__detail__column_mapper_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/column_type.h>
#include <quince/mappers/detail/abstract_mapper_base.h>


namespace quince {

/*
    Base class for any mapper that represents something that could have an SQL column alias.
*/
class column_mapper : public virtual abstract_mapper_base {

    // Everything in this class is for quince internal use only.

public:
    column_mapper(const boost::optional<std::string> &name);
    virtual ~column_mapper()  {}

    virtual column_type get_column_type(bool is_generated) const = 0;

    column_id id() const                { return _id; }
    const std::string &alias() const    { return _alias; }

    virtual void for_each_column(std::function<void(const column_mapper &)>) const override;

private:
    column_id _id;
    std::string _alias;
};

}

#endif
