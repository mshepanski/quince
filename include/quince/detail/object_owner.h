#ifndef QUINCE__mappers__detail__object_owner_h
#define QUINCE__mappers__detail__object_owner_h

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

#include <assert.h>
#include <vector>
#include <boost/any.hpp>


/*
    Everything in this file is for quince internal use only.
*/

namespace quince {

// object_owner provides a small but very frequently used service: it keeps a shared_ptr
// to some constant data item.
//
// The pattern is this: you make object_owner a base of your class Foo.  Then, every time
// you want a Foo to take posession of a Bar, with shared_ptr semantics (so posession of
// the Bar will be shared when Foo is copied), you pass the Bar by unique_ptr to own().
// own() returns a const reference to the shared Bar, and from then on you can choose to
// use the const reference, or to take its address and use a const bald pointer.
//
// I haven't extended it to non-const sharing because I think that would be too error-prone.
//
// The impetus to write this class came from class mappers, where I wanted the member mappers
// to be accessible with "." rather than "->", so that server-side expressions would resemble
// client-side expressions visually.  But then I got to like this idea: confining all the
// memory management policy to the object_owner class, rather than nodding to it all through
// the code by explicit mention of shared_ptrs.
//

class object_owner {
protected:
    template<typename T>
    const T &
    own(std::unique_ptr<T> &&object) {
        const T * const raw = object.get();
        assert(raw != nullptr);
        const std::shared_ptr<const void> shared(object.release());
        _objects.push_back(shared);
        assert(raw == _objects.back().get());
        return *raw;
    }

    template<typename T>
    const T &
    own(std::unique_ptr<T> &object) {
        return own(std::move(object));
    }

    template<typename T>
    const T *
    own_or_null(std::unique_ptr<T> &object) {
        if (object) return &own(object);
        else        return nullptr;
    }

private:
    std::vector<std::shared_ptr<const void>> _objects;
};

}

#endif
