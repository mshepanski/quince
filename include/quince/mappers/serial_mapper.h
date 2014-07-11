#ifndef QUINCE__mappers__serial_mapper_h
#define QUINCE__mappers__serial_mapper_h

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

#include <stdint.h>
#include <quince/detail/compiler_specific.h>
#include <quince/mappers/direct_mapper.h>
#include <quince/query.h>
#include <quince/serial.h>


QUINCE_SUPPRESS_MSVC_DOMINANCE_WARNING

namespace quince {

template<typename> class abstract_mapper;

class mapper_factory;

// serial_mapper<Content> is, by default, the mapper class for serial.
//
// However serial is a dynamically mapped type, so it's possible for a backend library
// to choose a different mapper class for serial, but for now that doesn't happen.
//
class serial_mapper : public abstract_mapper<serial>, public direct_mapper<int64_t> {

    // Everything in this class is for quince internal use only.

public:
    serial_mapper(const boost::optional<std::string> &name, const mapper_factory &);

    virtual std::unique_ptr<cloneable> clone_impl() const override;
    virtual void from_row(const row &src, serial &dest) const override;
    virtual void to_row(const serial &src, row &dest) const override;
    virtual column_type get_column_type(bool is_generated) const override;
    virtual void for_each_column(std::function<void(const column_mapper &)> op) const override;

    static void static_forbid_optionals()  {};

private:
    virtual void build_match_tester(const query_base &, predicate &result) const override;
};

}

QUINCE_UNSUPPRESS_MSVC_WARNING

#endif
