#include <string>
#include <cstdlib>
#include <vector>

#include "package.hpp"

#define PACKAGE_HEADER "AINCRAD_PACKAGE"

namespace network {

Package::Package() : content( NULL ), size( 0 ), buffer( NULL ){};

Package::Package( string _content )
        : content( _content ), size( 0 ), buffer( NULL ){};

Package::~Package(){};

    size_t Package::decrypt( char* buffer ) {
        (void)buffer;
        return 0;
    };

    Package& Package::encrypt() {
        buffer = (char*)malloc(1);
        size = 1;
        buffer[0] = 'A';
        return *this;
    };

    char* Package::data() {
        return buffer;
    };

    size_t Package::length() {
        return size;
    }
}
