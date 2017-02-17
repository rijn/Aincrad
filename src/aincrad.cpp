#include "aincrad.h"

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
    arguments   _arg;
    std::string test_id;
    if ( !_arg.process_arguments( argc, argv ) ) return ( EXIT_FAILURE );
    if ( _arg.exist( "test_id" ) ) {
        test_id = _arg.value( "test_id" ).c_str();
    } else {
        return 0;
    }

    cout << colorize::make_color(colorize::LIGHT_BLUE, THEMIS) << endl;

    /* check environment */
    config _conf_remote;

    if ( !_conf_remote.read_config( working_path + "/." + test_id +
                                    "/.config" ) ) {
        exit_with_error( "Cannot find config file" );
    }

}
