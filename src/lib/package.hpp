#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

#define PACKAGE_HEADER AINCRAD_PACKAGE

namespace network {

/*
 * package construction
 * | HEADER 15 bytes | LENGTH 8 bytes | TYPE 4 bytes | CONTENT |
 */

class Package : public std::enable_shared_from_this<Package> {
   public:
    enum { header_length = 15 };
    enum { size_length = 8 };
    enum { _COMMAND = 1, _FILE = 2 };
    enum { max_body_length = 1024 };

    static uint8_t header_len() {
        return header_length + size_length + 4;
    }

    Package() : _body_length( 0 ) {
        _data =
            (char*)malloc( header_length + size_length + 4 + max_body_length );
        _type = _COMMAND;
    }

    Package( std::string s ) {
        size_t s_len = s.length();

        _data        = (char*)malloc( header_length + size_length + 4 + s_len );
        _body_length = s_len;
        _type        = _COMMAND;
        encrypt();
        std::memcpy( body(), s.c_str(), _body_length );
    }

    const char* data() const {
        return _data;
    }

    char* data() {
        return _data;
    }

    bool is_command() {
        return _type == _COMMAND;
    }

    std::uint16_t length() const {
        return header_length + size_length + 4 + _body_length;
    }

    const char* body() const {
        return _data + header_length + size_length + 4;
    }

    char* body() {
        return _data + header_length + size_length + 4;
    }

    std::uint16_t body_length() const {
        return _body_length;
    }

    void body_length( std::uint16_t new_length ) {
        _body_length                                       = new_length;
        if ( _body_length > max_body_length ) _body_length = max_body_length;
    }

    bool decrypt() {
        char size[size_length + 1] = "";
        std::strncat( size, _data + header_length, size_length );
        _body_length     = std::strtoul( size, nullptr, 0 );
        char type[4 + 1] = "";
        std::strncat( type, _data + header_length + size_length, 4 );
        _type = std::atoi( type );
        _data = (char*)realloc(
            _data, header_length + size_length + 4 + _body_length );
        return true;
    }

    void encrypt() {
        char header[header_length + size_length + 4 + 1] = "";
        std::sprintf( header, "PACKAGE_HEADER%8u%4d",
                      static_cast<uint16_t>( _body_length ),
                      static_cast<int>( _type ) );
        std::memcpy( _data, header, header_length + size_length + 4 );
    }

   private:
    char*         _data;  //[header_length + size_length + max_body_length];
    std::uint16_t _body_length;
    std::size_t   _type;
};

typedef std::shared_ptr<Package> package_ptr;
};
