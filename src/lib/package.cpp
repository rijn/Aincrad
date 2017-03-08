#include <string>
#include <vector>

namespace network {

using std::string;

class Package {
   public:
    Package() {
        size    = 0;
        content = "";
    };

    Package( size_t _size, string _content )
        : size( _size ), content( _content ){};

    ~Package(){};

    size_t decrypt( char* buffer ) {
        (void)buffer;
    };

    char* encrypt() {
        return NULL;
    };

   private:
    size_t size;
    string content;
    char*  data;
};
}
