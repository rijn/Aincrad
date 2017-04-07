#include "editor.h"
#include "window.h"

bool Editor::init( int maxrow, int maxcol ) {
    if ( is_init ) return false;
    file.init( 1, maxcol, maxrow - 1, 0 );
    status.init( maxrow, maxcol );
    is_init = true;
    return true;
}
