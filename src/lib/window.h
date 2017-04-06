#ifndef __WINDOW_H__
#define __WINDOW_H__
#include <ncurses.h>
#include <list>
#include <string>
#include <vector>
#include "util.h"
using std::vector;
using std::list;
using std::string;

enum CTRL_KEY_TYPE {
    KEY_CTRL_C = 3,
    KEY_CTRL_G = 7,
    KEY_CTRL_O = 15,
    KEY_CTRL_Q = 17,
    KEY_CTRL_S = 19,
    KEY_CTRL_X = 24,
    KEY_DELETE = 127,
};

class Window {
   public:
    Window() = default;
    Window( int h, int w, int starty, int startx )
        : max_row( h ), max_col( w ) {
        is_init = true;
        win     = newwin( h, w, starty, startx );
    }
    // disable copy constructor and assignment operator
    Window( const Window& s ) = delete;
    Window& operator=( const Window& s ) = delete;

    bool init( int h, int w, int starty, int startx );

    void printline( const string& line, int row = 0, int col = 0 );
    void clear();

    WINDOW* get_window() const {
        return win;
    }
    bool isinit() const {
        return is_init;
    }

    operator WINDOW*() const {
        return win;
    }

    ~Window() {
        if ( is_init ) delwin( win );
    }

   protected:
    bool    is_init = false;
    WINDOW* win;  // status bar
    size_t  max_row = 0;
    size_t  max_col = 0;
};

class StatusBar : public Window {
   public:
    bool init( int maxrow, int maxcol ) {
        height = maxrow - 2;
        return Window::init( height, maxcol, 0, 0 );
    }

    void print_aincrad();
    void print_filename( const string& file_name );

    std::vector<string> history;
    int                 last_line;
    size_t              height;

   private:
    size_t currrow = 11;
};

class FileContent : public Window {
   public:
    std::vector<string> history;
    int                 vec_idx;
};
#endif