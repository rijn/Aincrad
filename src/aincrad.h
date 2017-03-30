#ifndef __AINCRAD_H__
#define __AINCRAD_H__

#include "arguments.h"
#include "config.h"
#include "util.h"

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