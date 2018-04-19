AUTOMAKE_OPTIONS = gnu

MAINTAINERCLEANFILES=           \
	$(srcdir)/configure     \
	$(srcdir)/Makefile.in 

SUBDIRS = include libftfs ftfs mkftfs ftpc

doc_DATA = README

dist-hook:
	$(MKDIR_P) "$(distdir)/m4"

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

libs:
	(cd libftfs && $(MAKE) libs) || exit 1;

libftfs:
	(cd libftfs && $(MAKE) libs) || exit 1;

drivers: lbftfs
	(cd ftfs && $(MAKE) drivers) || exit 1;

ftfsprogs: libftfs
	(cd ftpc && $(MAKE)) || exit 1;
	(cd mkftfs && $(MAKE)) || exit 1;

