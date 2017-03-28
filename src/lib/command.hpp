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
#include "package.hpp"
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
        auto argv = util::split( line, '$' );
        _const( argv, package, session, server, client );
        wrapped w( std::deque<std::string>(), std::deque<std::string>(),
                   package, session, server, client );
        for ( auto arg : argv ) {
            w.astack.push_back( arg );
        }
        next( w );
    };

    static std::string _pack( wrapped& w ) {
        auto vp = std::accumulate(
            w.vstack.begin(), w.vstack.end(), string( "" ),
            [&]( const string& s1, const string& s2 ) -> string {
                return s1.empty() ? s2 : s2 + "$" + s1;
            } );
        auto ap = std::accumulate(
            w.astack.begin(), w.astack.end(), string( "" ),
            [&]( const string& s1, const string& s2 ) -> string {
                return s1.empty() ? s2 : s1 + "$" + s2;
            } );
        return ap + ( vp == "" ? "" : "$" ) + vp;
    }

    static void dup( wrapped& w ) {
        w.vstack.push_back( w.vstack.back() );
        next( w );
    }

    static void swap( wrapped& w ) {
        auto a = w.vstack.back();
        w.vstack.pop_back();
        auto b = w.vstack.back();
        w.vstack.pop_back();
        w.vstack.push_back( a );
        w.vstack.push_back( b );
        next( w );
    }

    static void size( wrapped& w ) {
        w.vstack.push_back( std::to_string( w.vstack.size() ) );
        next( w );
    }

    static void _if( wrapped& w ) {
        bool                    _else_part = false;
        std::deque<std::string> t_deque;
        std::deque<std::string> f_deque;

        while ( w.astack.back() != "then" ) {
            if ( w.astack.back() == "else" ) {
                _else_part = true;
            } else if ( _else_part ) {
                f_deque.push_back( w.astack.back() );
            } else {
                t_deque.push_back( w.astack.back() );
            }
            w.astack.pop_back();
        }
        w.astack.pop_back();

        auto cond = w.vstack.back();
        w.vstack.pop_back();
        if ( cond == "0" ) {
            // if false
            while ( !f_deque.empty() ) {
                w.astack.push_back( f_deque.back() );
                f_deque.pop_back();
            }
        } else {
            while ( !t_deque.empty() ) {
                w.astack.push_back( t_deque.back() );
                t_deque.pop_back();
            }
        }

        next( w );
    }

    static void to( wrapped& w ) {
        auto hostname = w.vstack.back();
        w.vstack.pop_back();
        auto p = _pack( w );
        w.server->sent_to( std::make_shared<network::Package>( p ), hostname );
    }

    static void broadcast( wrapped& w ) {
        auto block = w.vstack.back();
        w.vstack.pop_back();
        auto p = _pack( w );
        w.server->broadcast( std::make_shared<network::Package>( p ),
                             [&]( network::session_ptr session ) {
                                 return block != session->hostname;
                             } );
    }

    static void time( wrapped& w ) {
        w.vstack.push_back( util::get_time() );
        next( w );
    }

    static void minus( wrapped& w ) {
        long a = std::stol( w.vstack.back() );
        w.vstack.pop_back();
        long b = std::stol( w.vstack.back() );
        w.vstack.pop_back();
        w.vstack.push_back( std::to_string( b - a ) );
        next( w );
    }

    static void add( wrapped& w ) {
        long a = std::stol( w.vstack.back() );
        w.vstack.pop_back();
        long b = std::stol( w.vstack.back() );
        w.vstack.pop_back();
        w.vstack.push_back( std::to_string( a + b ) );
        next( w );
    }

    static void greater( wrapped& w ) {
        long a = std::stol( w.vstack.back() );
        w.vstack.pop_back();
        long b = std::stol( w.vstack.back() );
        w.vstack.pop_back();
        w.vstack.push_back( b > a ? "1" : "0" );
        next( w );
    }

    static void equal( wrapped& w ) {
        auto a = w.vstack.back();
        w.vstack.pop_back();
        auto b = w.vstack.back();
        w.vstack.pop_back();
        w.vstack.push_back( b == a ? "1" : "0" );
        next( w );
    }

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

    static void push_host( wrapped& w ) {
        if ( w.server == nullptr ) return;
        for ( auto it = w.server->get_clients().begin();
              it != w.server->get_clients().end(); ++it ) {
            w.vstack.push_back( ( *it )->hostname );
        }
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
        if ( w.client )
            w.client->send( std::make_shared<network::Package>( p ) );
    }

    static void system( wrapped& w ) {
        auto command = w.vstack.back();
        w.vstack.pop_back();

        auto output = util::exec( command.c_str(), false );
        w.vstack.push_back( output );

        next( w );
    }

    static void set_hostname( wrapped& w ) {
        w.client->set_hostname( w.vstack.back() );
        w.vstack.pop_back();
        w.client->send( std::make_shared<network::Package>(
            "reg$" + w.client->hostname() ) );
    }

    static void s_list_host( wrapped& w ) {
        w.astack.clear();
        w.vstack.clear();
        w.astack.push_back( "print" );
        w.astack.push_back( "->" );
        w.astack.push_back( w.client->hostname() );
        w.astack.push_back( "list_host" );
        w.astack.push_back( "->>" );
        next( w );
    }

    static void s_ping( wrapped& w ) {
        auto client_hostname = w.vstack.back();
        w.vstack.clear();
        w.astack.clear();
        w.astack.push_back( "print" );
        w.astack.push_back( "connected" );
        w.astack.push_back( "<" );
        w.astack.push_back( "this" );
        w.astack.push_back( ">" );
        w.astack.push_back( "ns" );
        w.astack.push_back( "-" );
        w.astack.push_back( "time" );
        w.astack.push_back( "->" );
        w.astack.push_back( w.client->hostname() );
        w.astack.push_back( "->>" );
        w.astack.push_back( "->" );
        w.astack.push_back( client_hostname );
        w.astack.push_back( "->>" );
        w.astack.push_back( "time" );
        next( w );
    }

    static void sft( wrapped& w ) {
        auto filename = w.vstack.back();
        w.vstack.pop_back();
        auto file = std::make_shared<boost::iostreams::mapped_file_source>();
        file->open( filename );
        if ( w.server ) {
            auto hostname = w.vstack.back();
            w.vstack.pop_back();
            w.server->sent_to( std::make_shared<network::Package>( file ),
                               hostname );
        } else {
            w.client->send( std::make_shared<network::Package>( file ) );
        }
        file->close();

        next( w );
    }

    static void popfs( wrapped& w ) {
        boost::filesystem::path full_path(
            boost::filesystem::initial_path<boost::filesystem::path>() );
        full_path = boost::filesystem::system_complete(
            boost::filesystem::path( TEMP_PATH ) );
        if ( !boost::filesystem::exists( full_path ) ) {
            boost::filesystem::create_directory( full_path );
        }
        boost::filesystem::directory_iterator end_iter;
        size_t                                file_count = 0;
        for ( boost::filesystem::directory_iterator dir_itr( full_path );
              dir_itr != end_iter; ++dir_itr ) {
            if ( boost::filesystem::is_regular_file( dir_itr->status() ) ) {
                if ( util::is_number( dir_itr->path().filename().string() ) ) {
                    ++file_count;
                }
            }
        }

        --file_count;

        auto filename = w.vstack.back();
        w.vstack.pop_back();

        try {
            boost::filesystem::rename(
                boost::filesystem::system_complete( boost::filesystem::path(
                    "./" + TEMP_PATH + "/" + std::to_string( file_count ) ) ),
                boost::filesystem::system_complete(
                    boost::filesystem::path( filename ) ) );
        } catch ( const std::exception& ex ) {
            std::cout << ex.what() << std::endl;
        }

        next( w );
    }

    static void tree( wrapped& w ) {
        auto dir = w.vstack.back();
        w.vstack.pop_back();

        boost::filesystem::path full_path(
            boost::filesystem::initial_path<boost::filesystem::path>() );
        full_path = boost::filesystem::system_complete(
            boost::filesystem::path( dir ) );
        if ( !boost::filesystem::exists( full_path ) ) {
            return;
        }

        boost::filesystem::recursive_directory_iterator end_iter;
        for ( boost::filesystem::recursive_directory_iterator dir_itr(
                  full_path );
              dir_itr != end_iter; ++dir_itr ) {
            if ( boost::filesystem::is_regular_file( dir_itr->status() ) ) {
                std::cout << boost::filesystem::relative( dir_itr->path(),
                                                          full_path )
                                 .string()
                          << std::endl;
            }
        }
    }

   private:
    typedef std::map<std::string, std::function<void( Operate::wrapped& )>>
                 FnMap;
    static FnMap fn_map;
};

