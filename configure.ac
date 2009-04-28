# configure.ac                            -*-autoconf-*-

# Initialize autoconf.

AC_PREREQ(2.53)
AC_INIT(jack.backup,0.1)

# Initialize automake.

AM_INIT_AUTOMAKE(jack.backup,0.1)

# Check for programs. These macros set and export variables that are
# used in the make process.

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LD
AM_PROG_LIBTOOL

# Require JACK library.

AC_CHECK_LIB(jack, 
             jack_client_new, 
	     , 
	     AC_MSG_ERROR("No JACK library.")
	     ,)
AC_CHECK_HEADERS(jack/jack.h)

# Require LIBSNDFILE library.

AC_CHECK_LIB(sndfile, 
             sf_open, 
	     , 
	     AC_MSG_ERROR("No LibSndFile library.")
	     ,)
AC_CHECK_HEADERS(sndfile.h)

# Set compiler flags.

CFLAGS_OPT="-O3 -funroll-loops"
CFLAGS_WARN="-Wall"
CFLAGS="$CFLAGS $CFLAGS_OPT $CFLAGS_WARN"

# Write Makefile.

AC_OUTPUT(Makefile common/Makefile)
