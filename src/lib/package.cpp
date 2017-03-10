#if 0
#include <stdlib.h>
#include <cstdlib>
#include <string>
#include <vector>

#include "package.hpp"

#define PACKAGE_HEADER "AINCRAD_PACKAGE"
#define SIZE_LEN 4

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
    size_t header_size   = sizeof( PACKAGE_HEADER ) - 1;  // remove the last nul
    size_t int_size      = sizeof( int );
    size_t head_tag_size = header_size + int_size;
    if ( size < head_tag_size ) {
        return 0;
    }

    int package_size =
        *(int*)( buffer + header_size );  // get the size of package
    char* package_start = buffer + head_tag_size;

    content.copy( package_start, package_size, 0 );

    return ( size_t )( package_size + head_tag_size );  // buffer size
};

// HEADER | SIZE(4 bytes) | CONTENT
// encrypt the content as the above format

void Package::encrypt() {
    size_t header_size = sizeof( PACKAGE_HEADER ) - 1;
    size_t int_size    = sizeof( int );
    size               = header_size + int_size + content.length();
    buffer             = (char*)realloc( buffer, size );
    strncpy( buffer, PACKAGE_HEADER, header_size );

    int* size_begin = (int*)( (char*)buffer + header_size );
    *size_begin     = (int)content.length();

    buffer[header_size + int_size] = '\0';
    strncat( buffer, content.c_str(), content.length() );
};

char* Package::data() {
    return buffer;
};

size_t Package::length() {
    return size;
}
};
#endif
