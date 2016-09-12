ACLOCAL_AMFLAGS=-I m4

lib_LTLIBRARIES = libalpaca.la libalpaca_cpp.la
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

libalpaca_cpp_la_LDFLAGS = \
   -release 0.0.1
libalpaca_cpp_la_CFLAGS = \
   -I$(top_srcdir)/include/c -Wall -std=c99
libalpaca_cpp_la_CXXFLAGS = \
   -I$(top_srcdir)/include/c -I$(top_srcdir)/include/cpp -std=c++11
libalpaca_cpp_la_SOURCES = \
   src/c/connections.c \
   src/c/modules.c \
   src/c/mutex.c \
   src/c/read.c \
   src/c/http.c \
   src/c/server.c \
   src/c/utils.c \
   src/cpp/server.cpp \
   src/cpp/connections.cpp \
   src/cpp/servers/basicserver.cpp \
   src/cpp/servers/emptyserver.cpp

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
   include/c/alpaca/server.h \
   include/c/alpaca/utils.h

noinst_PROGRAMS = echoserver httpserver cpptest
echoserver_SOURCES = src/examples-c/echoserver.c
echoserver_CFLAGS  = -I$(top_srcdir)/include/c -Wall -std=c99
echoserver_LDFLAGS = -L../lib -lalpaca -lpthread

httpserver_SOURCES = src/examples-c/httpserver.c
httpserver_CFLAGS  = -I$(top_srcdir)/include/c -Wall -std=c99
httpserver_LDFLAGS = -L../lib -lalpaca -lpthread

cpptest_SOURCES = AlPACAcpp/AlPACAcpp/main.cpp
cpptest_CXXFLAGS  = -I$(top_srcdir)/include/c -I$(top_srcdir)/include/cpp -Wall -std=c++11
cpptest_LDFLAGS = -L../lib -lalpaca_cpp -lpthread