Operate::FnMap Operate::fn_map = {{"dup", &Operate::dup},
                                  {"swap", &Operate::swap},
                                  {"print", &Operate::print},
                                  // archimatic operation
                                  {"-", &Operate::minus},
                                  {"+", &Operate::add},
                                  {">", &Operate::greater},
                                  {"==", &Operate::equal},
                                  // cond operation
                                  {"if", &Operate::_if},
                                  // network operation
                                  {"->>", &Operate::forward},
                                  {"forward", &Operate::forward},
                                  {"reg", &Operate::reg},
                                  {"->", &Operate::to},
                                  {"to", &Operate::to},
                                  {"system", &Operate::system},
                                  {"time", &Operate::time},
                                  {"broadcast", &Operate::broadcast},
                                  {"set_hostname", &Operate::set_hostname},
                                  {"list_host", &Operate::list_host},
                                  {"push_host", &Operate::push_host},
                                  // file stack operation
                                  {"tree", &Operate::tree},
                                  {"sft", &Operate::sft},
                                  {"sf", &Operate::sft},
                                  {"sendfile", &Operate::sft},
                                  {"popfs", &Operate::popfs},
                                  // sugar
                                  {"@list_host", &Operate::s_list_host},
                                  //{"@system", &Operate::s_system},
                                  {"@ping", &Operate::s_ping}};

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
