#ifndef __ARGUMENTS_H__
#define __ARGUMENTS_H__

#include "optionparser.h"
#include "util.h"

#include <map>
#include <string>

namespace util {

class arguments {
   private:
    std::map<std::string, std::string> content_;

   public:
    bool process_arguments( int& argc, char**& argv );
    std::string const& value( std::string const& entry ) const;
    bool exist( std::string const& entry ) const;
};
};

#endif
