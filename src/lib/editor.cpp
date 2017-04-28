#include "editor.h"
#include "window.h"

bool Editor::init( int maxrow, int maxcol ) {
    if ( is_init ) return false;
    // use Window's init
    // init( int h, int w, int starty, int startx )
    // at the bottom of the window
    bar.init( 1, maxcol, maxrow - 1, 0 );
    // use block's own init function
    // at the top of the window
    block.init( maxrow, maxcol );
    is_init = true;
    return true;
}
