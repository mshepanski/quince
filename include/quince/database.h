#ifndef QUINCE__database_h
#define QUINCE__database_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <vector>
#include <string>
#include <boost/optional.hpp>
#include <boost/thread/tss.hpp>
#include <quince/detail/mapper_factory.h>
#include <quince/detail/object_owner.h>
#include <quince/detail/row.h>
#include <quince/detail/session.h>
#include <quince/database.h>
#include <quince/mapping_customization.h>
#include <quince/serial.h>


namespace quince {

enum class combination_type;
enum class conditional_junction_type;
struct index_spec;
class serial_mapper;
class sql;


// Base class of the database classes defined by the backend libraries.
//
class database : private object_owner, private boost::noncopyable {

    // Everything in this class is for quince internal use only.

public:
    virtual ~database();

    bool operator==(const database &that) const     { return this == &that; }
    bool operator!=(const database &that) const     { return this != &that; }

    session get_session() const;

    bool is_using_session(const session &comparand) const;

    void discard_connections() const;

    virtual std::unique_ptr<sql>            make_sql() const = 0;
    virtual boost::optional<std::string>    get_default_enclosure() const = 0;
    virtual void                            make_enclosure_available(const boost::optional<std::string> &enclosure_name) const = 0;
    virtual new_session                     make_session() const = 0;
    virtual std::vector<std::string>        retrieve_column_titles(const binomen &table) const = 0;
    virtual serial                          insert_with_readback(std::unique_ptr<sql> insert, const serial_mapper &readback_mapper) const = 0;
    virtual column_type                     retrievable_column_type(column_type declared) const;
    virtual std::string                     column_type_name(quince::column_type) const = 0;
    virtual boost::optional<size_t>         max_column_name_length() const;

    virtual bool supports_join(conditional_junction_type) const = 0;
    virtual bool supports_combination(combination_type, bool all) const = 0;
    virtual bool supports_nested_combinations() const = 0;
    virtual bool supports_index(const index_spec &) const = 0;
    virtual bool imposes_combination_precedence() const = 0;

    template<typename SingleColumnType>
    void
    from_cell(const cell &src, SingleColumnType &dest) const {
        row tmp_row(this);
        const auto mapper = temporary_mapper<SingleColumnType>();
        tmp_row.add_cell(src, mapper->only_column().alias());
        mapper->from_row(tmp_row, dest);
    }

    template<typename SingleColumnType>
    cell
    to_cell(const SingleColumnType &src) const {
        row tmp_row(this);
        const auto mapper = temporary_mapper<SingleColumnType>();
        mapper->to_row(src, tmp_row);
        return * tmp_row.find_cell(mapper->only_column().name());
    }

    template<typename T>
    column_type
    only_column_type() const {
        // Could probably optimize to avoid creating the mapper.
        return temporary_mapper<T>()->only_column().get_column_type(false);
    }

    const mapper_factory &get_mapper_factory() const  { return _mapper_factory; }

protected:
    explicit database(
        std::unique_ptr<const mapping_customization> for_db,
        std::unique_ptr<const mapping_customization> for_dbms
    );

private:
    template<typename T>
    std::unique_ptr<exposed_mapper_type<T>>
    temporary_mapper() const {
        return _mapper_factory.create<T>(std::string("$tmp"));
    }

    const mapper_factory _mapper_factory;
    class session_pool;
    const std::unique_ptr<session_pool> _sessions;
    mutable boost::thread_specific_ptr<std::weak_ptr<abstract_session_impl>> _session_finder;
};

}

#endif
