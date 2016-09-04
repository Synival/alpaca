ACLOCAL_AMFLAGS=-I m4

lib_LTLIBRARIES = libalpaca.la
libalpaca_la_LDFLAGS = \
   -release 0.0.1
libalpaca_la_CFLAGS = \
   -I$(top_srcdir)/include/c -Wall -std=c99
libalpaca_la_SOURCES = \
   src/c/connections.c \
   src/c/modules.c \
   src/c/mutex.c \
   src/c/read.c \
   src/c/http.c \
   src/c/server.c \
   src/c/utils.c
otherincludedir = \
   $(includedir)/alpaca
otherinclude_HEADERS = \
   include/c/alpaca/alpaca.h \
   include/c/alpaca/defs.h \
   include/c/alpaca/connections.h \
   include/c/alpaca/llist.h \
   include/c/alpaca/modules.h \
   include/c/alpaca/mutex.h \
   include/c/alpaca/read.h \
   include/c/alpaca/http.h \
   include/c/alpaca/server.h

noinst_PROGRAMS = echoserver httpserver
echoserver_SOURCES = src/examples-c/echoserver.c
echoserver_CFLAGS  = -I$(top_srcdir)/include/c -Wall -std=c99
echoserver_LDFLAGS = -L../lib -lalpaca -lpthread

httpserver_SOURCES = src/examples-c/httpserver.c
httpserver_CFLAGS  = -I$(top_srcdir)/include/c -Wall -std=c99
httpserver_LDFLAGS = -L../lib -lalpaca -lpthread
