//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/asio/detail/socket_ops.hpp>  // for htonl() etc.
#include <quince/detail/cell.h>
#include <quince/detail/util.h>

using std::string;


namespace quince {

cell::cell() {
    clear();
}

cell::cell(boost::optional<column_type> type, bool is_binary, const void *data, size_t size) :
    _type(type),
    _is_binary(is_binary)
{
    const uint8_t *bytes = static_cast<const uint8_t *>(data);
    _bytes.assign(bytes, bytes + size);
}

void cell::clear() {
    _type = boost::none;
    _bytes.clear();
    _is_binary = true;
}

void cell::set(const string &src) {
    set_type(get_column_type<string>());
    set_string(src);
}

void cell::set(const timestamp &src) {
    set_type(get_column_type<timestamp>());
    set_string(src);
}

void cell::set(const byte_vector &src) {
    set_type(get_column_type<byte_vector>());
    _bytes = src;
    _is_binary = true;
}

void cell::get(string &dest) const {
    check_type<string>();
    get_string(dest);
}

void cell::get(timestamp &dest) const {
    check_type<string>();
    get_string(dest);
}

void cell::get(byte_vector &dest) const {
    check_type<byte_vector>();
    dest = _bytes;
}

column_type cell::type() const {
    if (! _type)  throw missing_type_exception();
    return *_type;
}

bool cell::has_value() const {
    return bool(_type);
}

const void *cell::data() const {
    return base_address(_bytes);
}

const char *cell::chars() const {
    return static_cast<const char *>(data());
}

size_t cell::size() const {
    return _bytes.size();
}


void cell::set_type(column_type type) {
    _type = type;
}

namespace {
    template<size_t Size>
    void native_to_big_endian(const void *src, void *dest);

    template<>
    void native_to_big_endian<1>(const void *src, void *dest) {
        *static_cast<uint8_t *>(dest) = *static_cast<const uint8_t *>(src);
    }
    template<>
    void native_to_big_endian<2>(const void *src, void *dest) {
        *static_cast<uint16_t *>(dest) = htons(*static_cast<const uint16_t *>(src));
    }
    template<>
    void native_to_big_endian<4>(const void *src, void *dest) {
        *static_cast<uint32_t *>(dest) = htonl(*static_cast<const uint32_t *>(src));
    }
    template<>
    void native_to_big_endian<8>(const void*src, void *dest) {
        uint64_t native = * static_cast<const uint64_t*>(src);
        uint8_t *big_endian = static_cast<uint8_t*>(dest);
        for (size_t i = sizeof(native); i --> 0; ) {
            big_endian[i] = native & 0xff;
            native >>= 8;
        }
    }

    template<size_t Size>
    void big_endian_to_native(const void *src, void *dest);

    template<>
    void big_endian_to_native<1>(const void *src, void *dest) {
        *static_cast<uint8_t *>(dest) = *static_cast<const uint8_t *>(src);
    }
    template<>
    void big_endian_to_native<2>(const void *src, void *dest) {
        *static_cast<uint16_t *>(dest) = ntohs(*static_cast<const uint16_t *>(src));
    }
    template<>
    void big_endian_to_native<4>(const void *src, void *dest) {
        *static_cast<uint32_t *>(dest) = ntohl(*static_cast<const uint32_t *>(src));
    }
    template<>
    void big_endian_to_native<8>(const void *src, void *dest) {
        uint64_t native = 0;
        const uint8_t *big_endian = static_cast<const uint8_t *>(src);
        for (size_t i = 0; i < sizeof(native); i++) {
            native <<= 8;
            native |= big_endian[i];
        }
        memcpy(dest, &native, sizeof(native));
    }
}


template<size_t DataSize>
void cell::get_data(void *data) const {
    if (_bytes.size() != DataSize)  throw retrieved_unexpected_size_exception(DataSize, _bytes.size());

    big_endian_to_native<DataSize>(&_bytes[0], data);
}
template void cell::get_data<1>(void *) const;
template void cell::get_data<2>(void *) const;
template void cell::get_data<4>(void *) const;
template void cell::get_data<8>(void *) const;


template<size_t DataSize>
void cell::set_data(const void *data) {
    _bytes.resize(DataSize);
    native_to_big_endian<DataSize>(data, &_bytes[0]);
    _is_binary = true;
}
template void cell::set_data<1>(const void *);
template void cell::set_data<2>(const void *);
template void cell::set_data<4>(const void *);
template void cell::set_data<8>(const void *);


void cell::get_string(string &string) const {
    string.assign(chars(), size());
}

void cell::set_string(const string &string) {
    const uint8_t * const base = reinterpret_cast<const uint8_t *>(base_address(string));
    const size_t size = string.size();
    _bytes.assign(base, base + size);
    _is_binary = false;
}

}