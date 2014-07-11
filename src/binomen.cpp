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