# Define version here, before initialization.
m4_define(al_version_major, 0)
m4_define(al_version_minor, 0)
m4_define(al_version_micro, 1)
m4_define(al_version, [al_version_major.al_version_minor.al_version_micro])

# Initialize.
AC_PREREQ([2.62])
AC_INIT(alpaca, [al_version])

# AC Configuration.
AC_CONFIG_SRCDIR([src/.dummy])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

# Configure libtool.
AC_PROG_LIBTOOL

# Configure automake.
AM_INIT_AUTOMAKE([subdir-objects])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/ioctl.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset select socket])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
