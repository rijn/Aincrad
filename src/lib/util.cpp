#include "util.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <chrono>
#include <ctime>

namespace util {

using namespace std::chrono;

std::string exec( const char* cmd, bool to_stdout ) {
    char        buffer[MAXPIPELEN];
    std::string result = "";
    FILE*       pipe   = popen( cmd, "r" );
    if ( !pipe ) throw std::runtime_error( "popen() failed!" );
    try {
        while ( !feof( pipe ) ) {
            if ( fgets( buffer, MAXPIPELEN, pipe ) != NULL ) {
                result += buffer;
                if ( to_stdout ) {
                    cout << buffer;
                }
            }
        }
    } catch ( ... ) {
        pclose( pipe );
        throw;
    }
    pclose( pipe );
    return result;
}

double diffclock( clock_t clock1, clock_t clock2 ) {

    double diffticks = clock1 - clock2;
    double diffms    = diffticks / ( CLOCKS_PER_SEC / 1000 );

    return diffms;
}

pair<std::string, double> exec_timer(const char* cmd, bool to_stdout) {
    char        buffer[MAXPIPELEN];
    std::string result = "";

    using Clock = std::chrono::high_resolution_clock;

    double t_start = Clock::now().time_since_epoch().count();

    FILE*       pipe   = popen( cmd, "r" );
    if ( !pipe ) throw std::runtime_error( "popen() failed!" );
    try {
        while ( !feof( pipe ) ) {
            if ( fgets( buffer, MAXPIPELEN, pipe ) != NULL ) {
                result += buffer;
                if ( to_stdout ) {
                    cout << buffer;
                }
            }
        }
    } catch ( ... ) {
        pclose( pipe );
        throw;
    }
    double value = pclose( pipe );

    double t_end = Clock::now().time_since_epoch().count();

    return pair<std::string, double>(result, t_end - t_start);
}

vector<string> split(string str, char delimiter) {
  vector<string> internal;
  stringstream ss(str); // Turn the string into a stream.
  string tok;
  
  while(getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }
  
  return internal;
}

// From Monad
void assertExists(const string & path, int exit_code /* = -1 */)
{
    if (!exists(path))
    {
        util::exit_with_error("File does not exist.");
    }
}

bool exists(const string & path)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0) return false;
    if ((st.st_mode & S_IRUSR) == 0) return false;

    if(path == "") throw;
    if (path[path.length()-1] != '/' &&
        (path.length() < 2 || !(path[path.length()-2] == '/' && path[path.length()-1] == '.')))
        return S_ISREG(st.st_mode);

    if ((st.st_mode & S_IXUSR) == 0) return false;
    return S_ISDIR(st.st_mode);
}

mode_t permissions(const string & path)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0) return -1;
    if ((st.st_mode & S_IRUSR) == 0) return -1;

    return (st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));
}

void copy_file(std::string source, std::string destination) {
    assertExists(source);
    vector<string> folders = split(destination, '/');
    string currdir = "";
    for (size_t i = 0; i < folders.size() - 1; i++)
    {
        currdir += folders[i] + '/';
        if (!exists(currdir))
            util::exec(("mkdir -p " + currdir).c_str(), false);
    }
    util::exec(("cp \"" + source + "\" \"" +  destination + "\"").c_str(), false);
}

void copy_files(std::string source, std::string destination, vector<std::string> file_list, bool display) {
    for (size_t i = 0; i < file_list.size(); i++) {
        if(display) cout << file_list[i] << "...";
        copy_file(source+file_list[i], destination+file_list[i]);
        if(display) cout << "done" << endl;
    }
}

// http://stackoverflow.com/questions/2203159/is-there-a-c-equivalent-to-getcwd
std::string get_working_path() {
    char temp[MAXPATHLEN];
    return ( getcwd( temp, MAXPATHLEN ) ? std::string( temp )
                                        : std::string( "" ) );
}

void exit_with_error( const char* message ) {
    fprintf( stderr, "[Error] %s\n", message );
    throw message;
}

void error_handler() {
    /*
     *fprintf( stderr, "Unhandled exception\n" );
     */
    std::abort();
}

namespace colorize
{
const char * BLACK      = "\033[00m";
const char * GREEN      = "\033[01;32m";
const char * RED        = "\033[01;31m";
const char * LIGHT_BLUE = "\033[01;34m";
const bool is_color_enabled = true;
}

std::string passed_string()
{
    return colorize::make_color(colorize::GREEN, "PASSED");
}

std::string failed_string()
{
    return colorize::make_color(colorize::RED  , "FAILED");
}


};
