#include "aincrad.h"

#include <boost/asio.hpp>
#include "lib/server.cpp"

using boost::asio::ip::tcp;

namespace aincrad {};

namespace opts {
bool svn = true;
};

int main( int argc, char* argv[] ) {
    /* error handler */
    std::set_terminate( error_handler );

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

    cout << "basic/role = " << _conf_remote.value( "basic", "role" ) << endl;

    try {
        boost::asio::io_service io_service;
        tcp::endpoint           endpoint( tcp::v4(), std::atoi( "8888" ) );
        io_service.run();
        auto server = new network::Server( io_service, endpoint );
        server->start();
    } catch ( std::exception& e ) {
        std::cerr << "Exception: " << e.what() << "\n";
    }


    while ( 1 )
        ;

    return 0;
}
