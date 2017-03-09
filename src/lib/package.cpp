#include <stdlib.h>
#include <string>
#include <vector>

#include "package.hpp"

#define PACKAGE_HEADER "AINCRAD_PACKAGE"

namespace network {

Package::Package() : content( NULL ), size( 0 ), buffer( NULL ){};

Package::Package( string _content )
    : content( _content ), size( 0 ), buffer( NULL ){};

Package::~Package() {
    if ( buffer ) free( buffer );
};

// HEADER | SIZE(4 bytes) | CONTENT
/*
 * Get the buffer, recognize the first 'package' and put it into
 * the package. return the size of package
 * _buffer: the buffer received
 * _size: the size of the received buffer
 */

size_t Package::decrypt( char* _buffer, size_t _size ) {
    (void)_buffer;
    int head_tag_size = sizeof( PACKAGE_HEADER ) + 4;
    if ( size < head_tag_size ) {
        return 0;
    }

    int package_size = *(
        int*)( buffer + sizeof( PACKAGE_HEADER ) );  // get the size of package
    char* package_start = buffer + head_tag_size;

    content.copy( package_start, package_size, 0 );

    return ( size_t )( package_size + head_tag_size );  // buffer size
};

// HEADER | SIZE(4 bytes) | CONTENT
// encrypt the content as the above format

Package& Package::encrypt() {
    buffer = (char*)malloc( 15 + 4 + content.length() );
    strncpy( buffer, PACKAGE_HEADER, 15 );

    int* size_begin = (int*)( (char*)buffer + 15 );
    *size_begin     = (int)size;

    buffer[19] = '\0';
    strncat( buffer, content, content.length() );

    return *shared_from_this();
};

char* Package::data() {
    return buffer;
};

size_t Package::length() {
    return size;
}
};
