#include "aincrad.h"
#include <boost/asio.hpp>
#include <thread>
#include "lib/client.cpp"
#include "lib/package.hpp"
#include "lib/server.cpp"

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

    if ( role == "server" ) {
        try {
            boost::asio::io_service io_service;
            tcp::endpoint           endpoint( tcp::v4(), std::atoi( "8888" ) );
            auto s = std::make_shared<network::Server>( io_service, endpoint );
            /*
             *auto s = new network::Server( io_service, endpoint );
             */
            s->start();

            io_service.run();
        } catch ( std::exception& e ) {
            std::cerr << "Exception: " << e.what() << "\n";
        }
    }

    if ( role == "client" ) {
        string addr = _conf_remote.value( "server", "addr" );
        string port = _conf_remote.value( "server", "port" );

        if ( _arg.exist( "server" ) ) addr = _arg.value( "server" );

        cout << "Server " << addr << ":" << port << endl;

        /*
         *try {
         */
        boost::asio::io_service io_service;

        tcp::resolver   resolver( io_service );
        auto            endpoint_iterator = resolver.resolve( {addr, port} );
        network::Client c( io_service, endpoint_iterator );

        std::thread t( [&io_service]() { io_service.run(); } );

        /*
         *char line[chat_message::max_body_length + 1];
         *while (
         *    std::cin.getline( line, chat_message::max_body_length + 1 ) )
         *{
         *    chat_message msg;
         *    msg.body_length( std::strlen( line ) );
         *    std::memcpy( msg.body(), line, msg.body_length() );
         *    msg.encode_header();
         *    c.write( msg );
         *}
         */

        char line[network::Package::max_body_length + 1];
        while (
            std::cin.getline( line, network::Package::max_body_length + 1 ) ) {
            network::Package msg;
            msg.body_length( std::strlen( line ) );
            std::memcpy( msg.body(), line, msg.body_length() );
            msg.encrypt();
            c.send( msg );
        }

        sleep( 5 );

        c.close();
        t.join();
        /*
         *} catch ( std::exception& e ) {
         *    std::cerr << "Exception: " << e.what() << "\n";
         *}
         */
    }

    while ( 1 ) sleep( 1 );

    return 0;
}
