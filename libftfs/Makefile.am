MAINTAINERCLEANFILES = $(srcdir)/Makefile.in

if INSTALL_LIBRARY
lib_LTLIBRARIES    = libftfs.la
pkgconfig_DATA     = libftfs.pc
else
noinst_LTLIBRARIES = libftfs.la
endif

libftfs_la_CFLAGS	= $(AM_CFLAGS) -I$(top_srcdir)/include
libftfs_la_LIBADD	= $(LIBFTFS_LIBS)
libftfs_la_LDFLAGS	= -version-info $(LIBFTFS_VERSION) -no-undefined

libftfs_la_SOURCES	= cityhash.c db.c ftfs.c fxattr.c misc.c msg.c xattr.c

if ENABLE_FTFSPROGS
libs:	$(lib_LTLIBRARIES)
endif

