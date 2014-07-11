#ifndef QUINCE__mappers__detail__sql_h
#define QUINCE__mappers__detail__sql_h

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
#include <string>
#include <vector>
#include <boost/optional.hpp>

#include <quince/detail/clone.h>
#include <quince/detail/column_id.h>
#include <quince/detail/row.h>
#include <quince/detail/util.h>
#include <quince/database.h>


/*
    Everything in this file is for quince internal use only.
*/

namespace quince {

typedef int64_t column_id;
enum class combination_type;
enum class relation;
typedef universalizable_set<column_id> universalizable_column_id_set;
class abstract_query_base;
struct binomen;
class column_mapper;
struct foreign_spec;
class abstract_mapper_base;
class persistent_column_mapper;
class query_base;
template<typename> class abstract_mapper;
typedef abstract_mapper<bool> abstract_predicate;
enum class conditional_junction_type;
class collective_base;


// An sql object represents an SQL command (possibly a work in progress), including the
// SQL text and any attached data.  It has methods for generating the various SQL phrases,
// along with any attached data.
//
// The backend libraries provide subclasses for their own dialects of SQL, which customize
// the behaviour of this class by overriding some of its virtuals -- including some pure
// virtuals, so this is an abstract class.
//
// To get an sql object, call database::make_sql().  It will get the appropriate backend
// library to make an empty sql object for its dialect.  Then call the various methods
// below to add text and attached data.
//
// The SQL object also holds a certain amount of state about the command being generated:
//
//  - the set of columns whose aliases are currently in scope,
//  - the set of columns for which it would be advantageous to generate aliases,
//  - the table that will be implcitly referenced by column names, in the context of
//    creating an index.
//
class sql : public cloneable {
public:
    typedef std::string::size_type marker_t;

    virtual ~sql() {}

    virtual void write(const std::string &);

    virtual void write_quoted(const std::string &);
    virtual void write_quoted(const binomen &);
    virtual void write_next_subquery_alias();
    virtual void write_value(const cell &value);
    template<typename T>
    void write_value(const T &value) {
        write_value(get_database().to_cell(value));
    }

    virtual void write_evaluation(const column_mapper &);
    virtual void write_persistent_column(const std::string &table_name, const std::string &column_name);
    virtual void write_function_call(const std::string &function_name, const std::vector<const column_mapper *> &args);
    virtual void write_prefix_exprn(const std::string &op, const column_mapper &operand);
    virtual void write_postfix_exprn(const column_mapper &operand, const std::string &op);
    virtual void write_infix_exprn(const column_mapper &lhs, const std::string &op, const column_mapper &rhs);
    virtual void write_cast(const column_mapper &arg, const column_type cast_type);
    virtual void write_case(
        boost::optional<const column_mapper &> switch_,
        const std::vector<std::pair<const column_mapper *, const column_mapper *>> &clauses,
        boost::optional<const column_mapper &> default_
    );
    virtual void write_scalar_subquery(const query_base &);
    virtual void write_subquery_exists(const query_base &);
    virtual void write_in(const column_mapper &lhs, const std::vector<const column_mapper *> &rhs);
    virtual void write_in(const column_mapper &lhs, const query_base &rhs);
    virtual void write_intrinsic_comparison(relation, const abstract_column_sequence &lhs, const abstract_column_sequence &rhs);
    virtual void write_extrinsic_comparison(relation, const abstract_column_sequence &lhs, const row &rhs);
    virtual void write_collective_comparison(relation, const abstract_column_sequence &lhs, const collective_base &rhs);
    virtual void write_are_all_null(const abstract_column_sequence &);

    virtual void write_nulls_low(bool invert) = 0;

    virtual void write_where(const abstract_predicate &);
    virtual void write_distinct();
    virtual void write_distinct(const std::vector<const abstract_mapper_base*> &) = 0;
    virtual void write_returning(const abstract_mapper_base &);
    virtual void write_no_limit() = 0;
    virtual void write_limit(uint32_t);
    virtual void write_offset(uint32_t);
    virtual void write_select_list(const abstract_column_sequence &);
    virtual void write_group_by(const std::vector<const abstract_mapper_base *> &);
    virtual void write_ordered_by(const std::vector<const abstract_mapper_base *> &);
    virtual void write_values(const abstract_mapper_base &, const row &, boost::optional<column_id> excluded);
    virtual void write_cross_join(const std::vector<const abstract_query_base *> &joinees);   
    virtual void write_qualified_join(const abstract_query_base &lhs, const abstract_query_base &rhs, conditional_junction_type);
    virtual void write_combination(combination_type type, bool all, const query_base &rhs);
    virtual void write_on(const abstract_predicate &);

    virtual void
    write_create_index(
        const binomen &table,
        size_t per_table_index_count,
        const std::vector<const abstract_mapper_base *> &,
        bool unique
    ) = 0;

    virtual void
    write_create_table(
        const binomen &table,
        const abstract_mapper_base &value_mapper,
        const abstract_mapper_base &key_mapper,
        boost::optional<column_id> generated_key,
        const std::vector<foreign_spec> &
    );

    virtual void write_unconstrain_as_not_null(const binomen &table, const abstract_mapper_base &);
    virtual void write_constrain_as_not_null_where_appropriate(const binomen &table, const abstract_mapper_base &);

    virtual void write_begin_transaction();
    virtual void write_commit_transaction();
    virtual void write_rollback_transaction();

    virtual void write_mark_save_point(const std::string &);
    virtual void write_forget_save_point(const std::string &);
    virtual void write_revert_to_save_point(const std::string &);

