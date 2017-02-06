AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = $(BOOST_CPPFLAGS)
AM_LDFLAGS = $(BOOST_THREAD_LDFLAGS)
LIBS = $(BOOST_THREAD_LIBS)

bin_PROGRAMS = aincrad
aincrad_SOURCES = aincrad.cpp aincrad.h
aincrad_CPPFLAGS = -Wall -Wextra -Wconversion -Wsign-compare -Wsign-conversion -Wno-unused-parameter