#pragma once

#include <string>
#include <vector>

#define PACKAGE_HEADER "AINCRAD_PACKAGE"

namespace network {

using std::string;

/*
 *class Package : public std::enable_shared_from_this<Package> {
 */
class Package { 
   public:
    Package();
    Package( string _content );
    ~Package();

    size_t decrypt( char* buffer, size_t size );
    void encrypt();
    char*    data();
    size_t   length();

   private:
    string content;
    size_t size;
    char*  buffer;
};
}
