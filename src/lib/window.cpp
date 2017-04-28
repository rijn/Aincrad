#include "window.h"
#include <ncurses.h>
#include <string>
#include <vector>
#include "util.h"
using std::vector;
using std::string;

vector<string> anicrad = {
    "  ______   __                                                __ ",
    " /      \\ |  \\                                              |  \\",
    "|  $$$$$$\\ \\$$ _______    _______   ______    ______    ____| $$",
    "| $$__| $$|  \\|       \\  /       \\ /      \\  |      \\  /      $$",
    "| $$    $$| $$| $$$$$$$\\|  $$$$$$$|  $$$$$$\\  \\$$$$$$\\|  $$$$$$$",
    "| $$$$$$$$| $$| $$  | $$| $$      | $$   \\$$ /      $$| $$  | $$",
    "| $$  | $$| $$| $$  | $$| $$_____ | $$      |  $$$$$$$| $$__| $$",
    "| $$  | $$| $$| $$  | $$ \\$$     \\| $$       \\$$    $$ \\$$    $$",
    " \\$$   \\$$ \\$$ \\$$   \\$$  \\$$$$$$$ \\$$        \\$$$$$$$  "
    "\\$$$$$$$"};

bool Window::init( int h, int w, int starty, int startx ) {
    if ( is_init ) return false;
    win     = newwin( h, w, starty, startx );
    max_row = h;
    max_col = w;
    is_init = true;
    keypad( win, true );
    return true;
}

void Window::printline( const string& line, int row, int col ) {
    if ( !is_init ) return;
    wmove( win, row, 0 );
    wclrtoeol( win );
    wmove( win, row, col );
    waddnstr( win, line.c_str(), max_col );
    wrefresh( win );
}

void Window::clear() {
    wclear( win );
    wrefresh( win );
}

void StatusBar::print_aincrad() {
    if ( !is_init ) return;
    int y = 0;
    wmove( win, 0, 0 );
    for ( const auto& i : anicrad ) {
        history.push_back( i );
        mvwprintw( win, y++, 0, i.c_str() );
    }
    history.push_back( "" );
    last_line = history.size();
    wrefresh( win );
}

void StatusBar::print_filename( const string& file_name ) {
    if ( !is_init ) return;
    vector<string> splited = util::split( file_name, '\n' );
    if ( file_name.back() == '\n' ) splited.push_back( "\n" );

    history.insert( history.end(), splited.begin(), splited.end() );
    last_line = history.size();

    int next_row = currrow + splited.size();
    int distance = next_row - height;

    if ( distance > 0 ) {
        int i = 0;
        for ( auto it = history.begin() + last_line - height;
              it != history.begin() + last_line; it++ ) {
            wmove( win, i++, 0 );
            wclrtoeol( win );  // erase current line
            waddnstr( win, ( *it ).c_str(), max_col );
        }
        currrow = height;
    } else {
        wmove( win, currrow, 0 );
        wclrtoeol( win );  // erase current line
        waddnstr( win, file_name.c_str(), -1 );
        // n is -1, then the entire string will be added
        currrow = next_row;
    }

    wrefresh( win );
}
