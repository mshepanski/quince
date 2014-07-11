#ifndef QUINCE__mappers__detail__session_h
#define QUINCE__mappers__detail__session_h

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

#include <memory>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>


/*
    Everything in this file is for quince internal use only.
*/

namespace quince {

class row;
class sql;

// abstract interface for backend-specific objects that hold the state of retrieval of
// multi-row output from a query.
//
// (I know, it's not much of an interface.  All the quince library does is get these things
// from the backend and pass them back to backend functions.)
//
struct abstract_result_stream_impl {
    virtual ~abstract_result_stream_impl()  {}
};
typedef std::shared_ptr<abstract_result_stream_impl> result_stream;


// abstract interface for backend-specific objects that represent a connection to a database.
//
class abstract_session_impl : private boost::noncopyable {
public:
    virtual ~abstract_session_impl()  {}

    virtual bool                    unchecked_exec(const sql &) = 0;
    virtual void                    exec(const sql &) = 0;
    virtual result_stream           exec_with_stream_output(const sql &, uint32_t fetch_size) = 0;
    virtual std::unique_ptr<row>    exec_with_one_output(const sql &) = 0;
    virtual std::unique_ptr<row>    next_output(const result_stream &) = 0;
};

typedef std::shared_ptr<abstract_session_impl> session;
typedef std::unique_ptr<abstract_session_impl> new_session;

}

#endif
