//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <tuple>
#include <quince/detail/binomen.h>

using boost::optional;
using std::string;


namespace quince {

binomen
binomen::split(const string &full, const optional<string> &default_enclosure) {
    const size_t dot = full.rfind('.');
    return dot == string::npos
        ? binomen(default_enclosure, full)
        : binomen(full.substr(0, dot), full.substr(dot+1));
}

binomen::binomen(const optional<string> &enclosure, const string &local) :
    _enclosure(enclosure),
    _local(local)
{}

}