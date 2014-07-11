#ifndef QUINCE__mappers__detail__column_mapper_h
#define QUINCE__mappers__detail__column_mapper_h

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
