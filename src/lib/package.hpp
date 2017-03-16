#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

#define PACKAGE_HEADER AINCRAD_PACKAGE

namespace network {

class Package : public std::enable_shared_from_this<Package> {
   public:
    enum { header_length = 15 };
    enum { size_length = 4 };
    enum { max_body_length = 1024 };

    Package() : _body_length( 0 ) {
    }

    Package(std::string s) {
        
    }

    const char* data() const {
        return _data;
    }

    char* data() {
        return _data;
    }

    std::size_t length() const {
        return header_length + size_length + _body_length;
    }

    const char* body() const {
        return _data + header_length + size_length;
    }

    char* body() {
        return _data + header_length + size_length;
    }

    std::size_t body_length() const {
        return _body_length;
    }

    void body_length( std::size_t new_length ) {
        _body_length                                       = new_length;
        if ( _body_length > max_body_length ) _body_length = max_body_length;
    }

    bool decrypt() {
        char size[size_length + 1] = "";
        std::strncat( size, _data + header_length, size_length );
        _body_length = std::atoi( size );
        if ( _body_length > max_body_length ) {
            _body_length = 0;
            return false;
        }
        return true;
    }

    void encrypt() {
        char header[header_length + size_length + 1] = "";
        std::sprintf( header, "PACKAGE_HEADER%4d",
                      static_cast<int>( _body_length ) );
        std::memcpy( _data, header, header_length + size_length );
    }

   private:
    char        _data[header_length + size_length + max_body_length];
    std::size_t _body_length;
};

typedef std::shared_ptr<Package> package_ptr;
};
