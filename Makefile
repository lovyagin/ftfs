TARGETS = all clean install uninstall
SUBDIRS = libftfs mkftfs ftpc ftfs

$(TARGETS): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
