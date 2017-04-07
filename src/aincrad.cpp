#include "aincrad.h"
#include <unistd.h>
#include <boost/asio.hpp>
#include <csignal>
#include <thread>
#include "lib/client.cpp"
#include "lib/command.hpp"
#include "lib/package.hpp"
#include "lib/server.hpp"
#include "lib/util.h"

#include <ncurses.h>
#include <string>
#include "lib/editor.h"
#include "lib/window.h"

using boost::asio::ip::tcp;
using std::thread;

namespace aincrad {};

namespace opts {
bool svn = true;
};

int    max_row, max_col;
Editor editor;  // for windows info, etc.

int main( int argc, char* argv[] ) {
    // cout << colorize::make_color( colorize::LIGHT_BLUE, AINCRAD ) << endl;

    // ----------- init ncurses ----------------
    initscr();               // setup ncurses screen
    raw();                   // enable raw mode so we can capture ctrl+c, etc.
    keypad( stdscr, true );  // to capture arrow key, etc.
    noecho();                // so that escape characters won't be printed
    getmaxyx( stdscr, max_row, max_col );  // get max windows size
    std::signal( SIGSEGV, segfault_handler );
    std::signal( SIGINT, segfault_handler );

    // -------------- done init ----------------

    run_editor();

    return aincrad_main( argc, argv );
    // return 0;
}

