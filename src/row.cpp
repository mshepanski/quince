//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <assert.h>
#include <boost/optional.hpp>
#include <quince/detail/row.h>
#include <quince/detail/util.h>

using boost::optional;
using std::string;
using std::vector;


namespace quince {

row::row(const database *database) :
    _database(database)
{
    assert(_database != nullptr);
}

const database &
row::get_database() const {
    return *_database;
}

const cell &
row::only_cell() const {
    switch (_cells.size()) {
        case 0:     throw missing_column_exception();
        case 1:     return _cells.front();
        default:    throw multi_column_exception();
    }
}

const cell *
row::find_cell(const string &name) const {
    if (optional<const uint32_t &> index = lookup(_map, name))
        return &_cells[*index];
    else
        return nullptr;
}

void
row::delete_if_exists(const string &name) {
    if (optional<const uint32_t &> index = lookup(_map, name))
        _cells[*index].clear();
}

void
row::add_cell(const cell &cell) {
    _cells.push_back(cell);
}

void
row::add_cell(const cell &cell, const string &name) {
    const auto index = _cells.size();

    add_cell(cell);
    _map[name] = boost::numeric_cast<int>(index);
}

void
row::add_cells(const vector<cell> &cells) {
    for (const cell &c : cells)  add_cell(c);
}

void
row::add_cells(const row &row) {
    add_cells(row._cells);
}

vector<cell>
row::values() const {
    vector<cell> result;
    result.reserve(_cells.size());
    for (const cell &c : _cells) if (c.has_value()) result.push_back(c);
    result.shrink_to_fit();
    return result;
}

optional<row>
row::pick(const vector<string> &names) const {
    row r(_database);
    bool got_values = false;
    for (const string &name : names) {
        if (const cell * const c = find_cell(name)) {
            r.add_cell(*c);
            got_values = true;
        }
        else
            r.add(boost::none);
    }
    return boost::make_optional(got_values, r);
}

uint64_t
get_as_count(const row &row) {
    return boost::numeric_cast<uint64_t>(row.get<int64_t>());
}

}