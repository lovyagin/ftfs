# Autoconf
AC_PREREQ(2.59)
AC_INIT([ftfs],[2018.04.20],[lovyagin@mail.com])
LIBFTFS_VERSION="1"
AC_CONFIG_SRCDIR([ftfs/ftops.c])

# Environment
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

# Automake
AM_INIT_AUTOMAKE([${PACKAGE_NAME}], [${PACKAGE_VERSION}])
AC_CONFIG_MACRO_DIR([m4])
AM_MAINTAINER_MODE

# Options
AC_ARG_ENABLE(
	[debug],
	[AS_HELP_STRING([--enable-debug],[enable debugging code and output])],
	,
	[enable_debug="no"]
)

AC_ARG_ENABLE(
	[warnings],
	[AS_HELP_STRING([--enable-warnings],[enable lots of compiler warnings])],
	,
	[enable_warnings="no"]
)

AC_ARG_ENABLE(
	[pedantic],
	[AS_HELP_STRING([--enable-pedantic],[enable compile pedantic mode])],
	,
	[enable_pedantic="no"]
)

AC_ARG_ENABLE(
    [mount-helper],
    [AS_HELP_STRING([--enable-mount-helper],[install mount helper @<:@default=enabled for linux@:>@])],
    ,
    [
	case "${target_os}" in
	    linux*) enable_mount_helper="yes" ;;
	    *) enable_mount_helper="no" ;;
	esac
    ]
)

AC_ARG_ENABLE(
    [ldscript],
    [AS_HELP_STRING([--enable-ldscript],[use ldscript instead of .so symlink])],
    ,
    [enable_ldscript="no"]
)

AC_ARG_ENABLE(
    [ldconfig],
    [AS_HELP_STRING([--disable-ldconfig],[do not update dynamic linker cache using ldconfig])],
    ,
    [enable_ldconfig="yes"]
)

AC_ARG_ENABLE(
    [library],
    [AS_HELP_STRING([--disable-library],[do not install libftfs but link it into ftfs])],
    ,
    [enable_library="yes"]
)

AC_ARG_ENABLE(
    [ftfs],
    [AS_HELP_STRING([--disable-ftfs],[disable the ftfs FUSE driver])],
    ,
    [enable_ftfs="yes"]
)

AC_ARG_ENABLE(
    [ftfsprogs],
    [AS_HELP_STRING([--disable-ftfsprogs],[disable ftfsprogs utilities
	       (default=no)])],
    ,
    [enable_ftfsprogs="yes"]
)


AC_GNU_SOURCE

AC_PROG_CC(gcc cc)
AC_PROG_LN_S
AM_PROG_CC_C_O

ifdef(
	[LT_INIT],
	[LT_INIT],
	[AC_PROG_LIBTOOL]
)

AC_PROG_INSTALL
PKG_PROG_PKG_CONFIG

AC_ARG_VAR([LDCONFIG], [ldconfig utility])
AC_PATH_PROG([LDCONFIG], [ldconfig], [true], [/sbin /usr/sbin $PATH])

AC_PATH_PROG([MV], [mv])
AC_PATH_PROG([RM], [rm])
#AC_PATH_PROG([SED], [sed])

