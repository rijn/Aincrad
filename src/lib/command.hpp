#ifndef __COMMAND__
#define __COMMAND__

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/any.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <regex>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#include "client.hpp"
#include "config.h"
#include "package.hpp"
#include "server.hpp"
#include "util.h"

#include "editor.h"

using std::vector;
using std::string;
using std::cout;
using std::endl;

namespace fs = boost::filesystem;

namespace boost {
namespace filesystem {

static fs::path relativeTo( fs::path from, fs::path to ) {
    fs::path::const_iterator fromIter = from.begin();
    fs::path::const_iterator toIter   = to.begin();

    while ( fromIter != from.end() && toIter != to.end() &&
            ( *toIter ) == ( *fromIter ) ) {
        ++toIter;
        ++fromIter;
    }

    fs::path finalPath;
    while ( fromIter != from.end() ) {
        finalPath /= "..";
        ++fromIter;
    }

    while ( toIter != to.end() ) {
        finalPath /= *toIter;
        ++toIter;
    }

    return finalPath;
}
}
}

static std::string script_dir( "" );

class Operate {
   public:
    struct wrapped {
        wrapped( std::deque<std::string> _astack,
                 std::deque<std::string> _vstack, network::package_ptr _package,
                 network::session_ptr _session, network::server_ptr _server,
                 network::client_ptr _client, Editor* _editor )
            : astack( _astack ),
              vstack( _vstack ),
              ostack(),
              package( _package ),
              session( _session ),
              server( _server ),
              client( _client ),
              editor( _editor ){};

