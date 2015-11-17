//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/ptr_container/ptr_deque.hpp>
#include <mutex>
#include <quince/database.h>
#include <quince/detail/session.h>
#include <quince/detail/util.h>

using boost::optional;
using std::lock_guard;
using std::mutex;
using std::string;
using std::unique_ptr;
using std::weak_ptr;


namespace quince {

class database::session_pool : private boost::noncopyable {
public:
    explicit session_pool(const database &database) :
        _database(database),
        _era(0)
    {}

    session
    get_session() {
        new_session s;
        uint64_t era;
        {
            lock_guard<mutex> lock(_mutex);
            const auto iter = _reserve.begin();
            if (iter != _reserve.end())
                s.reset(_reserve.release(iter).release());
            era = _era;
        }
        if (! s)  s = _database.make_session();

        return session(
            s.release(),
            [=](abstract_session_impl * const released) {
                lock_guard<mutex> lock(_mutex);
                if (era == _era)
                    _reserve.push_front(released);
            }
        );
    }

    void
    reset() {
        lock_guard<mutex> lock(_mutex);
        _reserve.clear();
        _era++;
    }

private:
    const database &_database;
    mutex _mutex;
    uint64_t _era;
    boost::ptr_deque<abstract_session_impl> _reserve;
};


database::database(
    unique_ptr<const mapping_customization> for_db,
    unique_ptr<const mapping_customization> for_dbms
) :
    _mapper_factory(own_or_null(for_db), own_or_null(for_dbms)),
    _sessions(quince::make_unique<session_pool>(*this))
{}

database::~database()
{}

session
database::get_session() const {
    if (_session_finder.get() == nullptr)
        // First time get_session() has been called for this database.
        _session_finder.reset(new weak_ptr<abstract_session_impl>);

    session result = _session_finder->lock();
    if (! result) {
        // First time get_session() has been called for this database on this thread.
        result = _sessions->get_session();
        *_session_finder = result;
    }
    return result;
}

bool
database::is_using_session(const session &comparand) const {
    const weak_ptr<abstract_session_impl> * const current = _session_finder.get();
    return current != nullptr  &&  current->lock() == comparand;
}

void
database::discard_connections() const {
    _sessions->reset();
}

column_type
database::retrievable_column_type(column_type declared) const {
    return declared;
}

optional<size_t>
database::max_column_name_length() const {
    return boost::none;
}

}
