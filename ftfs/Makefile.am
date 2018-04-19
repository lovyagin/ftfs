if ENABLE_FTFS

AM_LFLAGS	= $(all_libraries)
LIBTOOL_LINK	= $(LIBTOOL) --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@

bin_PROGRAMS	= ftfs

CLEANFILES	= $(PROGRAMS)
MAINTAINERCLEANFILES	= Makefile.in

ftfs_LDADD	= $(AM_LIBS) $(SQLITE3_MODULE_LIBS) $(FUSE_MODULE_LIBS) $(top_builddir)/libftfs/libftfs.la
ftfs_CFLAGS	= $(AM_CFLAGS) $(FUSE_CFLAGS) $(SQLITE3_CFLAGS) -I$(top_srcdir)/include/ $(all_includes)
ftfs_SOURCES	= ftops.c main.c

drivers : $(FUSE_LIBS) $(SQLITE3_LIBS) ftfs libftfs

install-exec-hook:
if RUN_LDCONFIG
	$(LDCONFIG)
endif

if ENABLE_MOUNT_HELPER
install-exec-local:	install-binPROGRAMS
	$(MKDIR_P) "$(DESTDIR)/$(sbindir)"
	$(LN_S) -f "$(DESTDIR)/$(bindir)/ftfs" "$(DESTDIR)/$(sbindir)/mount.ftfs"
endif

endif # ENABLE_FTFS