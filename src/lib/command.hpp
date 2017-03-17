#ifndef __COMMAND__
#define __COMMAND__

#include <boost/any.hpp>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <stack>
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
    struct wrapped {
        wrapped( std::deque<std::string> _astack,
                 std::deque<std::string> _vstack, network::package_ptr _package,
                 network::session_ptr _session, network::server_ptr _server,
                 network::client_ptr _client )
            : astack( _astack ),
              vstack( _vstack ),
              package( _package ),
              session( _session ),
              server( _server ),
              client( _client ){};

        std::deque<std::string> astack;
        std::deque<std::string> vstack;
        network::package_ptr    package;
        network::session_ptr    session;
        network::server_ptr     server;
        network::client_ptr     client;
    };

    static void next( wrapped& w ) {
        while ( !w.astack.empty() ) {
            if ( fn_map.find( w.astack.back() ) == fn_map.end() ) {
                w.vstack.push_back( w.astack.back() );
                w.astack.pop_back();
            } else {
                auto command = w.astack.back();
                w.astack.pop_back();
                fn_map[command]( w );
                break;
            }
        }
    };

    static void _const( std::vector<std::string>& argv, network::package_ptr,
                        network::session_ptr, network::server_ptr,
                        network::client_ptr client ) {
        int level = 0;
        for ( auto it = argv.begin(); it != argv.end(); ++it ) {
            if ( *it == "<" ) {
                if ( level == 0 ) it = argv.erase( it );
                ++level;
            } else if ( *it == ">" ) {
                --level;
                if ( level == 0 ) it = argv.erase( it );
            } else if ( *it == "this" && !level ) {
                *it = client->hostname();
            }
            if ( it == argv.end() ) break;
        }
    }

    static void process( std::string line, network::package_ptr package,
                         network::session_ptr session,
                         network::server_ptr  server,
                         network::client_ptr  client ) {
        auto argv = util::split( line, ' ' );
        _const( argv, package, session, server, client );
        wrapped w( std::deque<std::string>(), std::deque<std::string>(),
                   package, session, server, client );
        for ( auto arg : argv ) {
            w.astack.push_back( arg );
        }
        next( w );
        /*
         *if ( argv.size() == 0 ) {
         *    return;
         *}
         *if ( session == NULL ) self( argv, client->hostname() );
         *if ( fn_map.find( argv[0] ) == fn_map.end() ) {
         *    if ( session == NULL )
         *        client->send(
         *            std::make_shared<network::Package>( "command error" ) );
         *        ;
         *    else
         *        session->send( std::make_shared<network::Package>(
         *            "print command error" ) );
         *} else {
         *    wrapped w( argv, package, session, server, client );
         *    next( w )
         *    fn_map[argv[0]]( line, argv, package, session, server, client );
         *}
         */
        return;
    };

    static std::string _pack( wrapped& w ) {
        auto p = std::accumulate(
            w.astack.begin(), w.astack.end(), string( "" ),
            []( const string& s1, const string& s2 ) -> string {
                return s1.empty() ? s2 : s1 + " " + s2;
            } );
        p = std::accumulate(
            w.vstack.begin(), w.vstack.end(), p,
            []( const string& s1, const string& s2 ) -> string {
                return s1.empty() ? s2 : s1 + " " + s2;
            } );
        return p;
    }

    static void to( wrapped& w ) {
        auto hostname = w.vstack.back();
        w.vstack.pop_back();
        auto p = _pack( w );
        w.server->sent_to( std::make_shared<network::Package>( p ), hostname );
    }

    /*
     *static void broadcast( std::string, std::vector<std::string>      argv,
     *                       network::package_ptr, network::session_ptr session,
     *                       network::server_ptr server, network::client_ptr ) {
     *    auto p = std::make_shared<network::Package>( std::accumulate(
     *        argv.begin() + 1, argv.end(), string( "" ),
     *        []( const string& s1, const string& s2 ) -> string {
     *            return s1.empty() ? s2 : s1 + " " + s2;
     *        } ) );
     *    server->broadcast( p, [&]( network::session_ptr current_session ) {
     *        if ( session == current_session ) return true;
     *        if ( session && current_session ) {
     *            return *session == *current_session;
     *        } else {
     *            return false;
     *        }
     *    } );
     *}
     */

    static void list_host( wrapped& w ) {
        if ( w.server == nullptr ) return;
        w.vstack.push_back( std::accumulate(
            w.server->get_clients().begin(), w.server->get_clients().end(),
            string( "" ),
            []( const string& s1, network::session_ptr s2 ) -> string {
                return s1.empty()
                           ? "[" + s2->get_client_s() + "] " + s2->hostname
                           : s1 + "\n[" + s2->get_client_s() + "] " +
                                 s2->hostname;
            } ) );
        next( w );
    }

    static void reg( wrapped& w ) {
        w.session->hostname = w.vstack.back();
        w.vstack.pop_back();
        next( w );
    }

    static void print( wrapped& w ) {
        std::cout << std::accumulate(
                         w.vstack.begin(), w.vstack.end(), string( "" ),
                         []( const string& s1, const string& s2 ) -> string {
                             return s1.empty() ? s2 : s1 + " " + s2;
                         } )
                  << std::endl;
        next( w );
    }

    static void forward( wrapped& w ) {
        auto p = _pack( w );
        w.client->send( std::make_shared<network::Package>( p ) );
    }

    static void system( wrapped& w ) {
        next( w );
    }

    static void set_hostname( wrapped& w ) {
        w.client->set_hostname( w.vstack.back() );
        w.vstack.pop_back();
        w.client->send( std::make_shared<network::Package>(
            "reg " + w.client->hostname() ) );
    }

   private:
    typedef std::map<std::string, std::function<void( Operate::wrapped& )>>
                 FnMap;
    static FnMap fn_map;
};

Operate::FnMap Operate::fn_map = {{"->>", &Operate::forward},
                                  {"forward", &Operate::forward},
                                  {"reg", &Operate::reg},
                                  {"->", &Operate::to},
                                  {"to", &Operate::to},
                                  /*
                                   *{"broadcast", &Operate::broadcast},
                                   */
                                  {"set_hostname", &Operate::set_hostname},
                                  {"list_host", &Operate::list_host},
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
