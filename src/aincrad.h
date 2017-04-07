#ifndef __AINCRAD_H__
#define __AINCRAD_H__

#include "arguments.h"
#include "config.h"
#include "util.h"

#include <ncurses.h>
#include <string>

using namespace std;
using namespace util;

#define AINCRAD                                                                                           \
    "   ▄▄   ▄▄▄▄▄  ▄▄   ▄   ▄▄▄  ▄▄▄▄▄    ▄▄   ▄▄▄▄  \n" \
    "   ██     █    █▀▄  █ ▄▀   ▀ █   ▀█   ██   █   ▀▄\n"             \
    "  █  █    █    █ █▄ █ █      █▄▄▄▄▀  █  █  █    █\n"             \
    "  █▄▄█    █    █  █ █ █      █   ▀▄  █▄▄█  █    █\n"             \
    " █    █ ▄▄█▄▄  █   ██  ▀▄▄▄▀ █    ▀ █    █ █▄▄▄▀ \n"

#endif

int aincrad_main( int argc, char* argv[] );

void segfault_handler( int sig );
void run_editor();
bool wgetline( WINDOW* w, string& s, size_t n = 0 );