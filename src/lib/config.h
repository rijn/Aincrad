#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "util.h"

#include <map>
#include <string>

namespace util {

class config {
   private:
    std::map<std::string, std::string> content_;

   public:
    bool read_config( std::string const& configFile );

    std::string const& value( std::string const& section,
                              std::string const& entry ) const;
    bool exist( std::string const& section, std::string const& entry ) const;
};
}

#endif
