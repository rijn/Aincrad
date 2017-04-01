#include "window.h"
#include <ncurses.h>
#include <string>
#include <vector>
#include "util.h"
using std::vector;
using std::string;

#define AINCRAD                                                                                           \
    "   ▄▄   ▄▄▄▄▄  ▄▄   ▄   ▄▄▄  ▄▄▄▄▄    ▄▄   ▄▄▄▄  \n" \
    "   ██     █    █▀▄  █ ▄▀   ▀ █   ▀█   ██   █   ▀▄\n"             \
    "  █  █    █    █ █▄ █ █      █▄▄▄▄▀  █  █  █    █\n"             \
    "  █▄▄█    █    █  █ █ █      █   ▀▄  █▄▄█  █    █\n"             \
    " █    █ ▄▄█▄▄  █   ██  ▀▄▄▄▀ █    ▀ █    █ █▄▄▄▀ \n"

vector<string> anicrad =
    // {"   ▄▄   ▄▄▄▄▄  ▄▄   ▄   ▄▄▄  ▄▄▄▄▄    ▄▄   ▄▄▄▄  ",
    //                          "   ██     █    █▀▄  █ ▄▀   ▀ █   ▀█   ██   █
    //                          ▀▄",
    //                          "  █  █    █    █ █▄ █ █      █▄▄▄▄▀  █  █  █
    //                          █",
    //                          "  █▄▄█    █    █  █ █ █      █   ▀▄  █▄▄█  █
    //                          █",
    //                          " █    █ ▄▄█▄▄  █   ██  ▀▄▄▄▀ █    ▀ █    █
    //                          █▄▄▄▀ "};

    {"  ______   __                                                __ ",
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

void StatusBar::print_filename( const string& file_name ) {
    if ( !is_init ) return;
    // wattron( win, A_REVERSE );  // print in reverse color
    int y = 0;
    wmove( win, 0, 0 );
    // wclrtoeol( win );
    // waddch( win, ' ' );
    // std::cout << anicrad << std::endl;

    for ( const auto& i : anicrad ) {
        // if ( i == '\n' ) {
        //     y++;
        //     wmove( win, y, 0 );
        //     continue;
        // }

        // waddch( win, i );
        // std::cout << i << std::endl;
        // wmove( win, ++y, 0 );
        mvwprintw( win, ++y, 0, i.c_str() );
    }
    // waddnstr( win, file_name.c_str(), file_name.size() );
    // for ( size_t i = file_name.size() + 1; i < max_col; ++i )
    //     waddch( win, ' ' );
    wrefresh( win );
    // wattroff( win, A_REVERSE );
    filename = file_name;
}

void StatusBar::print_status( const string& status ) {
    if ( !is_init ) return;
    Window::printline( status, 1, 1 );
}

void FileList::print_filelist( const vector<string>& file_list, ssize_t sel ) {
    size_t chosen;
    if ( sel == -1 )
        chosen = selected;
    else
        chosen     = sel;
    size_t maxline = file_list.size() < max_row ? file_list.size() : max_row;
    if ( !is_init || chosen > maxline ) return;

    curs_set( 0 );
    werase( win );
    for ( size_t i = 0; i < maxline; ++i ) {
        if ( i != chosen )
            mvwaddnstr( win, i, 0, file_list[i].c_str(), max_col );
    }

    // default is to select the first item
    wattron( win, A_REVERSE );
    mvwaddnstr( win, chosen, 0, file_list[chosen].c_str(), max_col );
    wattroff( win, A_REVERSE );
    selected = chosen;

    wrefresh( win );
}

void FileList::scroll_up( const vector<string>& file_list ) {
    if ( selected == 0 ) return;
    print_filelist( file_list, --selected );
}

void FileList::scroll_down( const vector<string>& file_list ) {
    if ( selected >= file_list.size() - 1 ) return;
    print_filelist( file_list, ++selected );
}

// void FileContent::set_file_content( list<ClientLineEntry>* fc, int row,
//                                     int col ) {
//     file_content = fc;
//     currrow      = fc->begin();
//     currrow_num  = row;
//     while ( row-- ) ++currrow;
//     currcol = col;
//     wmove( win, row, col );
// }

// list<ClientLineEntry>::iterator FileContent::get_line( int row ) {
//     auto ret = file_content->begin();
//     while ( row-- ) ++ret;
//     return ret;
// }

// void FileContent::refresh_file_content( int row ) {
//     if ( row != -1 ) {
//         Window::printline( get_line( row )->s, row );
//         wmove( win, currrow_num, currcol );
//         wrefresh( win );
//         return;
//     }
//     werase( win );
//     wmove( win, 0, 0 );
//     for ( const auto& l : *file_content )
//         mvwaddnstr( win, ++row, 0, l.s.c_str(), max_col );
//     wrefresh( win );
//     currrow = get_line( currrow_num );
//     if ( ( currrow->s ).size() < currcol ) {
//         currcol = currrow->s.size();
//     }
//     wmove( win, currrow_num, currcol );
// }

// void FileContent::refresh_file_content( list<ClientLineEntry>::iterator&
// iter,
//                                         int                              row
//                                         ) {
//     Window::printline( iter->s, row );
//     wmove( win, currrow_num, currcol );
//     wrefresh( win );
// }

// void FileContent::refresh_file_content( const string& line, int row ) {
//     Window::printline( line, row );
//     wmove( win, currrow_num, currcol );
//     wrefresh( win );
// }

// void FileContent::refresh_currrow() {
//     Window::printline( currrow->s, currrow_num );
//     wmove( win, currrow_num, currcol );
//     wrefresh( win );
// }

// int FileContent::scroll_up() {
//     if ( currrow_num == 0 ) {
//         if ( currrow->linenum )
//             return -1;  // ask to retieve the line before
//         else
//             return -2;  // we are at the front of file
//     }
//     --currrow;  // move back a line
//     if ( ( currrow->s ).size() < currcol ) {
//         currcol = currrow->s.size();
//     }
//     wmove( win, --currrow_num, currcol );
//     return 1;
// }

// int FileContent::scroll_down() {
//     if ( currrow_num == max_row - 1 ) {
//         if ( currrow->linenum == num_file_lines )
//             return -2;
//         else
//             return -1;  // ask to retieve the line after
//     }
//     if ( ++currrow == file_content->end() ) {
//         --currrow;
//         return -2;
//     }
//     if ( ( currrow->s ).size() < currcol ) {
//         currcol = currrow->s.size();
//     }
//     wmove( win, ++currrow_num, currcol );
//     return 1;
// }

// int FileContent::scroll_right() {
//     if ( currcol >= max_col || currcol == currrow->s.size() ) return 0;
//     wmove( win, currrow_num, ++currcol );
//     return 1;
// }

// int FileContent::scroll_left() {
//     if ( currcol == 0 ) return 0;
//     --currcol;
//     wmove( win, currrow_num, currcol );
//     return 1;
// }

// void FileContent::insertchar( const char& c ) {
//     if ( currcol > max_col || currcol > currrow->s.size() ) return;
//     currrow->s.insert( currrow->s.begin() + currcol, c );
//     refresh_currrow();
//     wmove( win, currrow_num, ++currcol );
// }

// void FileContent::delchar() {
//     if ( currrow->s.empty() || currcol == 0 ) return;
//     if ( currrow->s.size() < currcol ) {
//         PERROR( "delete character past end of line" );
//         return;
//     }
//     --currcol;
//     currrow->s.erase( currrow->s.begin() + currcol );
//     refresh_currrow();
//     wmove( win, currrow_num, currcol );
// }

// void FileContent::add_line() {
//     // prevent client fro inserting new line at the end
//     // of editor
//     if ( !file_content || currrow_num == max_row - 1 ) return;
//     // the contents in original line is splited based on
//     // currcol
//     string temp( currrow->s.begin() + currcol, currrow->s.end() );
//     currrow->s.erase( currrow->s.begin() + currcol, currrow->s.end() );
//     Window::printline( currrow->s, currrow_num );
//     auto iter = currrow;
//     currrow   = file_content->emplace( ++iter, std::move( temp ),
//                                      currrow->linenum + 1 );
//     ++currrow_num;
//     Window::printline( currrow->s, currrow_num );
//     if ( file_content->size() > max_row ) file_content->pop_back();
//     iter = currrow;
//     ++iter;  // in case iter get pop out
//     int y = currrow_num + 1;

//     currcol = 0;
//     while ( iter != file_content->end() ) {
//         Window::printline( iter->s, y );
//         ++( iter->linenum );
//         ++iter;
//         ++y;
//     }
//     // refresh entire file
//     // refresh_file_content(-1);

//     wmove( win, currrow_num, currcol );
// }

// void FileContent::insert_line( const string& s, size_t linenum ) {
//     if ( !file_content ) return;
//     // the contents in original line is splited based on
//     // currcol
//     auto   iter = file_content->begin();
//     size_t y    = 0;
//     for ( ; iter != file_content->end(); ++iter, ++y ) {
//         if ( iter->linenum == linenum ) {
//             ( file_content )->emplace( iter, s, linenum );
//             break;
//         }
//     }
//     for ( ; iter != file_content->end(); ++iter ) {
//         ++( iter->linenum );
//     }
//     if ( file_content->size() > max_row ) file_content->pop_back();
//     ++num_file_lines;
//     // refresh entire file
//     refresh_file_content( -1 );
//     if ( currrow_num <= y ) ++currrow_num;
//     wmove( win, currrow_num, currcol );
//     wrefresh( win );
// }

// ssize_t FileContent::del_line() {
//     if ( !file_content || currrow_num == 0 || file_content->size() == 1 )
//         return -1;
//     currrow = file_content->erase( currrow );
//     for ( auto iter = currrow; iter != file_content->end(); ++iter )
//         --( iter->linenum );
//     --currrow;
//     --currrow_num;
//     --num_file_lines;
//     currcol = 0;
//     if ( num_file_lines > max_row - 1 + file_content->front().linenum )
//         return 1;
//     refresh_file_content( -1 );
//     return 0;
// }

// ssize_t FileContent::delete_line( size_t linenum ) {
//     if ( !file_content ) return -1;
//     int r, c;
//     getyx( win, r, c );
//     auto iter = file_content->begin();
//     for ( ; iter != file_content->end(); ++iter ) {
//         if ( iter->linenum == linenum ) {
//             if ( currrow == iter ) {
//                 currrow = file_content->erase( iter );
//                 iter    = currrow;
//             } else
//                 iter = file_content->erase( iter );
//             break;
//         }
//     }
//     for ( ; iter != file_content->end(); ++iter ) --( iter->linenum );
//     if ( num_file_lines > max_row - 1 + file_content->front().linenum )
//         return 1;
//     refresh_file_content( -1 );
//     set_pos( r - 1, c );
//     wrefresh( win );
//     return 0;
// }

// void FileContent::set_pos( int row, int col ) {
//     currrow     = get_line( row );
//     currrow_num = row;
//     if ( static_cast<int>( currrow->s.size() ) < col )
//         currcol = currrow->s.size();
//     else
//         currcol = col;
//     wmove( win, currrow_num, currcol );
//     wrefresh( win );
// }

// const string& FileContent::get_prevline() const {
//     if ( !file_content || currrow == file_content->begin() ) return
//     currrow->s;
//     auto iter = currrow;
//     return ( --iter )->s;
// }