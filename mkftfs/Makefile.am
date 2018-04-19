if ENABLE_FTFSPROGS

AM_LFLAGS	= $(all_libraries)
LIBTOOL_LINK	= $(LIBTOOL) --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@

bin_PROGRAMS	= mkftfs

CLEANFILES	= $(PROGRAMS)

MAINTAINERCLEANFILES	= Makefile.in

ftpc_LDADD	= $(AM_LIBS) $(SQLITE3_MODULE_LIBS) $(FUSE_MODULE_LIBS) $(top_builddir)/libftfs/libftfs.la
ftpc_CFLAGS	= $(AM_CFLAGS) $(FUSE_CFLAGS) $(SQLITE3_CFLAGS) -I$(top_srcdir)/include/ $(all_includes)
ftpc_SOURCES	= main.c

#strip:	$(bin_PROGRAMS) $(sbin_PROGRAMS)
#	$(STRIP) $^
#
#libs:
#	(cd ../libftfs && $(MAKE) libs) || exit 1;

if ENABLE_MOUNT_HELPER
install-exec-hook:
	$(INSTALL) -d $(DESTDIR)/$(sbindir)
	$(LN_S) -f $(DESTDIR)/$(bindir)/mkftfs $(DESTDIR)/$(sbindir)/mkfs.ftfs
endif

endif
