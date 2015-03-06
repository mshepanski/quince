#ifndef QUINCE__mappers__serial_mapper_h
#define QUINCE__mappers__serial_mapper_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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
