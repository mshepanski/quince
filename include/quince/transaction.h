#ifndef QUINCE__transaction_h
#define QUINCE__transaction_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <boost/noncopyable.hpp>
#include <quince/detail/compiler_specific.h>
#include <quince/detail/session.h>


namespace quince {

class database;
class transaction;

// See http://quince-lib.com/transactions.html
//
class transaction : private boost::noncopyable {
public:
    transaction(const database &);
    ~transaction();

    void commit();


    // --- Everything from here to end of class is for quince internal use only. ---

    void abort();

    const database &get_database() const;
    bool is_running() const;

    static const transaction *current_for(const database &);

    class impl;

private:
    static std::unique_ptr<impl> new_impl(const database &);

    static QUINCE_STATIC_THREADLOCAL const transaction *_current;

    const transaction * const _pending_current;
    const std::unique_ptr<impl> _impl;
};

}

#endif
