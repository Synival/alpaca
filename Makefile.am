ACLOCAL_AMFLAGS=-I m4

lib_LTLIBRARIES = libalpaca.la
libalpaca_la_LDFLAGS = \
   -release 0.0.1
libalpaca_la_CFLAGS = \
   -I$(top_srcdir)/include/c
libalpaca_la_SOURCES = \
   src/c/server.c \
   src/c/connections.c
otherincludedir = \
   $(includedir)/alpaca
otherinclude_HEADERS = \
   include/c/alpaca/alpaca.h \
   include/c/alpaca/defs.h \
   include/c/alpaca/connections.h \
   include/c/alpaca/llist.h \
   include/c/alpaca/server.h \
   include/c/alpaca/utils.h
