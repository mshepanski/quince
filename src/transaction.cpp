//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/format.hpp>
#include <quince/database.h>
#include <quince/exceptions.h>
#include <quince/detail/sql.h>
#include <quince/transaction.h>
#include <quince/detail/compiler_specific.h>
#include <quince/detail/util.h>

using boost::format;
using std::shared_ptr;
using std::string;
using std::unique_ptr;


namespace quince {

namespace {
    class outermost;  // below
}

class transaction::impl {
public:
    impl(const database &database, outermost &outermost_for_same_db) :
        _database(database),
        _session(database.get_session()),
        _outermost_for_same_db(outermost_for_same_db),
        _is_running(true)
    {}

    virtual ~impl()  {}

    const database &get_database() const        { return _database; }
    bool is_running() const                     { return _is_running; }
    outermost &outermost_for_same_db() const    { return _outermost_for_same_db; }

    const session &
    get_session() const {
        if (! _database.is_using_session(_session))  throw non_current_txn_exception();

        return _session;
    }

    void
    commit() {
        if (! is_running())  throw no_current_txn_exception();

        try {
            rollforward();
            _is_running = false;
        }
        catch (...) {
            _is_running = false;
            throw;
        }
    }
    
    void
    abort() {
        if (! is_running())  throw no_current_txn_exception();

        rollback();
        _is_running = false;
    }

protected:
    virtual void rollforward() = 0;
    virtual void rollback() = 0;

    void stop() {
        if (_is_running) abort();
    }

private:
    const database &_database;
    const session _session;  // As long as we hold this, _database will keep using the same session
    outermost &_outermost_for_same_db;
    bool _is_running;
};


namespace {

    class outermost : public transaction::impl {
    public:
        outermost(const database &db) :
            transaction::impl(db, *this),
            _sql_begin(db.make_sql()),
            _sql_commit(db.make_sql()),
            _sql_rollback(db.make_sql()),
            _n_save_points(0)
        {
            const_cast<sql&>(*_sql_begin).write_begin_transaction();
            const_cast<sql&>(*_sql_commit).write_commit_transaction();
            const_cast<sql&>(*_sql_rollback).write_rollback_transaction();

            get_session()->exec(*_sql_begin);
        }

        ~outermost() {
            try {
                stop();
            } catch (...) {}
        }

        string
        next_save_point() {
            return (format("save_point_%1%") % _n_save_points++).str();
        }

    private:
        virtual void rollforward() override { get_session()->exec(*_sql_commit); }
        virtual void rollback() override    { get_session()->exec(*_sql_rollback); }

        // TODO: don't recreate these commands for each outermost object
        const unique_ptr<sql> _sql_begin;
        const unique_ptr<sql> _sql_commit;
        const unique_ptr<sql> _sql_rollback;

        uint32_t _n_save_points;
    };


    class inner : public transaction::impl {
    public:
        explicit inner(transaction::impl &current_for_same_db) :
            transaction::impl(current_for_same_db),  // copy
            _save_point(outermost_for_same_db().next_save_point()),
            _sql_mark(get_database().make_sql()),
            _sql_forget(get_database().make_sql()),
            _sql_revert(get_database().make_sql())
        {
            const_cast<sql&>(*_sql_mark).write_mark_save_point(_save_point);
            const_cast<sql&>(*_sql_forget).write_forget_save_point(_save_point);
            const_cast<sql&>(*_sql_revert).write_revert_to_save_point(_save_point);

            if (! is_running())  throw no_current_txn_exception();

            const string save_point(outermost_for_same_db().next_save_point());
            get_session()->exec(*_sql_mark);
        }

        ~inner() {
            try {
                stop();
                get_session()->exec(*_sql_forget);
            } catch (...) {}
        }

    private:
        virtual void rollforward() override {}
        virtual void rollback() override    { get_session()->exec(*_sql_revert); }

        const string _save_point;

        const unique_ptr<sql> _sql_mark;
        const unique_ptr<sql> _sql_forget;
        const unique_ptr<sql> _sql_revert;
    };
}


transaction::transaction(const database &database) :
    _pending_current(_current),
    _impl(new_impl(database))
{
    _current = this;
}

transaction::~transaction() {
    _current = _pending_current;
}

void
transaction::commit() {
    if (this != current_for(get_database()))  throw non_current_txn_exception();
    _impl->commit();
}

void
transaction::abort() {
    if (this != current_for(get_database()))  throw non_current_txn_exception();
    _impl->abort();
}

const database &
transaction::get_database() const {
    return _impl->get_database();
}

bool
transaction::is_running() const {
    return _impl->is_running();
}

const transaction *
transaction::current_for(const database &database) {
    for (const transaction *t = _current; t != nullptr; t = t->_pending_current)
        if (t->get_database() == database  &&  t->is_running())
            return t;
    return nullptr;
}

unique_ptr<transaction::impl>
transaction::new_impl(const database &database) {
    if (const transaction *current_for_db = current_for(database))
        return quince::make_unique<inner>(*current_for_db->_impl);
    else
        return quince::make_unique<outermost>(database);
}

QUINCE_STATIC_THREADLOCAL const transaction *transaction::_current = nullptr;

}
