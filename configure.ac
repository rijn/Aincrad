AC_INIT(aincrad, 0.1, bxbian951122@gmail.com, aincrad, https://github.com/rijn/aincrad)
# AC_CONFIG_SRCDIR([src/btfs.cc])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX

# AC_CONFIG_FILES([
# 	man/Makefile
# ])

AC_CONFIG_MACRO_DIR([m4])

BOOST_REQUIRE([1.61],[AC_MSG_ERROR([Boost not found])])
# AX_BOOST_BASE([1.48],, [AC_MSG_ERROR([libfoo needs Boost, but it was not found in your system])])
# AX_BOOST_SYSTEM
# AX_BOOST_FILESYSTEM

# Checks for libraries.
# PKG_CHECK_MODULES(BOOST, boost >= 1.61)
# PKG_CHECK_MODULES(LIBTORRENT, libtorrent-rasterbar >= 0.16.0)
# PKG_CHECK_MODULES(LIBCURL, libcurl >= 7.22.0)

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

# Checks for library functions.
# AC_CHECK_FUNCS([memset mkdir realpath strdup])

# Check for unportable pthread_setname_np()
AC_CHECK_LIB(pthread, pthread_setname_np)

AC_OUTPUT(Makefile src/Makefile)

