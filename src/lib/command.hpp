#ifndef __COMMAND__
#define __COMMAND__

#include <boost/any.hpp>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <vector>

using std::vector;
using std::string;
using std::cout;
using std::endl;

#include "server.hpp"
#include "util.h"

class Operate {
   public:
    static void process( std::string line, network::package_ptr package,
                         network::session_ptr session,
                         network::server_ptr  server ) {
        auto argv = util::split( line, ' ' );
        fn_map[argv[0]]( line, argv, package, session, server );
        return;
    };

    static void echo( std::string, std::vector<std::string> argv,
                      network::package_ptr package,
                      network::session_ptr session, network::server_ptr ) {
        network::package p(
            std::accumulate( argv.begin() + 1, argv.end(), string( " " ) ) );
        network::Package msg;
        msg.body_length( std::strlen( line ) );
        std::memcpy( msg.body(), line, msg.body_length() );
        msg.encrypt();
        session->send( p );
    }

   private:
    typedef std::map<
        std::string,
        std::function<void( std::string, std::vector<std::string>,
                            network::package_ptr, network::session_ptr,
                            network::server_ptr )>>
                 FnMap;
    static FnMap fn_map;
};

Operate::FnMap Operate::fn_map = {{"echo", &Operate::echo}};

// register command processor
void register_processor( network::server_ptr server ) {
    server->on( "recv_package", []( network::package_ptr package,
                                    network::session_ptr session,
                                    network::server_ptr  server ) {
        /*
         *server->broadcast( package, []( auto ) { return true; } );
         */
        Operate::process( string( package->body(), package->body_length() ),
                          package, session, server );
    } );

    return;
}

#endif