    virtual void write_insert(
        const binomen &table,
        const abstract_mapper_base &,
        boost::optional<column_id> excluded
    );

    virtual void write_update(
        const binomen &table,
        const abstract_mapper_base &dest,
        const abstract_mapper_base &src,
        boost::optional<column_id> excluded
    );

    virtual void write_update(
        const binomen &table,
        const abstract_mapper_base &dest,
        const row &src,
        boost::optional<column_id> excluded
    );

    virtual void write_delete_from(const binomen &table);
    virtual void write_set_search_path(const std::string &schema_name);

    virtual void write_rename_table(const binomen &old, const std::string &new_local);
    virtual void write_add_columns(const binomen &table, const abstract_mapper_base &, boost::optional<column_id> generated_key);
    virtual void write_drop_columns(const binomen &table, const abstract_mapper_base &);
    virtual void write_rename_column(const binomen &table, const std::string &before, const std::string &after);
    virtual void write_set_columns_types(const binomen &table, const abstract_mapper_base &, boost::optional<column_id> generated_key);

    virtual void write_drop_table(const binomen &);
    virtual void write_drop_table_if_exists(const binomen &);

    virtual void write_select_none(const binomen &table);

    void attach_cell(const cell &);

    const database &get_database() const    { return *_database; }
    const row &get_input() const            { return _input; }
    const std::string &get_text() const     { return _text; }

    bool alias_is_wanted(column_id) const;
    bool alias_is_defined(column_id) const;

    std::string column_type_name(column_type) const;

    static std::string relop(relation);

    static const std::string unary_plus_operator;
    static const std::string unary_minus_operator;

    // Record the current write position, for later use by text_insertion_scope.
    //
    marker_t here() const;

    // Any text added to this sql object during the lifetime of a text_insertion_scope
    // will be inserted, starting at a given marker, rather than appended to the end.
    //
    // The reason you'd do this, rather than write all the text from left to right,
    // is because you want some text to be exempted from the effects of any wanted_aliases_scope,
    // additional_wanted_alises_scope, aliased_columns_scope or additional_aliased_columns_scope
    // that were in force when the text in its immediate vicinity was created.
    //
    // So, on the first pass, you don't write the exempted text; you merely record the
    // insertion point (by calling here()).  Later on, when the wanted_aliases_scopes etc.
    // have been destructed, you use text_insertion_scope to go back and insert the text
    // you missed.
    //
    class text_insertion_scope : private boost::noncopyable {
    public:
        // Beware: invalidates other markers:
        //
        text_insertion_scope(sql &, marker_t insertion_point);
        ~text_insertion_scope();
    private:
        sql &_command;
        const std::string _pending_tail;
    };

    class comma_separated_list_scope : private boost::noncopyable {
    public:
        explicit comma_separated_list_scope(sql &);
        void start_item();
        bool has_begun() const;

    private:
        sql &_command;
        bool _begun;
    };

    class wanted_aliases_scope : private boost::noncopyable {
    public:
        wanted_aliases_scope(sql &, const universalizable_column_id_set &wanted);
        ~wanted_aliases_scope();
    protected:
        explicit wanted_aliases_scope(sql &);
    private:
        sql &_command;
        const universalizable_column_id_set _pending;
    };

    class additional_wanted_aliases_scope : private wanted_aliases_scope {
    public:
        additional_wanted_aliases_scope(sql &, const universalizable_column_id_set &additional_wanted);
    };

    class aliased_columns_scope : private boost::noncopyable {
    public:
        aliased_columns_scope(sql &, const column_id_set &aliaseds);
        ~aliased_columns_scope();
    protected:
        explicit aliased_columns_scope(sql &);
    private:
        sql &_command;
        const column_id_set _pending;
    };

    class additional_aliased_columns_scope : private aliased_columns_scope {
    public:
        additional_aliased_columns_scope(sql &, const column_id_set &additional_aliaseds);
    };

    class expression_restriction_scope : private boost::noncopyable {
    public:
        expression_restriction_scope(sql &, const std::string &implicit_table);
        ~expression_restriction_scope();
    private:
        sql &_command;
    };

protected:
    explicit sql(const database &);

    typedef std::function<void(sql &)> thunk;

    virtual void write_parenthesized_persistent_column_list(
        const abstract_mapper_base &,
        boost::optional<column_id> excluded = boost::none
    );

    virtual void write_title(const persistent_column_mapper &, boost::optional<column_id> generated_key);
    virtual void write_titles(const abstract_mapper_base &, boost::optional<column_id> generated_key);

    virtual void write_simple_comparison(const std::string &op, thunk lhs, thunk rhs);

    virtual void
    write_flat_lexicographic_comparison(
        const std::string &compare_op,
        const std::string &logic_op,
        const std::vector<thunk> &lhs,
        const std::vector<thunk> &rhs
    );

    virtual void
    write_nesting_lexicographic_comparison(
        const std::string &compare_op,
        const std::string &strict_compare_op,
        const std::vector<thunk> &lhs,
        const std::vector<thunk> &rhs
    );

    virtual void
    write_lexicographic_comparison(
        relation r,
        const std::vector<thunk> &lhs,
        const std::vector<thunk> &rhs
    );

    virtual void write_subquery_exprn(const query_base &);

    virtual void write_alter_table(const binomen &table);

    static std::string strict_relop(relation);
    virtual std::string next_placeholder() = 0;

private:
    const database *_database;
    row _input;
    std::string _text;
    universalizable_column_id_set _wanted_aliases;
    column_id_set _aliased_columns;
    boost::optional<std::string> _implicit_table;
    uint32_t _next_subquery_alias;
};

}

#endif
