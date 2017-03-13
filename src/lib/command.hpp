#ifndef __COMMAND__
#define __COMMAND__

#include <iostream>
#include <string>
#include <vector>

using std::vector;
using std::string;
using std::cout;
using std::endl;

#include "server.hpp"
#include "util.h"

class Command {
   public:
    Command( string line ) {
        argv = util::split( line, ' ' );
    };

    void process() {
        return;
    };

   private:
    vector<string> argv;
};

// register command processor
void register_processor( network::server_ptr server ) {
    server->on( "recv_package", []( network::package_ptr package,
                                    network::session_ptr session,
                                    network::server_ptr  server ) {
        /*
         *server->broadcast( package, []( auto ) { return true; } );
         */
        Command c( string( package->body(), package->body_length() ) );
        c.process();
        (void)package;
        (void)session;
        (void)server;
    } );

    return;
}

#endif
