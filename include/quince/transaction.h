#ifndef QUINCE__transaction_h
#define QUINCE__transaction_h

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
