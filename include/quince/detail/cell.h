#ifndef QUINCE__mappers__detail__cell_h
#define QUINCE__mappers__detail__cell_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>
#include <boost/lexical_cast.hpp>
#include <quince/detail/column_type.h>
#include <quince/exceptions.h>


/*
    Everything in this file is for quince internal use only.
*/

namespace quince {

typedef std::vector<uint8_t> byte_vector;

// A cell is a single-column component of a row.  See class row for further comments.
//
class cell {
public:
    cell();

    cell(boost::optional<column_type> type, bool is_binary, const void *data, size_t size);

    template<typename CxxType>
    explicit cell(const CxxType &value) {
        set(value);
    }
 
    void clear();

    template<typename CxxType>
    void
    set(const CxxType &value) {
        set_type(get_column_type<CxxType>());
        set_data<sizeof(value)>(&value);
    }

    void set(const std::string &);
    void set(const timestamp &);
    void set(const time_type &);
    void set(const byte_vector &);

    template<typename CxxType>
    void
    get(CxxType &value) const {
        check_type<CxxType>();
        if (_is_binary)
            get_data<sizeof(value)>(&value);
        else
            value = boost::lexical_cast<CxxType>(chars(), size());
    }

    template<typename CxxType>
    CxxType
    get() const {
        CxxType result;
        get(result);
        return result;
    }

    void get(std::string &) const;
    void get(timestamp &) const;
    void get(time_type &) const;
    void get(byte_vector &) const;

    column_type type() const;

    bool has_value() const;

    const void *data() const;

    const char *chars() const;

    size_t size() const;

private:
    template<typename CxxType>
    void
    check_type() const {
        if (!_type)  throw missing_type_exception();

        const column_type expected = get_column_type<CxxType>();
        if (*_type != expected)  throw retrieved_unexpected_type_exception(expected, *_type);
    }

    boost::optional<column_type> _type;
    byte_vector _bytes;
    bool _is_binary;

    void set_type(column_type);

    template<size_t>
    void get_data(void *) const;

    template<size_t>
    void set_data(const void *);

    void get_string(std::string &) const;
    void set_string(const std::string &);
};

}

#endif