        std::deque<std::string> astack;
        std::deque<std::string> vstack;
        std::deque<std::string> ostack;
        network::package_ptr    package;
        network::session_ptr    session;
        network::server_ptr     server;
        network::client_ptr     client;
        Editor*                 editor;
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
            }
        }
    };

    static void _const( std::vector<std::string>& argv, network::package_ptr,
                        network::session_ptr, network::server_ptr,
                        network::client_ptr client ) {
        int level = 0;
        for ( auto it = argv.begin(); it != argv.end(); ++it ) {
            if ( *it == "[" ) {
                if ( level == 0 ) it = argv.erase( it );
                ++level;
            } else if ( *it == "]" ) {
                --level;
                if ( level == 0 ) it = argv.erase( it );
            } else if ( *it == "this" && !level ) {
                *it = client->hostname();
            }
            if ( it == argv.end() ) break;
        }
    }

    static std::deque<std::string> process( std::string          line,
                                            network::package_ptr package,
                                            network::session_ptr session,
                                            network::server_ptr  server,
                                            network::client_ptr  client,
                                            Editor*              editor ) {
        auto argv = util::split( line, '$' );
        _const( argv, package, session, server, client );
        wrapped w( std::deque<std::string>(), std::deque<std::string>(),
                   package, session, server, client, editor );
        for ( auto arg : argv ) {
            w.astack.push_back( arg );
        }
        next( w );
        return w.ostack;
    };

    static void output( wrapped& w ) {
        w.ostack.push_back( w.vstack.back() );
        w.vstack.pop_back();
    }

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
    }

    static void lwc( wrapped& w ) {
        auto s = w.vstack.back();
        boost::algorithm::to_lower( s );
        w.vstack.pop_back();
        w.vstack.push_back( s );
    }

    static void split( wrapped& w ) {
        auto s = w.vstack.back();
        w.vstack.pop_back();
        auto b = w.vstack.back();
        w.vstack.pop_back();
        std::regex e( b );
        string     res  = std::regex_replace( s, e, "$" );
        auto       argv = util::split( res, '$' );
        auto       it   = argv.begin();
        for ( ; it != argv.end(); it++ ) {
            w.vstack.push_back( *it );
        }
    }

    static void newline( wrapped& w ) {
        w.vstack.push_back( "\n" );
    }

    static void empty( wrapped& w ) {
        w.vstack.push_back( "" );
    }

    static void parse( wrapped& w ) {
        auto s = w.vstack.back();
        w.vstack.pop_back();
        string res  = util::easy_type( s );
        auto   argv = util::split( res, '$' );
        auto   it   = argv.begin();
        for ( ; it != argv.end(); it++ ) {
            w.astack.push_back( *it );
        }
    }

    static void upc( wrapped& w ) {
        std::string s = w.vstack.back();
        std::transform( s.begin(), s.end(), s.begin(),
                        []( unsigned char c ) { return std::toupper( c ); } );
        w.vstack.pop_back();
        w.vstack.push_back( s );
    }

    static void swap( wrapped& w ) {
        auto a = w.vstack.back();
        w.vstack.pop_back();
        auto b = w.vstack.back();
        w.vstack.pop_back();
        w.vstack.push_back( a );
        w.vstack.push_back( b );
    }

    static void size( wrapped& w ) {
        w.vstack.push_back( std::to_string( w.vstack.size() ) );
    }

    static void print( wrapped& w ) {
        // std::cout << std::accumulate(
        //                  w.vstack.begin(), w.vstack.end(), string( "" ),
        //                  []( const string& s1, const string& s2 ) -> string {
        //                      return s1.empty() ? s2 : s1 + " " + s2;
        //                  } )
        //           << std::endl;
        w.editor->status.print_filename( std::accumulate(
            w.vstack.begin(), w.vstack.end(), string( "" ),
            []( const string& s1, const string& s2 ) -> string {
                return s1.empty() ? s2 : s1 + " " + s2;
            } ) );
    }

    static void print_limit( wrapped& w ) {
        auto n = std::atoi( w.vstack.back().c_str() );
        w.vstack.pop_back();
        std::string p;
        for ( auto it = w.vstack.rbegin(); it != w.vstack.rend() && n > 0;
              ++it, --n ) {
            p = *it + " " + p;
        }
        w.editor->status.print_filename(p);
    }

    static void drop( wrapped& w ) {
        auto n = std::atoi( w.vstack.back().c_str() );
        w.vstack.pop_back();
        while ( n-- > 0 ) {
            w.vstack.pop_back();
        }
    }

    static void drop_one( wrapped& w ) {
        w.vstack.push_back( "1" );
        drop( w );
    }

    static void _if( wrapped& w ) {
        bool                    _else_part = false;
        std::deque<std::string> t_deque;
        std::deque<std::string> f_deque;

        int level = 1;

        while ( level > 0 ) {
            if ( w.astack.back() == "if" ) ++level;
            if ( w.astack.back() == "then" ) --level;
            if ( level == 0 ) break;
            if ( level == 1 && w.astack.back() == "else" ) {
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
    }

    static void begin( wrapped& w ) {
        std::deque<std::string> inner;

        auto it = w.astack.rbegin();
        int  c  = 1;

        while ( c > 0 ) {
            if ( *it == "begin" ) ++c;
            if ( *it == "end" ) --c;
            if ( c == 0 ) break;
            inner.push_back( *it );
            ++it;
        }

        w.astack.push_back( "begin" );

        while ( !inner.empty() ) {
            w.astack.push_back( inner.back() );
            inner.pop_back();
        }
    }

    static void exit( wrapped& w ) {
        while ( !w.astack.empty() && w.astack.back() != "begin" ) {
            w.astack.pop_back();
        }
        if ( !w.astack.empty() && w.astack.back() == "begin" ) {
            w.astack.pop_back();
        }
        int level = 1;
        while ( !w.astack.empty() && level > 0 ) {
            if ( w.astack.back() == "begin" ) ++level;
            if ( w.astack.back() == "end" ) --level;
            w.astack.pop_back();
        }
    }

    static void to( wrapped& w ) {
        auto hostname = w.vstack.back();
        w.vstack.pop_back();
        auto p = _pack( w );
        w.server->sent_to( std::make_shared<network::Package>( p ), hostname );
        w.astack.clear();
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
    }

    static void minus( wrapped& w ) {
        long a = std::stol( w.vstack.back() );
        w.vstack.pop_back();
        long b = std::stol( w.vstack.back() );
        w.vstack.pop_back();
        w.vstack.push_back( std::to_string( b - a ) );
    }

    static void add( wrapped& w ) {
        long a = std::stol( w.vstack.back() );
        w.vstack.pop_back();
        long b = std::stol( w.vstack.back() );
        w.vstack.pop_back();
        w.vstack.push_back( std::to_string( a + b ) );
    }

    static void greater( wrapped& w ) {
        long a = std::stol( w.vstack.back() );
        w.vstack.pop_back();
        long b = std::stol( w.vstack.back() );
        w.vstack.pop_back();
        w.vstack.push_back( b > a ? "1" : "0" );
    }

    static void equal( wrapped& w ) {
        auto a = w.vstack.back();
        w.vstack.pop_back();
        auto b = w.vstack.back();
        w.vstack.pop_back();
        w.vstack.push_back( b == a ? "1" : "0" );
    }

    static void sadd( wrapped& w ) {
        auto a = w.vstack.back();
        w.vstack.pop_back();
        auto b = w.vstack.back();
        w.vstack.pop_back();
        w.vstack.push_back( a + b );
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
    }

    static void push_host( wrapped& w ) {
        if ( w.server == nullptr ) return;
        for ( auto it = w.server->get_clients().begin();
              it != w.server->get_clients().end(); ++it ) {
            w.vstack.push_back( ( *it )->hostname );
        }
    }

    static void reg( wrapped& w ) {
        w.session->hostname = w.vstack.back();
        w.vstack.pop_back();
    }

    static void forward( wrapped& w ) {
        auto p = _pack( w );
        if ( w.client )
            w.client->send( std::make_shared<network::Package>( p ) );
        w.astack.clear();
    }

    static void system( wrapped& w ) {
        auto command = w.vstack.back();
        w.vstack.pop_back();

        auto output = util::exec( command.c_str(), false );
        w.vstack.push_back( output );
    }

    static void set_hostname( wrapped& w ) {
        w.client->set_hostname( w.vstack.back() );
        w.vstack.pop_back();
        w.client->send( std::make_shared<network::Package>(
            "reg$" + w.client->hostname() ) );
    }

    static void hostname( wrapped& w ) {
        w.vstack.push_back( w.client ? w.client->hostname() : "server" );
    }

    static void run( wrapped& w ) {
        auto filename = w.vstack.back();
        w.vstack.pop_back();

        auto path = fs::path( script_dir + filename );

        if ( !fs::exists( path ) ) {
            return;
        }

        fs::ifstream file;
        file.open( path );
        string str;
        string command;

        std::map<string, string> var;

        while ( getline( file, str ) ) {
            if ( str.length() == 0 ) {
                if ( !command.empty() ) {
                    auto _ostack = Operate::process(
                        _pack( w ) + "$promise$" + command, w.package,
                        w.session, w.server, w.client, w.editor );
                }
                command = "";
                continue;
            }
            if ( str[0] == '#' ) {
                var[str.substr( 1 )] = w.vstack.back();
                w.vstack.pop_back();
                continue;
            }
            if ( var.find( str ) != var.end() ) {
                str = var[str];
            }
            command = str + ( command.empty() ? "" : "$" ) + command;
        }

        w.astack.clear();
    }

    static void promise( wrapped& w ) {
        w.vstack.clear();
        while ( !w.astack.empty() &&
                fn_map.find( w.astack.back() ) == fn_map.end() ) {
            w.vstack.push_back( w.astack.back() );
            w.astack.pop_back();
        }
        while ( !w.ostack.empty() ) {
            w.vstack.push_back( w.ostack.front() );
            w.ostack.pop_front();
        }
    }

    static void s_list_host( wrapped& w ) {
        w.astack.clear();
        w.vstack.clear();
        w.astack.push_back( "print" );
        w.astack.push_back( "->" );
        w.astack.push_back( w.client->hostname() );
        w.astack.push_back( "list_host" );
        w.astack.push_back( "->>" );
    }

    static void s_ping( wrapped& w ) {
        auto client_hostname = w.vstack.back();
        w.vstack.clear();
        w.astack.clear();
        w.astack.push_back( "print" );
        w.astack.push_back( "connected" );
        w.astack.push_back( "[" );
        w.astack.push_back( "this" );
        w.astack.push_back( "]" );
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
    }

    static void sft( wrapped& w ) {
        auto filename = w.vstack.back();
        w.vstack.pop_back();

        try {
            if ( !fs::exists( fs::path( filename ) ) ) {
                return;
            }
            auto file =
                std::make_shared<boost::iostreams::mapped_file_source>();
            if ( fs::file_size( fs::path( filename ) ) > 0 ) {
                file->open( filename );
            }
            if ( w.server ) {
                auto hostname = w.vstack.back();
                w.vstack.pop_back();
                w.server->sent_to( std::make_shared<network::Package>( file ),
                                   hostname );
            } else {
                w.client->send( std::make_shared<network::Package>( file ) );
            }
            if ( file->is_open() ) file->close();
        } catch ( const std::exception& ex ) {
            std::cout << ex.what() << std::endl;
        }
    }

    static void popfs( wrapped& w ) {
        fs::path full_path( fs::initial_path<fs::path>() );
        full_path = fs::system_complete( fs::path( TEMP_PATH ) );
        if ( !fs::exists( full_path ) ) {
            fs::create_directory( full_path );
        }
        fs::directory_iterator end_iter;
        size_t                 file_count = 0;
        for ( fs::directory_iterator dir_itr( full_path ); dir_itr != end_iter;
              ++dir_itr ) {
            if ( fs::is_regular_file( dir_itr->status() ) ) {
                if ( util::is_number( dir_itr->path().filename().string() ) ) {
                    ++file_count;
                }
            }
        }

        --file_count;

        auto filename = w.vstack.back();
        w.vstack.pop_back();

        try {
            fs::path target( fs::system_complete( fs::path( filename ) ) );
            fs::path accu( "/" );
            fs::path::iterator it( target.begin() ), it_end( target.end() );
            ++it;
            --it_end;
            for ( ; it != it_end; ++it ) {
                accu /= it->filename().string();
                if ( !fs::exists( accu ) ) {
                    fs::create_directory( accu );
                }
            }

            fs::rename(
                fs::system_complete( fs::path( "./" + TEMP_PATH + "/" +
                                               std::to_string( file_count ) ) ),
                fs::system_complete( fs::path( filename ) ) );
        } catch ( const std::exception& ex ) {
            std::cout << ex.what() << std::endl;
        }
    }

    static void tree( wrapped& w ) {
        auto dir = w.vstack.back();
        w.vstack.pop_back();

        fs::path full_path( fs::initial_path<fs::path>() );
        full_path = fs::system_complete( fs::path( dir ) );
        if ( !fs::exists( full_path ) ) {
            return;
        }

        if ( !fs::is_directory( full_path ) ) {
            return;
        }

        fs::recursive_directory_iterator end_iter;
        for ( fs::recursive_directory_iterator dir_itr( full_path );
              dir_itr != end_iter; ++dir_itr ) {
            if ( fs::is_regular_file( dir_itr->status() ) ) {
                w.vstack.push_back(
                    fs::relativeTo( dir_itr->path(), full_path ).string() );
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
                                  {"size", &Operate::size},
                                  {"print", &Operate::print},
                                  {"print_limit", &Operate::print_limit},
                                  {"drop_one", &Operate::drop_one},
                                  {"drop", &Operate::drop},
                                  {"lwc", &Operate::lwc},
                                  {"upc", &Operate::upc},
                                  {"split", &Operate::split},
                                  {"newline", &Operate::newline},
                                  {"\\n", &Operate::newline},
                                  {"_", &Operate::empty},
                                  {"parse", &Operate::parse},
                                  {"*", &Operate::output},
                                  {"promise", &Operate::promise},
                                  // archimatic operation
                                  {"-", &Operate::minus},
                                  {"+", &Operate::add},
                                  {">", &Operate::greater},
                                  {"==", &Operate::equal},
                                  {"++", &Operate::sadd},
                                  // cond operation
                                  {"if", &Operate::_if},
                                  {"begin", &Operate::begin},
                                  {"exit", &Operate::exit},
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
                                  {"hostname", &Operate::hostname},
                                  {"list_host", &Operate::list_host},
                                  {"push_host", &Operate::push_host},
                                  {"run", &Operate::run},
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
void register_processor( network::server_ptr server, network::client_ptr client,
                         Editor* editor ) {
    if ( script_dir == "" ) {
        util::config _conf_remote;
        _conf_remote.read_config( util::get_working_path() + "/.config" );
        script_dir = _conf_remote.value( "script", "dir" );
    }

    if ( server )
        server->on( "recv_package", [editor]( network::package_ptr package,
                                              network::session_ptr session,
                                              network::server_ptr  server ) {
            Operate::process( string( package->body(), package->body_length() ),
                              package, session, server, NULL, editor );
        } );

    if ( client )
        client->on( "recv_package", [editor]( network::package_ptr package,
                                              network::client_ptr client ) {
            Operate::process( string( package->body(), package->body_length() ),
                              package, NULL, NULL, client, editor );
        } );
    return;
}

#endif