int aincrad_main( int argc, char* argv[] ) {
    /* error handler */
    /*
     *std::set_terminate( error_handler );
     */

    /* getting workpath */
    std::string working_path = get_working_path();
#ifdef DEBUG
    cout << "working path " << working_path << endl;
#endif

    /* parser arguments */
    arguments _arg;
    if ( !_arg.process_arguments( argc, argv ) ) return ( EXIT_FAILURE );

    /* check environment */
    config _conf_remote;

    if ( !_conf_remote.read_config( working_path + "/.config" ) ) {
        exit_with_error( "Cannot find config file" );
    }

    string role                      = _conf_remote.value( "basic", "role" );
    if ( _arg.exist( "role" ) ) role = _arg.value( "role" );

    // cout << "role = " << role << endl;
    editor.status.print_filename( "role = " + role );

    try {
        if ( role == "server" ) {
            boost::asio::io_service io_service;
            tcp::endpoint           endpoint( tcp::v4(), std::atoi( "8888" ) );
            auto s = std::make_shared<network::Server>( io_service, endpoint );
            s->start();

            register_processor( s, NULL, editor );

            io_service.run();

            // for ( ;; ) {
            //     if ( wgetch( stdscr ) == KEY_CTRL_C ) {
            //         endwin();
            //         std::exit( 0 );
            //         break;
            //     }
            // }
        }

        if ( role == "client" ) {
            string addr = _conf_remote.value( "server", "addr" );
            string port = _conf_remote.value( "server", "port" );

            cout << "Server " << addr << ":" << port << endl;

            boost::asio::io_service io_service;

            tcp::resolver resolver( io_service );
            auto          endpoint_iterator = resolver.resolve( {addr, port} );
            auto          c = std::make_shared<network::Client>( io_service,
                                                        endpoint_iterator );
            c->set_hostname( util::get_hostname() );
            std::cout << "Hostname " << c->hostname();

            // register event handler
            register_processor( NULL, c, editor );
            c->on( "connect",
                   []( network::package_ptr, network::client_ptr client ) {
                       client->send( std::make_shared<network::Package>(
                           "reg$" + client->hostname() ) );
                   } );

            std::thread t( [&io_service]() { io_service.run(); } );

            while ( 1 ) sleep( 1 );

            c->close();
            t.join();
        }

        if ( role == "terminal" ) {
            string addr = _conf_remote.value( "server", "addr" );
            string port = _conf_remote.value( "server", "port" );

            if ( _arg.exist( "server" ) ) addr = _arg.value( "server" );

            // cout << "Server " << addr << ":" << port << endl;
            editor.status.print_filename( "Server " + addr + ":" + port );

            boost::asio::io_service io_service;

            tcp::resolver resolver( io_service );
            auto          endpoint_iterator = resolver.resolve( {addr, port} );
            auto          c = std::make_shared<network::Client>( io_service,
                                                        endpoint_iterator );
            c->set_hostname( util::get_hostname() );
            // std::cout << "Hostname " << c->hostname() << std::endl;
            editor.status.print_filename( "Hostname " + c->hostname() );

            register_processor( NULL, c, editor );
            c->on( "connect",
                   []( network::package_ptr, network::client_ptr client ) {
                       client->send( std::make_shared<network::Package>(
                           "reg$" + client->hostname() ) );
                   } );

            std::thread t( [&io_service]() { io_service.run(); } );

            // char line[network::Package::max_body_length + 1];
            std::string line;
            while ( wgetline( editor.file, line,
                              network::Package::max_body_length + 1 ) ) {
                Operate::process( line, NULL, NULL, NULL, c, editor );
            }

            c->close();
            t.join();
        }
    } catch ( std::exception& e ) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

void segfault_handler( int sig ) {
    // so that ncurses won't mess up our screen
    endwin();
}

void run_editor() {
    int y, x;
    getyx( stdscr, y, x );
    editor.init( max_row, max_col );
    erase();
    refresh();
    editor.status.print_aincrad();
    editor.file.printline( "> " );
}

bool wgetline( WINDOW* w, string& s, size_t n ) {
    s.clear();
    int orig_y, orig_x;
    getyx( w, orig_y, orig_x );
    int curr;  // current character to read
    while ( !n || s.size() != n ) {
        curr = wgetch( w );
        if ( std::isprint( curr ) ) {
            if ( ++orig_x <= max_col ) {
                s.insert( s.begin() + ( orig_x - 3 ), curr );
                wmove( w, 0, 2 );
                wclrtoeol( w );
                waddnstr( w, s.c_str(), max_col );
                wmove( w, orig_y, orig_x );
                wrefresh( w );
            }

        } else if ( !s.empty() && ( orig_x > 2 ) &&
                    ( curr == KEY_BACKSPACE || curr == KEY_DC ||
                      curr == KEY_DELETE || curr == '\b' ) ) {
            if ( --orig_x <= max_col ) {
                mvwdelch( w, orig_y, orig_x );
                wrefresh( w );
                s.erase( s.begin() + orig_x - 2 );
            }

        } else if ( curr == KEY_ENTER || curr == '\n' ) {
            if ( !s.empty() ) {
                editor.file.history.push_back( s );
                editor.file.vec_idx = editor.file.history.size();
            }
            wclrtoeol( w );  // erase current line
            editor.file.printline( "> " );
            return true;
        } else if ( curr == KEY_LEFT ) {
            if ( --orig_x < 2 ) {
                orig_x = 2;
            } else {
                wmove( w, orig_y, orig_x );
            }
        } else if ( curr == KEY_RIGHT ) {
            if ( ++orig_x > static_cast<int>( s.size() + 2 ) ) {
                orig_x -= 1;
            } else {
                wmove( w, orig_y, orig_x );
            }
        } else if ( curr == KEY_DOWN ) {
            ++editor.file.vec_idx;
            if ( editor.file.vec_idx >= (int)editor.file.history.size() ) {
                editor.file.vec_idx = editor.file.history.size();
                s                   = "";
            } else {
                int idx = editor.file.vec_idx;
                s       = editor.file.history[idx];
            }
            editor.file.printline( "> " + s );
        } else if ( curr == KEY_UP ) {
            --editor.file.vec_idx;
            if ( editor.file.vec_idx < 0 ) editor.file.vec_idx = 0;
            int idx = editor.file.vec_idx;
            s       = editor.file.history[idx];
            editor.file.printline( "> " + s );
        } else if ( curr == ERR ) {
            if ( s.empty() ) return false;
            return true;
        } else if ( curr == KEY_CTRL_Q || curr == KEY_CTRL_C ) {
            endwin();
            std::exit( 0 );
            // return false;
        }
    }
    return true;
}