if test "${enable_ldscript}" = "yes"; then
	AC_MSG_CHECKING([Output format])
	OUTPUT_FORMAT="$(${CC} ${CFLAGS} ${LDFLAGS} -Wl,--verbose 2>&1 | ${SED} -n 's/^OUTPUT_FORMAT("\([[^"]]*\)",.*/\1/p')"
	if test -z "${OUTPUT_FORMAT}"; then
		AC_MSG_RESULT([None])
	else
		AC_MSG_RESULT([${OUTPUT_FORMAT}])
		OUTPUT_FORMAT="OUTPUT_FORMAT ( ${OUTPUT_FORMAT} )"
	fi
fi

# Fuse
if test -z "$PKG_CONFIG"; then
	AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
fi
test "x${PKG_CONFIG}" = "xno" && AC_MSG_ERROR([pkg-config was not found])
PKG_CHECK_MODULES(
	[FUSE_MODULE],
	[fuse >= 2.8.0],
	,
	[
		AC_MSG_ERROR([FUSE >= 2.8.0 was not found])
	]
)
FUSE_MODULE_LIBS=`$PKG_CONFIG --libs fuse`

# Sqlite 
PKG_CHECK_MODULES(
	[SQLITE3_MODULE],
	[sqlite3 >= 3.0.0],
	,
	[
		AC_MSG_ERROR([sqlite >= 3.0 was not found])
	]
)
SQLITE3_MODULE_LIBS=`$PKG_CONFIG --libs sqlite3`

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_MAJOR
AC_CHECK_HEADERS([attr/xattr.h byteswap.h db.h dirent.h errno.h fcntl.h ftfs.h fuse.h grp.h \
inttypes.h libgen.h libkern/OSByteOrder.h limits.h machine/bswap.h memory.h pwd.h sqlite3.h \
stdarg.h stdint.h stdio.h stdlib.h string.h strings.h sys/byteorder.h sys/endian.h \
sys/stat.h sys/statvfs.h sys/time.h sys/types.h sys/xattr.h unistd.h utime.h])

#AC_SEARCH_LIBS([sqlite3_open], [sqlite3], [], [      \
#  AC_MSG_ERROR([required library sqlite3 not found]) \
#])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_ST_BLOCKS
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_CHECK_MEMBERS([struct stat.st_atim])
AC_CHECK_MEMBERS([struct stat.st_atimespec])
AC_CHECK_MEMBERS([struct stat.st_atimensec])

AC_SYS_LARGEFILE

if test "$GCC" = "yes" ; then
	# We add -Wall to enable some compiler warnings.
	CFLAGS="${CFLAGS} -Wall"
fi

if test "${enable_pedantic}" = "yes"; then
	enable_warnings="yes"
	CFLAGS="${CFLAGS} -pedantic"
fi

if test "${enable_warnings}" = "yes"; then
	CFLAGS="${CFLAGS} -W -Wall -Wbad-function-cast -Wcast-align -Wcast-qual -Wdisabled-optimization -Wdiv-by-zero -Wfloat-equal -Winline -Wmissing-format-attribute -Wmissing-noreturn -Wmultichar -Wnested-externs -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-compare -Wstrict-prototypes -Wundef -Wwrite-strings -Wformat -Wformat-security -Wuninitialized"
fi

if test "${enable_debug}" = "yes"; then
	CFLAGS="${CFLAGS} -ggdb3 -DDEBUG"
	AC_DEFINE(
		[ENABLE_DEBUG],
		[1],
		[Define to 1 if debug should be enabled]
	)
else	
	CFLAGS="${CFLAGS} -DNDEBUG"
fi

CFLAGS="${CFLAGS} -D_FILE_OFFSET_BITS=64"

# Settings
pkgconfigdir="\$(libdir)/pkgconfig"
ftfsincludedir="\$(includedir)"
AC_SUBST([pkgconfigdir])
AC_SUBST([ftfsincludedir])
AC_SUBST([LIBFTFS_LIBS])
AC_SUBST([LIBFTFS_VERSION])
AC_SUBST([OUTPUT_FORMAT])
AM_CONDITIONAL([INSTALL_HEADERS], [test "${enable_headers}" = "yes"])
AM_CONDITIONAL([INSTALL_LIBRARY], [test "${enable_library}" = "yes"])
AM_CONDITIONAL([ENABLE_MOUNT_HELPER], [test "${enable_mount_helper}" = "yes"])
AM_CONDITIONAL([ENABLE_FTFS], [test "${enable_ftfs}" = "yes"])
AM_CONDITIONAL([ENABLE_FTFSPROGS], [test "${enable_ftfsprogs}" = "yes"])
AM_CONDITIONAL([RUN_LDCONFIG], [test "${enable_ldconfig}" = "yes"])
AM_CONDITIONAL([GENERATE_LDSCRIPT], [test "${generate_ldscript}" = "yes"])

# workaround for <autoconf-2.60
if test -z "${docdir}"; then
	docdir="\$(datarootdir)/doc/\$(PACKAGE_NAME)"
	AC_SUBST([docdir])
fi
# workaround for <automake-1.10
if test -z "${MKDIR_P}"; then
	MKDIR_P="\$(mkdir_p)"
	AC_SUBST([MKDIR_P])
fi

# generate files
AC_CONFIG_FILES([
	Makefile
	include/Makefile
	libftfs/Makefile
	libftfs/libftfs.pc
	libftfs/libftfs.script.so
	mkftfs/Makefile
	ftpc/Makefile
	ftfs/Makefile
])
AC_OUTPUT

echo "You can type now 'make' to build ftfs."
