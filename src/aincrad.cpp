#include "aincrad.h"
#include <boost/asio.hpp>
#include <thread>
#include "lib/client.cpp"
#include "lib/command.hpp"
#include "lib/package.hpp"
#include "lib/server.hpp"
#include "lib/util.h"

using boost::asio::ip::tcp;
using std::thread;

namespace aincrad {};

namespace opts {
bool svn = true;
};

int main( int argc, char* argv[] ) {
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

    cout << colorize::make_color( colorize::LIGHT_BLUE, AINCRAD ) << endl;

    /* check environment */
    config _conf_remote;

    if ( !_conf_remote.read_config( working_path + "/.config" ) ) {
        exit_with_error( "Cannot find config file" );
    }

    string role                      = _conf_remote.value( "basic", "role" );
    if ( _arg.exist( "role" ) ) role = _arg.value( "role" );

    cout << "role = " << role << endl;

    try {
        if ( role == "server" ) {
            boost::asio::io_service io_service;
            tcp::endpoint           endpoint( tcp::v4(), std::atoi( "8888" ) );
            auto s = std::make_shared<network::Server>( io_service, endpoint );
            s->start();

            register_processor( s, NULL );

            io_service.run();

            while ( 1 ) sleep( 1 );
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
            register_processor( NULL, c );
            c->on( "connect",
                   []( network::package_ptr, network::client_ptr client ) {
                       client->send( std::make_shared<network::Package>(
                           "reg " + client->hostname() ) );
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

            cout << "Server " << addr << ":" << port << endl;

            boost::asio::io_service io_service;

            tcp::resolver resolver( io_service );
            auto          endpoint_iterator = resolver.resolve( {addr, port} );
            auto          c = std::make_shared<network::Client>( io_service,
                                                        endpoint_iterator );
            c->set_hostname( util::get_hostname() );
            std::cout << "Hostname " << c->hostname() << std::endl;

            register_processor( NULL, c );
            c->on( "connect",
                   []( network::package_ptr, network::client_ptr client ) {
                       client->send( std::make_shared<network::Package>(
                           "reg " + client->hostname() ) );
                   } );

            std::thread t( [&io_service]() { io_service.run(); } );

            char line[network::Package::max_body_length + 1];
            while ( std::cin.getline(
                line, network::Package::max_body_length + 1 ) ) {
                auto msg = std::make_shared<network::Package>();
                msg->body_length( std::strlen( line ) );
                std::memcpy( msg->body(), line, msg->body_length() );
                msg->encrypt();
                Operate::process( line, NULL, NULL, NULL, c );
                /*
                 *c->send( msg );
                 */
            }

            c->close();
            t.join();
        }
    } catch ( std::exception& e ) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
