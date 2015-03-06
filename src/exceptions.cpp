//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/binomen.h>
#include <quince/exceptions.h>
#include <quince/detail/util.h>


namespace quince {

using std::set;
using std::string;
using std::to_string;

enum class column_type;

namespace {
    string
    to_string(const binomen &b) {
        string result;
        if (b._enclosure)
            result = *b._enclosure + ".";
        result += b._local;
        return result;
    }
}

exception::exception(const string &msg) :
    _msg(msg)
{}

const char *
exception::what() const throw() {
    return _msg.c_str();
}


deadlock_exception::deadlock_exception(const string &msg) :
    exception(msg)
{}

failed_connection_exception::failed_connection_exception() :
    exception("failed to connect to the database")
{}

broken_connection_exception::broken_connection_exception(const string &msg) :
    exception(msg)
{}



definition_exception::definition_exception(const string &msg) :
    exception(msg)
{}

unmapped_member_exception::unmapped_member_exception() :
    definition_exception("operation not permitted on a C++ member that has no quince mapper")
{}

column_name_length_exception::column_name_length_exception(const string &name, size_t max) :
    definition_exception(
        "Column name '" + name + "' is at or above the dbms-imposed limit of " + to_string(max) + " chars."
        "  Either take steps to shorten the column name, or configure the dbms to set a higher limit"
    )
{}


formation_exception::formation_exception(const string &msg) :
    exception(msg)
{}

server_only_exception::server_only_exception() :
    formation_exception("operation not permitted on an expression that has no corresponding C++ type")
{}

multi_column_exception::multi_column_exception() :
    formation_exception("operation not permitted on a multi-column type")
{}

cross_database_query_exception::cross_database_query_exception() :
    formation_exception("two databases referenced in one query")
{}

malformed_modification_exception::malformed_modification_exception() :
    formation_exception("table modification based on a query that is not directly tied to a table")
{}

outside_table_exception::outside_table_exception(const binomen &table) :
    formation_exception("illegal attempt to use fields that do not belong to table " + to_string(table))
{}

ambiguous_nulls_exception::ambiguous_nulls_exception() :
    formation_exception("type that allows all NULLs used in the part of a join that gives another interpretation to all NULLs")
{}

execution_attempt_exception::execution_attempt_exception(const string &msg) :
    exception(msg)
{}

table_closed_exception::table_closed_exception() :
    execution_attempt_exception("operation not permitted on a table that is not open")
{}


table_open_exception::table_open_exception() :
    execution_attempt_exception("operation not permitted on a table that is open")
{}

unsupported_exception::unsupported_exception() :
    execution_attempt_exception("an operation was attempted on a dbms that does not support it")
{}

non_current_txn_exception::non_current_txn_exception() :
    execution_attempt_exception("use of a transaction that is not the current one")
{}

no_current_txn_exception::no_current_txn_exception() :
    execution_attempt_exception("operation requires a current transaction")
{}

no_primary_key_exception::no_primary_key_exception() :
    execution_attempt_exception("attempt to open a table with no primary key")
{}


execution_response_exception::execution_response_exception(const string &msg) :
    exception(msg)
{}

no_row_exception::no_row_exception() :
    execution_response_exception("operation unexpectedly generated no output")
{}

multi_row_exception::multi_row_exception() :
    execution_response_exception("operation unexpectedly generated more than one row")
{}

missing_type_exception::missing_type_exception() :
    execution_response_exception("dbms has not provided a column type")
{}

missing_column_exception::missing_column_exception(const string &column_name) :
    execution_response_exception("retrieved data omitted data for column " + column_name)
{}


malformed_results_exception::malformed_results_exception() :
    execution_response_exception("retrieved data is not of the expected form")
{}

retrieved_unrecognized_type_exception::retrieved_unrecognized_type_exception(const string &column_type_name) :
    execution_response_exception("result column has type " + column_type_name + ", which quince does not recognize")
{}

retrieved_unexpected_type_exception::retrieved_unexpected_type_exception(column_type expected, column_type actual) :
    execution_response_exception(
        "expected column of type " + to_string(static_cast<int>(expected)) +
        ", received type was " + to_string(static_cast<int>(actual))
    )
{}

retrieved_unexpected_size_exception::retrieved_unexpected_size_exception(size_t expected, size_t actual) :
    execution_response_exception(
        "expected " + to_string(expected) + " bytes of data "
        ", received " + to_string(actual) + " bytes"
    )
{}

namespace {
    string
    to_string(const set<string> &strings) {
        string result;
        bool begun = false;
        for (const string &s: strings) {
            if (begun)  result += ", ";
            else        begun = true;
            result += s;
        }
        return result;
    }

    string
    diff(const set<string> &expected, const set<string> &actual) {
        const set<string> surplus = set_difference(actual, expected);
        const set<string> deficit = set_difference(expected, actual);
        string result;
        if (! surplus.empty())  result += "  The following are unexpectedly present: " + to_string(surplus) + ".";
        if (! deficit.empty())  result += "  The following are unexpectedly absent: " + to_string(deficit) + ".";
        return result;
    }
}

table_mismatch_exception::table_mismatch_exception(
    const binomen &table,
    const set<string> &expected_column_titles,
    const set<string> &actual_column_titles
) :
    execution_response_exception(
          "table " + to_string(table) + ", already exists, and its column titles are not the same as expected."
        + diff(expected_column_titles, actual_column_titles)
    )
{}

dbms_exception::dbms_exception(const string &msg) :
    execution_response_exception("dbms-detected error: " + msg)
{}

}
