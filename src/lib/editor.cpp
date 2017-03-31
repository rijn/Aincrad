#include "editor.h"
#include "window.h"

bool Editor::init( int maxrow, int maxcol ) {
    if ( is_init ) return false;
    // dir.init( maxrow - 2, maxcol, 0, 0 );
    file.init( 0, maxcol, maxrow - 2, 0 );
    status.init( maxrow, maxcol );
    is_init = true;
    return true;
}
