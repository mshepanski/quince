#ifndef QUINCE__mappers__exceptions_h
#define QUINCE__mappers__exceptions_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <exception>
#include <set>
#include <string>


/*
    See http://quince-lib.com/exceptions.html
*/
namespace quince {

struct binomen;
enum class column_type;

class exception : public std::exception {
public:
    explicit exception(const std::string &msg);
    const char *what() const throw();

private:
    const std::string _msg;
};


class deadlock_exception : public exception {
public:
    explicit deadlock_exception(const std::string &msg);
};

class failed_connection_exception : public exception {
public:
    failed_connection_exception();
};

class broken_connection_exception : public exception {
public:
    explicit broken_connection_exception(const std::string &msg);
};


class definition_exception : public exception {
protected:
    explicit definition_exception(const std::string &msg);
};

class unmapped_member_exception : public definition_exception {
public:
    unmapped_member_exception();
};

class column_name_length_exception : public definition_exception {
public:
    column_name_length_exception(const std::string &name, size_t max);
};


class formation_exception : public exception {
protected:
    explicit formation_exception(const std::string &msg);
};

class server_only_exception : public formation_exception {
public:
    server_only_exception();
};

class multi_column_exception : public formation_exception {
public:
    multi_column_exception();
};

class cross_database_query_exception : public formation_exception {
public:
    cross_database_query_exception();
};

class malformed_modification_exception : public formation_exception {
public:
    malformed_modification_exception();
};

class outside_table_exception : public formation_exception {
public:
    explicit outside_table_exception(const binomen &table);
};

class ambiguous_nulls_exception : public formation_exception {
public:
    explicit ambiguous_nulls_exception();
};

class execution_attempt_exception : public exception {
protected:
    explicit execution_attempt_exception(const std::string &msg);
};

class table_closed_exception : public execution_attempt_exception {
public:
    table_closed_exception();
};

class table_open_exception : public execution_attempt_exception {
public:
    table_open_exception();
};

class unsupported_exception : execution_attempt_exception {
public:
    unsupported_exception();
};

class non_current_txn_exception : public execution_attempt_exception {
public:
    non_current_txn_exception();
};

class no_current_txn_exception : public execution_attempt_exception {
public:
    no_current_txn_exception();
};

class no_primary_key_exception : public execution_attempt_exception {
public:
    no_primary_key_exception();
};

class execution_response_exception : public exception {
protected:
    explicit execution_response_exception(const std::string &msg);
};

class no_row_exception : public execution_response_exception {
public:
    no_row_exception();
};

class multi_row_exception : public execution_response_exception {
public:
    multi_row_exception();
};

class missing_type_exception : public execution_response_exception {
public:
    missing_type_exception();
};

class missing_column_exception : public execution_response_exception {
public:
    explicit missing_column_exception(const std::string &column_name = "<any>");
};

class malformed_results_exception : public execution_response_exception {
public:
    malformed_results_exception();
};

class retrieved_unrecognized_type_exception : public execution_response_exception {
public:
    template<typename ServerColumnType>
    explicit retrieved_unrecognized_type_exception(ServerColumnType invalid) :
        retrieved_unrecognized_type_exception(std::to_string(invalid))
    {}

private:
    explicit retrieved_unrecognized_type_exception(const std::string &column_type_name);
};

class retrieved_unexpected_type_exception : public execution_response_exception {
public:
    retrieved_unexpected_type_exception(column_type expected, column_type actual);
};

class retrieved_unexpected_size_exception : public execution_response_exception {
public:
    retrieved_unexpected_size_exception(size_t expected, size_t actual);
};

class table_mismatch_exception : public execution_response_exception {
public:
    table_mismatch_exception(
        const binomen &table,
        const std::set<std::string> &expected_column_titles,
        const std::set<std::string> &actual_column_titles
    );
};

class dbms_exception : public execution_response_exception {
public:
    explicit dbms_exception(const std::string &msg);
};

}

#endif
