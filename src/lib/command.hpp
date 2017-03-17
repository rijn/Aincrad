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

#include "client.hpp"
#include "server.hpp"
#include "util.h"

class Operate {
   public:
    static void self( std::vector<std::string>& argv, std::string hostname ) {
        int level = 0;
        for ( auto it = argv.begin(); it != argv.end(); ++it ) {
            if ( *it == "<" ) {
                if (level == 0) it = argv.erase( it );
                ++level;
            } else if ( *it == ">" ) {
                --level;
                if (level == 0) it = argv.erase( it );
            } else if ( *it == "this" && !level ) {
                *it = hostname;
            }
            if ( it == argv.end() ) break;
        }
    }

    static void process( std::string line, network::package_ptr package,
                         network::session_ptr session,
                         network::server_ptr  server,
                         network::client_ptr  client ) {
        auto argv = util::split( line, ' ' );
        if ( session == NULL ) self( argv, client->hostname() );
        if ( fn_map.find( argv[0] ) == fn_map.end() ) {
            if ( session == NULL )
                client->send(
                    std::make_shared<network::Package>( "command error" ) );
            else
                session->send(
                    std::make_shared<network::Package>( "command error" ) );
        } else {
            fn_map[argv[0]]( line, argv, package, session, server, client );
        }
        return;
    };

    static void to( std::string, std::vector<std::string>      argv,
                    network::package_ptr, network::session_ptr session,
                    network::server_ptr server, network::client_ptr client ) {
        auto p = std::make_shared<network::Package>( std::accumulate(
            argv.begin() + ( session == NULL ? 0 : 2 ), argv.end(),
            string( "" ), []( const string& s1, const string& s2 ) -> string {
                return s1.empty() ? s2 : s1 + " " + s2;
            } ) );
        if ( session != NULL )
            server->sent_to( p, argv[1] );
        else
            client->send( p );
    }

    static void reg( std::string, std::vector<std::string>      argv,
                     network::package_ptr, network::session_ptr session,
                     network::server_ptr, network::client_ptr ) {
        session->hostname = argv[1];
    }

    static void print( std::string, std::vector<std::string> argv,
                       network::package_ptr, network::session_ptr,
                       network::server_ptr, network::client_ptr ) {
        std::cout << std::accumulate(
                         argv.begin() + 1, argv.end(), string( "" ),
                         []( const string& s1, const string& s2 ) -> string {
                             return s1.empty() ? s2 : s1 + " " + s2;
                         } )
                  << std::endl;
    }

    static void echo( std::string, std::vector<std::string>      argv,
                      network::package_ptr, network::session_ptr session,
                      network::server_ptr, network::client_ptr   client ) {
        auto p = std::make_shared<network::Package>( std::accumulate(
            argv.begin() + 1, argv.end(), string( "" ),
            []( const string& s1, const string& s2 ) -> string {
                return s1.empty() ? s2 : s1 + " " + s2;
            } ) );
        client->send( p );
    }

    static void set_hostname( std::string, std::vector<std::string> argv,
                              network::package_ptr, network::session_ptr,
                              network::server_ptr,
                              network::client_ptr client ) {
        client->set_hostname( argv[1] );
        client->send(
            std::make_shared<network::Package>( "reg " + client->hostname() ) );
    }

   private:
    typedef std::map<
        std::string,
        std::function<void( std::string, std::vector<std::string>,
                            network::package_ptr, network::session_ptr,
                            network::server_ptr, network::client_ptr )>>
                 FnMap;
    static FnMap fn_map;
};

Operate::FnMap Operate::fn_map = {{"echo", &Operate::echo},
                                  {"reg", &Operate::reg},
                                  {"to", &Operate::to},
                                  {"set_hostname", &Operate::set_hostname},
                                  {"print", &Operate::print}};

// register command processor
void register_processor( network::server_ptr server,
                         network::client_ptr client ) {
    if ( server )
        server->on( "recv_package", []( network::package_ptr package,
                                        network::session_ptr session,
                                        network::server_ptr  server ) {
            Operate::process( string( package->body(), package->body_length() ),
                              package, session, server, NULL );
        } );

    if ( client )
        client->on( "recv_package", []( network::package_ptr package,
                                        network::client_ptr client ) {
            Operate::process( string( package->body(), package->body_length() ),
                              package, NULL, NULL, client );
        } );
    return;
}

#endif
