#include <string>
#include <vector>

#define PACKAGE_HEADER "AINCRAD_PACKAGE"

namespace network {

using std::string;

class Package : public std::enable_shared_from_this<Package> {
   public:
    Package() : content( NULL ), size( 0 ), buffer( NULL ){};

    Package( string _content )
        : content( _content ), size( 0 ), buffer( NULL ){};

    ~Package(){};

    size_t decrypt( char* buffer ) {
        (void)buffer;
        return 0;
    };

    Package& encrypt() {
        return *shared_from_this();
    };

    char* data() {
        return buffer;
    };

    size_t length() {
        return size;
    }

   private:
    string content;
    size_t size;
    char*  buffer;
};
}
